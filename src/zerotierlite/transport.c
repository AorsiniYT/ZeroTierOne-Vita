

#include "transport.h"
#include "../debug.h"
#include <string.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/kernel/threadmgr.h>

static int g_udp_sock = -1;

int ztl_transport_init(void) {
    vita_debug_log("[transport] ztl_transport_init()");
    if (g_udp_sock >= 0) {
        vita_debug_log("[transport] El socket UDP ya estaba inicializado: %d", g_udp_sock);
        return 0;
    }
    g_udp_sock = sceNetSocket("zt_sock", SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM, 0);
    vita_debug_log("[transport] socket creado: %d", g_udp_sock);
    if (g_udp_sock < 0) {
        vita_debug_log("[transport] ERROR: no se pudo crear socket UDP");
        return -1;
    }
    SceNetSockaddrIn local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = SCE_NET_AF_INET;
    local_addr.sin_port = sceNetHtons(9993); // Puerto local 9993
    local_addr.sin_addr.s_addr = sceNetHtonl(0); // INADDR_ANY
    int bindret = sceNetBind(g_udp_sock, (SceNetSockaddr*)&local_addr, sizeof(local_addr));
    vita_debug_log("[transport] bind ret=%d (socket=%d, puerto=%d)", bindret, g_udp_sock, 9993);
    if (bindret < 0) {
        vita_debug_log("[transport] ERROR: no se pudo hacer bind al puerto UDP");
        sceNetSocketClose(g_udp_sock);
        g_udp_sock = -1;
        return -2;
    }
    return 0;
}

int ztl_transport_send(const void* data, int len) {
    vita_debug_log("[transport] ztl_transport_send(len=%d)", len);
    if (g_udp_sock < 0) {
        vita_debug_log("[transport] ERROR: socket UDP no inicializado");
        return -1;
    }
    SceNetSockaddrIn addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = SCE_NET_AF_INET;
    addr.sin_port = sceNetHtons(9993);
    int ipret = sceNetInetPton(SCE_NET_AF_INET, "35.205.255.45", &addr.sin_addr);
    vita_debug_log("[transport] sceNetInetPton ret=%d", ipret);
    int ret = sceNetSendto(g_udp_sock, data, len, 0, (SceNetSockaddr*)&addr, sizeof(addr));
    vita_debug_log("[transport] sceNetSendto ret=%d", ret);
    return ret;
}

int ztl_transport_recv(void* buf, int buflen) {
    vita_debug_log("[transport] ztl_transport_recv(buflen=%d)", buflen);
    if (g_udp_sock < 0) {
        vita_debug_log("[transport] ERROR: socket UDP no inicializado para recv");
        return -1;
    }
    SceNetSockaddrIn from_addr;
    unsigned int from_len = sizeof(from_addr);
    int rlen = -1;
    int retries = 5;
    for (int i = 0; i < retries; ++i) {
        rlen = sceNetRecvfrom(g_udp_sock, buf, buflen, SCE_NET_MSG_DONTWAIT, (SceNetSockaddr*)&from_addr, &from_len);
        vita_debug_log("[transport] recv intento %d: ret=%d", i+1, rlen);
        if (rlen > 0) {
            vita_debug_log("[transport] Paquete recibido (%d bytes)", rlen);
            break;
        }
        sceKernelDelayThread(200000); // 200 ms
    }
    if (rlen <= 0) {
        vita_debug_log("[transport] No se recibió paquete UDP tras %d intentos", retries);
    }
    return rlen;
}

void ztl_transport_close(void) {
    vita_debug_log("[transport] ztl_transport_close()");
    if (g_udp_sock >= 0) {
        sceNetSocketClose(g_udp_sock);
        vita_debug_log("[transport] socket UDP cerrado: %d", g_udp_sock);
        g_udp_sock = -1;
    }
}
