
// Definición local mínima para compatibilidad con ZeroTierLite
typedef enum {
    ZT_STATE_OBJECT_IDENTITY_PUBLIC = 1,
    ZT_STATE_OBJECT_IDENTITY_SECRET = 2,
    ZT_STATE_OBJECT_PLANET = 3,
    ZT_STATE_OBJECT_NETWORK_CONFIG = 4
} ZT_StateObjectType;


#include <stdio.h>
#include <string.h>
#include <psp2/io/stat.h>
#include <sys/types.h>
#include <stdlib.h>
// #include <ZeroTierOne.h> // Deshabilitado temporalmente
#include "debug.h"
#include <psp2/io/fcntl.h>

#define ZT_STORAGE_DIR "ux0:data/zerotierone/"

extern "C" void ensure_storage_dir() {
    int res = sceIoMkdir(ZT_STORAGE_DIR, 0777);
    if (res >= 0 || res == 0x80010011) {
        vita_debug_log("[ZT_STORAGE] mkdir OK/existe: %s (res=%d)\n", ZT_STORAGE_DIR, res);
    } else {
        vita_debug_log("[ZT_STORAGE] mkdir ERROR: %s (res=%d)\n", ZT_STORAGE_DIR, res);
    }
}

// Guarda un objeto de estado en un archivo
extern "C" void zt_save_state(ZT_StateObjectType type, const uint64_t id[2], const void* data, int len) {
    vita_debug_log("[ZT_STORAGE] save: type=%d id0=%llx id1=%llx len=%d\n", type, id[0], id[1], len);
    ensure_storage_dir();
    char fname[256];
    switch(type) {
        case ZT_STATE_OBJECT_IDENTITY_PUBLIC:
            snprintf(fname, sizeof(fname), "%sidentity.public", ZT_STORAGE_DIR);
            break;
        case ZT_STATE_OBJECT_IDENTITY_SECRET:
            snprintf(fname, sizeof(fname), "%sidentity.secret", ZT_STORAGE_DIR);
            break;
        case ZT_STATE_OBJECT_PLANET:
            snprintf(fname, sizeof(fname), "%splanet", ZT_STORAGE_DIR);
            break;
        case ZT_STATE_OBJECT_NETWORK_CONFIG:
            snprintf(fname, sizeof(fname), "%snet_%016llx.conf", ZT_STORAGE_DIR, id[0]);
            break;
        default:
            snprintf(fname, sizeof(fname), "%sobj_%d_%016llx.dat", ZT_STORAGE_DIR, type, id[0]);
            break;
    }
    vita_debug_log("[ZT_STORAGE] fopen: %s (wb)\n", fname);
    FILE* f = fopen(fname, "wb");
    if (f) {
        vita_debug_log("[ZT_STORAGE] fwrite: ptr=%p len=%d\n", data, len);
        // Mostrar los primeros bytes en hex
        char hex[128] = {0};
        int hexlen = (len > 32) ? 32 : len;
        for (int i = 0; i < hexlen; ++i) {
            snprintf(hex + i*3, 4, "%02X ", ((unsigned char*)data)[i]);
        }
        vita_debug_log("[ZT_STORAGE] datos(hex): %s\n", hex);
        fwrite(data, 1, len, f);
        vita_debug_log("[ZT_STORAGE] fclose: %s\n", fname);
        fclose(f);
        vita_debug_log("[ZT_STORAGE] guardado: %s (%d bytes)\n", fname, len);
    } else {
        vita_debug_log("[ZT_STORAGE] ERROR al guardar: %s\n", fname);
    }
}

// Recupera un objeto de estado desde archivo
extern "C" int zt_load_state(ZT_StateObjectType type, const uint64_t id[2], void* buf, unsigned int buflen) {
    vita_debug_log("[ZT_STORAGE] load: type=%d id0=%llx id1=%llx buflen=%u\n", type, id[0], id[1], buflen);
    char fname[256];
    switch(type) {
        case ZT_STATE_OBJECT_IDENTITY_PUBLIC:
            snprintf(fname, sizeof(fname), "%sidentity.public", ZT_STORAGE_DIR);
            break;
        case ZT_STATE_OBJECT_IDENTITY_SECRET:
            snprintf(fname, sizeof(fname), "%sidentity.secret", ZT_STORAGE_DIR);
            break;
        case ZT_STATE_OBJECT_PLANET:
            snprintf(fname, sizeof(fname), "%splanet", ZT_STORAGE_DIR);
            break;
        case ZT_STATE_OBJECT_NETWORK_CONFIG:
            snprintf(fname, sizeof(fname), "%snet_%016llx.conf", ZT_STORAGE_DIR, id[0]);
            break;
        default:
            snprintf(fname, sizeof(fname), "%sobj_%d_%016llx.dat", ZT_STORAGE_DIR, type, id[0]);
            break;
    }
    FILE* f = fopen(fname, "rb");
    if (f) {
        int rlen = fread(buf, 1, buflen, f);
        // Mostrar los primeros bytes en hex
        char hex[128] = {0};
        int hexlen = (rlen > 32) ? 32 : rlen;
        for (int i = 0; i < hexlen; ++i) {
            snprintf(hex + i*3, 4, "%02X ", ((unsigned char*)buf)[i]);
        }
        vita_debug_log("[ZT_STORAGE] datos(hex): %s\n", hex);
        fclose(f);
        vita_debug_log("[ZT_STORAGE] cargado: %s (%d bytes)\n", fname, rlen);
        return rlen;
    } else {
        vita_debug_log("[ZT_STORAGE] ERROR al cargar: %s\n", fname);
    }
    return 0;
}
