// Incluir el header primero para macros y linkage
#include "gxm_helper.h"
#include <psp2/gxm.h>
#include <string.h>

// Fuente bitmap 8x8 ASCII (solo caracteres imprimibles 32-127)
static const unsigned char font8x8_basic[96][8] = {
    // Solo algunos caracteres de ejemplo, el resto deben completarse para uso real
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // '!'
    {0x36,0x36,0x24,0x00,0x00,0x00,0x00,0x00}, // '"'
    // ... (debes completar el resto para fuente completa)
};

// Dibuja un carácter en el framebuffer GXM
static void gxm_draw_char(int x, int y, char c, uint32_t color) {
    if (c < 32 || c > 127) return;
    const unsigned char* glyph = font8x8_basic[c - 32];
    uint32_t* fb = (uint32_t*)gxm_get_backbuffer();
    int stride = GXM_DISPLAY_STRIDE / 4;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (glyph[row] & (1 << col)) {
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < GXM_DISPLAY_WIDTH && py >= 0 && py < GXM_DISPLAY_HEIGHT) {
                    fb[py * stride + px] = color;
                }
            }
        }
    }
}

// Dibuja texto usando la fuente bitmap
#ifdef __cplusplus
extern "C" {
#endif
void gxm_draw_text(int x, int y, const char* text, uint32_t color) {
    int cursor_x = x;
    int cursor_y = y;
    for (size_t i = 0; text[i]; ++i) {
        if (text[i] == '\n') {
            cursor_x = x;
            cursor_y += 8;
        } else {
            gxm_draw_char(cursor_x, cursor_y, text[i], color);
            cursor_x += 8;
        }
    }
}
#ifdef __cplusplus
}
#endif
// ...existing code...

static struct {
    void* data;
    SceGxmSyncObject* sync;
    SceGxmColorSurface surf;
    SceUID uid;
} dbuf[GXM_DISPLAY_BUFFER_COUNT];

static unsigned int backBufferIndex = 0;
static unsigned int frontBufferIndex = 0;

static void* dram_alloc(unsigned int size, SceUID *uid) {
    void *mem;
    *uid = sceKernelAllocMemBlock("gpu_mem", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, ((size + 256*1024-1) & ~(256*1024-1)), NULL);
    sceKernelGetMemBlockBase(*uid, &mem);
    SceGxmMemoryAttribFlags attrib = (SceGxmMemoryAttribFlags)(SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE);
    sceGxmMapMemory(mem, ((size + 256*1024-1) & ~(256*1024-1)), attrib);
    return mem;
}

static void gxm_vsync_cb(const void *callback_data) {
    SceDisplayFrameBuf framebuf;
    framebuf.size = sizeof(SceDisplayFrameBuf);
    framebuf.base = *((void **)callback_data);
    framebuf.pitch = GXM_DISPLAY_STRIDE;
    framebuf.pixelformat = 0;
    framebuf.width = GXM_DISPLAY_WIDTH;
    framebuf.height = GXM_DISPLAY_HEIGHT;
    sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_SETBUF_NEXTFRAME);
}

void gxm_init() {
    SceGxmInitializeParams params;
    params.flags = 0;
    params.displayQueueMaxPendingCount = 1;
    params.displayQueueCallback = gxm_vsync_cb;
    params.displayQueueCallbackDataSize = sizeof(void *);
    params.parameterBufferSize = SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE;
    sceGxmInitialize(&params);
    for (int i = 0; i < GXM_DISPLAY_BUFFER_COUNT; i++) {
        dbuf[i].data = dram_alloc(4 * GXM_DISPLAY_STRIDE * GXM_DISPLAY_HEIGHT, &dbuf[i].uid);
        sceGxmColorSurfaceInit(&dbuf[i].surf, SCE_GXM_COLOR_FORMAT_A8B8G8R8, SCE_GXM_COLOR_SURFACE_LINEAR, SCE_GXM_COLOR_SURFACE_SCALE_NONE, SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT, GXM_DISPLAY_WIDTH, GXM_DISPLAY_HEIGHT, GXM_DISPLAY_STRIDE, dbuf[i].data);
        sceGxmSyncObjectCreate(&dbuf[i].sync);
    }
}

void gxm_swap() {
    sceGxmPadHeartbeat(&dbuf[backBufferIndex].surf, dbuf[backBufferIndex].sync);
    sceGxmDisplayQueueAddEntry(dbuf[frontBufferIndex].sync, dbuf[backBufferIndex].sync, &dbuf[backBufferIndex].data);
    frontBufferIndex = backBufferIndex;
    backBufferIndex = (backBufferIndex + 1) % GXM_DISPLAY_BUFFER_COUNT;
}

void gxm_term() {
    sceGxmTerminate();
    for (int i = 0; i < GXM_DISPLAY_BUFFER_COUNT; ++i)
        sceKernelFreeMemBlock(dbuf[i].uid);
}

void* gxm_get_backbuffer() {
    return dbuf[backBufferIndex].data;
}

SceGxmSyncObject* gxm_get_sync() {
    return dbuf[backBufferIndex].sync;
}
