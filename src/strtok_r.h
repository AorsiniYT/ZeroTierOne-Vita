// Stub portable para strtok_r en PS Vita
#pragma once
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Implementación mínima para PS Vita. No es thread-safe, solo para evitar error de compilación.
#ifdef __vita__
extern "C" char* strtok_r_vita(char* str, const char* delim, char** saveptr);
#endif
inline char* strtok_r_vita(char* str, const char* delim, char** saveptr) {
    // Si saveptr no se usa, simplemente delega a strtok
    return strtok(str, delim);
}

#ifdef __cplusplus
}
#endif
