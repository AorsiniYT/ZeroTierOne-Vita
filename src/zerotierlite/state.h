#pragma once
// Estado y eventos del cliente ZeroTierLite

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ZTL_STATE_DISCONNECTED = 0,
    ZTL_STATE_AUTHENTICATING,
    ZTL_STATE_JOINING,
    ZTL_STATE_CONNECTED,
    ZTL_STATE_ERROR
} ztl_state_t;

const char* ztl_state_str(ztl_state_t state);

#ifdef __cplusplus
}
#endif
