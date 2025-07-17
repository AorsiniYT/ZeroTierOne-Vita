#pragma once
// Unión y gestión de red virtual

#ifdef __cplusplus
extern "C" {
#endif

#include "identity.h"
#include "state.h"

int ztl_network_join(const char* network_id, ztl_identity_t* id);
int ztl_network_leave(const char* network_id);

#ifdef __cplusplus
}
#endif
