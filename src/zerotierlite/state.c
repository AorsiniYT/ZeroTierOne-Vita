#include "state.h"

const char* ztl_state_str(ztl_state_t state) {
    switch(state) {
        case ZTL_STATE_DISCONNECTED: return "Desconectado";
        case ZTL_STATE_AUTHENTICATING: return "Autenticando";
        case ZTL_STATE_JOINING: return "Uniendo";
        case ZTL_STATE_CONNECTED: return "Conectado";
        case ZTL_STATE_ERROR: return "Error";
        default: return "Desconocido";
    }
}
