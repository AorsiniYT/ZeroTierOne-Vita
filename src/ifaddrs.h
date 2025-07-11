// Stub para <ifaddrs.h> en PS Vita
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ifaddrs {
    // Estructura vacía, solo para compatibilidad
};

inline int getifaddrs(struct ifaddrs **ifap) {
    // No implementado en Vita, retorna error
    if (ifap) *ifap = 0;
    return -1;
}

inline void freeifaddrs(struct ifaddrs *ifa) {
    // No hace nada
}

#ifdef __cplusplus
}
#endif
