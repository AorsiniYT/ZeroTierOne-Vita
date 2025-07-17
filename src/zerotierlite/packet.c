#include "packet.h"
#include <string.h>

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
