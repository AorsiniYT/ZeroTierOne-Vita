 #pragma once
#include <stddef.h>
#include <stdint.h>
// Prototipo de RIPEMD-160 portable
#include "ripemd160.h"
// Módulo de identidad: generación y manejo de claves ZeroTier

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char public_key[32];
    unsigned char private_key[64];
    unsigned char address[8]; // 40 bits, pero se usa 8 bytes para alineación
} ztl_identity_t;

int ztl_identity_generate(ztl_identity_t* id);
int ztl_identity_load(ztl_identity_t* id, const char* path);
int ztl_identity_save(const ztl_identity_t* id, const char* path);

// Convierte el address de la identidad a string hexadecimal (al menos 11 bytes)
void ztl_identity_get_id_str(const ztl_identity_t* id, char* out, size_t outlen);

#ifdef __cplusplus
}
#endif
