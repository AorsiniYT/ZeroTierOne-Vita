#pragma once
#include "identity.h"
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

// Construye un hello packet compatible con ZeroTierOne 1.14.2
int ztl_packet_build_hello(unsigned char* buffer, const ztl_identity_t* identity);

#ifdef __cplusplus
}
#endif
