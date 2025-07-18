
#include <stdio.h>
#include "../debug.h"
#include <string.h>
#include <stdint.h>
#include "identity.h"

#include "../../third_party/ed25519-donna/ed25519.h"
#include "../../third_party/ed25519-donna/ed25519-hash.h"



int ztl_identity_generate(ztl_identity_t* id) {
    if (!id) return -1;
    // Generar clave privada aleatoria
    ed25519_randombytes_unsafe(id->private_key, 32);
    // Generar clave pública
    ed25519_publickey(id->private_key, id->public_key);
    // Log clave pública
    char pkbuf[65] = {0};
    for (int i = 0; i < 32; ++i) sprintf(pkbuf + i*2, "%02x", id->public_key[i]);
    vita_debug_log("[IDENTITY] public_key: %s", pkbuf);
    char pkraw[128] = {0};
    snprintf(pkraw, sizeof(pkraw), "[IDENTITY] public_key raw: %02X %02X %02X %02X ... %02X", id->public_key[0], id->public_key[1], id->public_key[2], id->public_key[3], id->public_key[31]);
    vita_debug_log("%s", pkraw);
    // Calcular SHA512 del public key
    uint8_t hash[64];
    ed25519_hash(hash, id->public_key, 32);
    char hashbuf[129] = {0};
    for (int i = 0; i < 64; ++i) sprintf(hashbuf + i*2, "%02x", hash[i]);
    vita_debug_log("[IDENTITY] sha512(public_key): %s", hashbuf);
    // Calcular RIPEMD-160 del hash SHA512
    ripemd160(hash, 64, id->address);
    char addrbuf[17] = {0};
    for (int i = 0; i < 8; ++i) sprintf(addrbuf + i*2, "%02x", id->address[i]);
    vita_debug_log("[IDENTITY] address (RIPEMD-160, 8 bytes): %s", addrbuf);
    char addrraw[64] = {0};
    snprintf(addrraw, sizeof(addrraw), "[IDENTITY] address raw: %02X %02X %02X %02X %02X", id->address[0], id->address[1], id->address[2], id->address[3], id->address[4]);
    vita_debug_log("%s", addrraw);
    return 0;
}

int ztl_identity_load(ztl_identity_t* id, const char* path) {
    // Implementación real: cargar desde archivo binario
    FILE* f = fopen(path ? path : "ux0:data/zerotierone/identity_secret.conf", "rb");
    if (!f) {
        vita_debug_log("[IDENTITY] No existe archivo de identidad, se generará nueva.");
        return -1;
    }
    size_t r = fread(id, 1, sizeof(ztl_identity_t), f);
    fclose(f);
    if (r != sizeof(ztl_identity_t)) {
        vita_debug_log("[IDENTITY] Error leyendo archivo de identidad, tamaño inesperado: %zu", r);
        return -2;
    }
    vita_debug_log("[IDENTITY] Identidad cargada correctamente de almacenamiento.");
    return (int)r;
}

int ztl_identity_save(const ztl_identity_t* id, const char* path) {
    // Implementación real: guardar a archivo binario
    FILE* f = fopen(path ? path : "ux0:data/zerotierone/identity_secret.conf", "wb");
    if (!f) {
        vita_debug_log("[IDENTITY] Error abriendo archivo para guardar identidad.");
        return -1;
    }
    size_t w = fwrite(id, 1, sizeof(ztl_identity_t), f);
    fclose(f);
    if (w != sizeof(ztl_identity_t)) {
        vita_debug_log("[IDENTITY] Error escribiendo archivo de identidad, tamaño inesperado: %zu", w);
        return -2;
    }
    vita_debug_log("[IDENTITY] Identidad guardada correctamente en almacenamiento.");
    return (int)w;
}

// Convierte el address de la identidad a string hexadecimal (al menos 11 bytes)
void ztl_identity_get_id_str(const ztl_identity_t* id, char* out, size_t outlen) {
    if (!id || !out || outlen < 11) return;
    // Solo los primeros 5 bytes (40 bits)
    snprintf(out, outlen, "%02x%02x%02x%02x%02x",
        id->address[0], id->address[1], id->address[2], id->address[3], id->address[4]);
    vita_debug_log("[IDENTITY] get_id_str: %02x%02x%02x%02x%02x", id->address[0], id->address[1], id->address[2], id->address[3], id->address[4]);
}
