#pragma once
// Encapsulamiento y parsing de paquetes ZeroTier

#ifdef __cplusplus
extern "C" {
#endif

#define ZTL_PACKET_MAX_SIZE 2048

typedef struct {
    unsigned char data[ZTL_PACKET_MAX_SIZE];
    int length;
} ztl_packet_t;

int ztl_packet_create(ztl_packet_t* pkt, const void* payload, int len);
int ztl_packet_parse(const ztl_packet_t* pkt, void* out, int* outlen);

#ifdef __cplusplus
}
#endif
