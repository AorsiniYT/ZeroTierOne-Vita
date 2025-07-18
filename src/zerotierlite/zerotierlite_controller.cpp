// zerotierlite_controller.cpp
// Lógica para conectar a los endpoints del planet y realizar failover
#include "../planet_reader.h"
#include <psp2/net/net.h>
#include <string>
#include <vector>
#include <cstring>
#include <psp2/net/netctl.h>
#include "../debug.h"

// Intenta conectar por UDP a cada endpoint del planet
// Devuelve true si al menos uno responde
// Recibe la identidad local para armar el paquete hello correctamente
#include "identity.h"
#include "packet.h"
#include "ripemd160.h"
#ifndef SceNetTimeval_defined
#define SceNetTimeval_defined
typedef struct SceNetTimeval {
    long tv_sec;
    long tv_usec;
} SceNetTimeval;
#endif

bool conectar_a_controladores(const PlanetData& planet, ztl_identity_t* identidad_local) {
    int sock = sceNetSocket("zt_ctrl", SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM, 0);
    if (sock < 0) {
        vita_debug_log("[ZTLITE] Error creando socket UDP");
        return false;
    }
    bool conectado = false;
    vita_debug_log("[ZTLITE] planet.roots.size=%zu", planet.roots.size());
    int root_idx = 0;
    for (const auto& root : planet.roots) {
        vita_debug_log("[ZTLITE] root[%d].identity=%s endpoints.size=%zu", root_idx, root.identity.c_str(), root.endpoints.size());
        int ep_idx = 0;
        for (const auto& ep : root.endpoints) {
            vita_debug_log("[ZTLITE] root[%d].endpoint[%d] ip=%s port=%d", root_idx, ep_idx, ep.ip.c_str(), ep.port);
            ep_idx++;
            SceNetSockaddrIn addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = SCE_NET_AF_INET;
            addr.sin_port = sceNetHtons(ep.port);
            int ipret = sceNetInetPton(SCE_NET_AF_INET, ep.ip.c_str(), &addr.sin_addr);
            vita_debug_log("[ZTLITE] sceNetInetPton ret=%d", ipret);
            if (ipret != 1) {
                vita_debug_log("[ZTLITE] IP inválida: %s", ep.ip.c_str());
                continue;
            }
            vita_debug_log("[ZTLITE] Intentando conectar a controlador %s:%d", ep.ip.c_str(), ep.port);
            // Log de identidad antes de construir el paquete hello
            char id_addr_hex[32] = {0};
            snprintf(id_addr_hex, sizeof(id_addr_hex), "[HELLO] identidad_local->address: %02X %02X %02X %02X %02X", identidad_local->address[0], identidad_local->address[1], identidad_local->address[2], identidad_local->address[3], identidad_local->address[4]);
            vita_debug_log("%s", id_addr_hex);
            char id_pk_hex[128] = {0};
            snprintf(id_pk_hex, sizeof(id_pk_hex), "[HELLO] identidad_local->public_key: %02X %02X %02X %02X ... %02X", identidad_local->public_key[0], identidad_local->public_key[1], identidad_local->public_key[2], identidad_local->public_key[3], identidad_local->public_key[31]);
            vita_debug_log("%s", id_pk_hex);
            // Construir y enviar paquete hello con formato ZeroTierOne 1.14.2
            unsigned char hello[ZTL_PACKET_MAX_SIZE] = {0};
            // Calcular address del controlador desde la clave pública (root.identity)
            unsigned char controller_addr[5] = {0};
            // Asume que root.identity es la clave pública en hex (64 caracteres)
            // Se debe calcular RIPEMD-160(pubkey) y tomar los primeros 5 bytes
            unsigned char pubkey_bin[32] = {0};
            for (int i = 0; i < 32; ++i) {
                sscanf(root.identity.c_str() + i*2, "%2hhx", &pubkey_bin[i]);
            }
            unsigned char ripemd[20] = {0};
            ripemd160(pubkey_bin, 32, ripemd);
            memcpy(controller_addr, ripemd, 5);
            // Construir el paquete hello con el address destino correcto
            int hello_len = ztl_packet_build_hello(hello, identidad_local);
            memcpy(hello+16, controller_addr, 5); // Poner address destino en el header
            // Log del campo destino real
            char destino_hex[32] = {0};
            snprintf(destino_hex, sizeof(destino_hex), "[HELLO] Destino: %02X %02X %02X %02X %02X", hello[16], hello[17], hello[18], hello[19], hello[20]);
            vita_debug_log("%s", destino_hex);
            int ret = sceNetSendto(sock, hello, hello_len, 0, (SceNetSockaddr*)&addr, sizeof(addr));
            vita_debug_log("[ZTLITE] sceNetSendto ret=%d (hello_len=%d)", ret, hello_len);
            if (ret > 0) {
                // Esperar respuesta (timeout corto)
                char buf[256];
                SceNetSockaddrIn from;
                unsigned int fromlen = sizeof(from);
                SceNetTimeval tv = {1, 0}; // 1 segundo
                sceNetSetsockopt(sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &tv, sizeof(tv));
                int rcv = sceNetRecvfrom(sock, buf, sizeof(buf), 0, (SceNetSockaddr*)&from, &fromlen);
                vita_debug_log("[ZTLITE] sceNetRecvfrom ret=%d", rcv);
                if (rcv > 0) {
                    conectado = true;
                    vita_debug_log("[ZTLITE] Controlador %s:%d respondió", ep.ip.c_str(), ep.port);
                    break;
                }
            }
        }
        if (conectado) break;
    }
    sceNetSocketClose(sock);
    return conectado;
}
