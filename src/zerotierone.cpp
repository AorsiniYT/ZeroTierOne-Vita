extern "C" {
#include "zerotierone.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ZeroTierOne.h>

static ZT_Node* g_node = NULL;

// Callbacks mínimos (persistencia real)
#include "zt_storage.h"

static void eventCallback(ZT_Node* node, void* uptr, void* tptr, enum ZT_Event event, const void* meta) {
    printf("[ZT] Event: %d\n", event);
}

static void portConfigCallback(ZT_Node* node, void* uptr, void* tptr, uint64_t nwid, void** nuptr, enum ZT_VirtualNetworkConfigOperation op, const ZT_VirtualNetworkConfig* config) {
    printf("[ZT] Port config: nwid=%llx op=%d status=%d\n", nwid, op, config ? config->status : -1);
}

static void statePutFunction(ZT_Node* node, void* uptr, void* tptr, enum ZT_StateObjectType type, const uint64_t id[2], const void* data, int len) {
    zt_save_state(type, id, data, len);
}

static int stateGetFunction(ZT_Node* node, void* uptr, void* tptr, enum ZT_StateObjectType type, const uint64_t id[2], void* buf, unsigned int buflen) {
    return zt_load_state(type, id, buf, buflen);
}

static int wirePacketSendFunction(ZT_Node* node, void* uptr, void* tptr, int64_t localSocket, const struct sockaddr_storage* remoteAddress, const void* packetData, unsigned int packetLength, unsigned int ttl) {
    // Dummy: no envía nada
    return 0;
}

static void virtualNetworkFrameFunction(ZT_Node* node, void* uptr, void* tptr, uint64_t nwid, void** nuptr, uint64_t srcMac, uint64_t dstMac, unsigned int etherType, unsigned int vlanId, const void* frameData, unsigned int frameLen) {
    // Dummy: no hace nada
}

static int virtualNetworkConfigFunction(ZT_Node* node, void* uptr, void* tptr, uint64_t nwid, void** nuptr, enum ZT_VirtualNetworkConfigOperation op, const ZT_VirtualNetworkConfig* config) {
    // Dummy: solo imprime
    printf("[ZT] Port config: nwid=%llx op=%d status=%d\n", nwid, op, config ? config->status : -1);
    return 0;
}

static struct ZT_Node_Callbacks g_callbacks = {
    0, // version
    statePutFunction,
    stateGetFunction,
    wirePacketSendFunction,
    virtualNetworkFrameFunction,
    virtualNetworkConfigFunction,
    eventCallback,
    NULL, // pathCheckFunction
    NULL  // pathLookupFunction
};

void zerotierone_init() {
    if (!g_node) {
        int64_t now = 0; // Puedes usar un timestamp real si lo necesitas
        enum ZT_ResultCode rc = ZT_Node_new(&g_node, NULL, NULL, &g_callbacks, now);
        printf("[ZT] Node init: %d\n", rc);
    }
}

void zerotierone_join(const char* network_id) {
    if (!g_node) zerotierone_init();
    uint64_t nwid = strtoull(network_id, NULL, 16); // Network ID en formato uint64
    enum ZT_ResultCode rc = ZT_Node_join(g_node, nwid, NULL, NULL);
    printf("[ZT] Join: %d\n", rc);
}

void zerotierone_leave(const char* network_id) {
    if (!g_node) return;
    uint64_t nwid = strtoull(network_id, NULL, 16);
    enum ZT_ResultCode rc = ZT_Node_leave(g_node, nwid, NULL, NULL);
    printf("[ZT] Leave: %d\n", rc);
}

void zerotierone_status() {
    if (!g_node) return;
    ZT_NodeStatus status;
    ZT_Node_status(g_node, &status);
    printf("[ZT] Status: address=%llx online=%d\n", status.address, status.online);
}
}
