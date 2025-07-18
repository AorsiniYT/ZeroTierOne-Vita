


#include "zerotieronevita.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zerotierlite/identity.h"
#include "zerotierlite/state.h"
#include "zerotierlite/network_config.h"
#include "zerotierlite/peer.h"
#include "zerotierlite/packet.h"
#include "zerotierlite/transport.h"
#include "zt_storage.h"
#include "debug.h"


// Estado global simplificado del cliente
   ztl_identity_t g_identity;
static ztl_state_t g_state = ZTL_STATE_DISCONNECTED;
static ztl_network_config_t g_network;
static ztl_peer_t g_peer;

// ID local (hex) generado desde la identidad
static char g_id_str[24] = {0};
static uint64_t g_address = 0;

// Devuelve el address local (uint64_t)
uint64_t zerotierone_get_address() {
    vita_debug_log("[ZeroTierOne] get_address: 0x%016llx", (unsigned long long)g_address);
    return g_address;
}

// Devuelve el address local como string hexadecimal
const char* zerotierone_get_address_str() {
    vita_debug_log("[ZeroTierOne] get_address_str: %s", g_id_str);
    return g_id_str;
}



void zerotierone_init() {
    vita_debug_log("[ZeroTierOne] Inicializando identidad local...");
    uint64_t dummy_id[2] = {0, 0};
    int loaded = zt_load_state(ZT_STATE_OBJECT_IDENTITY_SECRET, dummy_id, &g_identity, sizeof(g_identity));
    if (loaded == sizeof(g_identity)) {
        vita_debug_log("[ZeroTierOne] Identidad cargada de almacenamiento.");
    } else {
        vita_debug_log("[ZeroTierOne] No hay identidad guardada, generando nueva...");
        if (ztl_identity_generate(&g_identity) == 0) {
            vita_debug_log("[ZeroTierOne] Identidad generada.");
            zt_save_state(ZT_STATE_OBJECT_IDENTITY_SECRET, dummy_id, &g_identity, sizeof(g_identity));
        } else {
            vita_debug_log("[ZeroTierOne] Error generando identidad.");
            g_state = ZTL_STATE_ERROR;
            return;
        }
    }
    // Mostrar el ID (hash público) como string hexadecimal y como uint64_t
    ztl_identity_get_id_str(&g_identity, g_id_str, sizeof(g_id_str));
    // El address está en g_identity.address (8 bytes, big endian)
    memcpy(&g_address, g_identity.address, 8);
    vita_debug_log("[ZeroTierOne] ID local: %s (address: 0x%016llx)", g_id_str, (unsigned long long)g_address);
    g_state = ZTL_STATE_AUTHENTICATING;
    if (ztl_transport_init() == 0) {
        vita_debug_log("[ZeroTierOne] Transporte inicializado.");
    } else {
        vita_debug_log("[ZeroTierOne] Error inicializando transporte.");
        g_state = ZTL_STATE_ERROR;
    }
}


void zerotierone_join(const char* network_id) {
    if (!network_id) {
        vita_debug_log("[ZeroTierOne] network_id nulo");
        return;
    }
    vita_debug_log("[ZeroTierOne] Uniéndose a red: %s", network_id);
    vita_debug_log("[ZeroTierOne] My address: %s (0x%016llx)", g_id_str, (unsigned long long)g_address);
    uint64_t net_id[2] = {0, 0};
    int loaded = zt_load_state(ZT_STATE_OBJECT_NETWORK_CONFIG, net_id, &g_network, sizeof(g_network));
    if (loaded == sizeof(g_network) && strcmp(g_network.network_id, network_id) == 0) {
        vita_debug_log("[ZeroTierOne] Configuración de red cargada de almacenamiento.");
    } else {
        if (ztl_network_config_init(&g_network, network_id) == 0) {
            vita_debug_log("[ZeroTierOne] Nueva configuración de red inicializada.");
            zt_save_state(ZT_STATE_OBJECT_NETWORK_CONFIG, net_id, &g_network, sizeof(g_network));
        } else {
            g_state = ZTL_STATE_ERROR;
            vita_debug_log("[ZeroTierOne] Error configurando red.");
            return;
        }
    }
    g_state = ZTL_STATE_JOINING;
    ztl_peer_init(&g_peer, &g_identity);
    if (ztl_peer_authenticate(&g_peer) == 0) {
        g_state = ZTL_STATE_CONNECTED;
        vita_debug_log("[ZeroTierOne] Conectado a red %s", network_id);
        // Intentar recibir respuesta del controlador con reintentos
        unsigned char recvbuf[256];
        int rlen = -1;
        int retries = 5;
        for (int i = 0; i < retries; ++i) {
            rlen = ztl_transport_recv(recvbuf, sizeof(recvbuf));
            if (rlen > 0) {
                vita_debug_log("[ZeroTierOne] Paquete recibido del controlador (%d bytes, intento %d)", rlen, i+1);
                // Procesar el paquete (aquí iría el parser real)
                char hex[513] = {0};
                for (int j = 0; j < rlen && j < 32; ++j) sprintf(hex + j*2, "%02x", recvbuf[j]);
                vita_debug_log("[ZeroTierOne] Datos hex: %s...", hex);
                break;
            }
            vita_debug_log("[ZeroTierOne] Reintento %d: no se recibió respuesta del controlador.", i+1);
            // Espera 200 ms antes del siguiente intento
            #ifdef VITA
            sceKernelDelayThread(200000);
            #endif
        }
        if (rlen <= 0) {
            vita_debug_log("[ZeroTierOne] No se recibió respuesta del controlador tras %d intentos.", retries);
        }
    } else {
        g_state = ZTL_STATE_ERROR;
        vita_debug_log("[ZeroTierOne] Error autenticando peer.");
    }
}

void zerotierone_leave(const char* network_id) {
    (void)network_id;
    vita_debug_log("[ZeroTierOne] Saliendo de la red...");
    ztl_transport_close();
    g_state = ZTL_STATE_DISCONNECTED;
}

void zerotierone_status() {
    vita_debug_log("[ZeroTierOne] Estado: %s", ztl_state_str(g_state));
    vita_debug_log("[ZeroTierOne] ID: %s", g_id_str);
    // Aquí podrías mostrar más info real del nodo si lo deseas
}
