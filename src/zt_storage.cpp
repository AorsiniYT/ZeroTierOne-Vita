#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ZeroTierOne.h>

#define ZT_STORAGE_DIR "ux0:data/zerotierone/"

static void ensure_storage_dir() {
    mkdir(ZT_STORAGE_DIR, 0777);
}

// Guarda un objeto de estado en un archivo
extern "C" void zt_save_state(enum ZT_StateObjectType type, const uint64_t id[2], const void* data, int len) {
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
    FILE* f = fopen(fname, "wb");
    if (f) {
        fwrite(data, 1, len, f);
        fclose(f);
    }
}

// Recupera un objeto de estado desde archivo
extern "C" int zt_load_state(enum ZT_StateObjectType type, const uint64_t id[2], void* buf, unsigned int buflen) {
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
    if (!f) return -1;
    int rlen = fread(buf, 1, buflen, f);
    fclose(f);
    return rlen;
}
