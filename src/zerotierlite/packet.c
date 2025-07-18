#include "packet.h"
#include <string.h>
#include "identity.h"
#include "mac.h"
#include "ip_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include "version.h"
#include <stdint.h>
#include <time.h>
// Para logs
#include "../debug.h"

int ztl_packet_create(ztl_packet_t* pkt, const void* payload, int len) {
    if (len > ZTL_PACKET_MAX_SIZE) return -1;
    memcpy(pkt->data, payload, len);
    pkt->length = len;
    return 0;
}

int ztl_packet_parse(const ztl_packet_t* pkt, void* out, int* outlen) {
    if (*outlen < pkt->length) return -1;
    memcpy(out, pkt->data, pkt->length);
    *outlen = pkt->length;
    return 0;
}

// Construye un hello packet compatible con ZeroTierOne 1.14.2
// El buffer debe tener al menos ZTL_PACKET_MAX_SIZE bytes

int ztl_packet_build_hello(unsigned char* buffer, const ztl_identity_t* identity) {
    // Header ZeroTierOne (28 bytes):
    // [0-7]   IV (packet id, random)
    // [8-15]  MAC (puede ser 0 para hello)
    // [16-20] Destino (address del controlador, para discovery puede ser 0)
    // [21-25] Fuente (address local)
    // [26]    Flags (cipher suite, hops, etc.)
    // [27]    Verb (VERB_HELLO = 1)
    // [28...] Payload

    // IV: random
    uint64_t iv = ((uint64_t)rand() << 32) | (uint64_t)rand();
    for (int i = 0; i < 8; ++i) buffer[i] = (iv >> (56 - i*8)) & 0xFF;
    // MAC: obtener MAC real de la Vita
    extern int ztl_get_mac_address(unsigned char* mac_out);
    unsigned char mac[8] = {0};
    if (ztl_get_mac_address(mac) == 0) {
        memcpy(buffer+8, mac, 8);
    } else {
        memset(buffer+8, 0, 8);
    }
    // Destino: 0 (descubrimiento)
    memset(buffer+16, 0, 5);
    // Fuente: address local (5 bytes)
    memcpy(buffer+21, identity->address, 5);
    // Flags: cipher suite NONE (0), hops=0
    buffer[26] = 0x00;
    // Verb: VERB_HELLO = 1
    buffer[27] = 0x01;

    // Payload
    int idx = 28;
    // Protocolo ZeroTierOne: version=12, major=1, minor=14, revision=2 (para 1.14.2)
    buffer[idx++] = 12; // Protocol version (ZT_PROTO_VERSION)
    buffer[idx++] = 1;  // Major version
    buffer[idx++] = 14; // Minor version
    buffer[idx++] = 2;  // Revision (primer byte)
    buffer[idx++] = 0;  // Revision (segundo byte)
    // Timestamp (8 bytes, big endian)
    uint64_t ts = (uint64_t)time(NULL);
    for (int i = 0; i < 8; ++i) buffer[idx++] = (ts >> (56 - i*8)) & 0xFF;
    // Identidad serializada (formato ZeroTierOne: tipo, address, public_key, signature)
    // tipo=1 (normal), address (5 bytes), public_key (32 bytes), longitud de firma (2 bytes, 0), sin firma
    buffer[idx++] = 1; // Tipo de identidad (normal)
    memcpy(buffer + idx, identity->address, 5); idx += 5;
    memcpy(buffer + idx, identity->public_key, 32); idx += 32;
    // Longitud de la firma (2 bytes, big endian, 0)
    buffer[idx++] = 0;
    buffer[idx++] = 0;
    // Sin firma
    // Dirección física de destino (InetAddress serializada: IPv4, 4 bytes)
    // Usar la IP real de la Vita
    char ip_str[32] = {0};
    unsigned char ip_bytes[4] = {0};
    extern int ztl_get_local_ip(char* out, size_t outlen); // Debe devolver la IP en formato string
    if (ztl_get_local_ip(ip_str, sizeof(ip_str)) == 0) {
        sscanf(ip_str, "%hhu.%hhu.%hhu.%hhu", &ip_bytes[0], &ip_bytes[1], &ip_bytes[2], &ip_bytes[3]);
        buffer[idx++] = ip_bytes[0];
        buffer[idx++] = ip_bytes[1];
        buffer[idx++] = ip_bytes[2];
        buffer[idx++] = ip_bytes[3];
    } else {
        buffer[idx++] = 0; buffer[idx++] = 0; buffer[idx++] = 0; buffer[idx++] = 0;
    }
    // (Opcional) World ID, planet timestamp, lunas, etc. pueden ir después

    // LOGS DETALLADOS
    char hex[128];
    snprintf(hex, sizeof(hex), "[HELLO] IV: %02X %02X %02X %02X %02X %02X %02X %02X", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
    vita_debug_log("%s", hex);
    snprintf(hex, sizeof(hex), "[HELLO] MAC: %02X %02X %02X %02X %02X %02X %02X %02X", buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
    vita_debug_log("%s", hex);
    snprintf(hex, sizeof(hex), "[HELLO] Destino: %02X %02X %02X %02X %02X", buffer[16], buffer[17], buffer[18], buffer[19], buffer[20]);
    vita_debug_log("%s", hex);
    snprintf(hex, sizeof(hex), "[HELLO] Fuente: %02X %02X %02X %02X %02X", buffer[21], buffer[22], buffer[23], buffer[24], buffer[25]);
    vita_debug_log("%s", hex);
    vita_debug_log("[HELLO] Flags: %02X", buffer[26]);
    vita_debug_log("[HELLO] Verb: %02X", buffer[27]);
    vita_debug_log("[HELLO] Payload version: %u.%u.%u.%u", buffer[28], buffer[29], buffer[30], buffer[31]);
    snprintf(hex, sizeof(hex), "[HELLO] Timestamp: %02X %02X %02X %02X %02X %02X %02X %02X", buffer[33], buffer[34], buffer[35], buffer[36], buffer[37], buffer[38], buffer[39], buffer[40]);
    vita_debug_log("%s", hex);
    snprintf(hex, sizeof(hex), "[HELLO] Identity: tipo=%u address=%02X%02X%02X%02X%02X pk=%02X...%02X", buffer[41], buffer[42], buffer[43], buffer[44], buffer[45], buffer[46], buffer[47], buffer[77]);
    vita_debug_log("%s", hex);
    snprintf(hex, sizeof(hex), "[HELLO] DestInet: %u.%u.%u.%u", buffer[78], buffer[79], buffer[80], buffer[81]);
    vita_debug_log("%s", hex);

    // Retornar tamaño mínimo del hello packet
    return idx;
}
