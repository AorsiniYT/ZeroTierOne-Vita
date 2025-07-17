#pragma once
// Gestión de peers y autenticación

#ifdef __cplusplus
extern "C" {
#endif

#include "identity.h"

typedef struct {
    ztl_identity_t identity;
    int authenticated;
    // Otros campos relevantes para el peer
} ztl_peer_t;

int ztl_peer_init(ztl_peer_t* peer, const ztl_identity_t* id);
int ztl_peer_authenticate(ztl_peer_t* peer);

#ifdef __cplusplus
}
#endif
