// Header para funciones de almacenamiento de ZeroTierOne
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int zt_load_state(enum ZT_StateObjectType type, const uint64_t id[2], void* buf, unsigned int buflen);
void zt_save_state(enum ZT_StateObjectType type, const uint64_t id[2], const void* data, int len);

#ifdef __cplusplus
}
#endif
