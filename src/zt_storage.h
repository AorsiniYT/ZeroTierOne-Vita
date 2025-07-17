
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Definición local mínima para compatibilidad con ZeroTierLite
typedef enum {
    ZT_STATE_OBJECT_IDENTITY_PUBLIC = 1,
    ZT_STATE_OBJECT_IDENTITY_SECRET = 2,
    ZT_STATE_OBJECT_PLANET = 3,
    ZT_STATE_OBJECT_NETWORK_CONFIG = 4
} ZT_StateObjectType;

void ensure_storage_dir();
int zt_load_state(ZT_StateObjectType type, const uint64_t id[2], void* buf, unsigned int buflen);
void zt_save_state(ZT_StateObjectType type, const uint64_t id[2], const void* data, int len);

#ifdef __cplusplus
}
#endif
