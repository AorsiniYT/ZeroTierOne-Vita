
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// Dibuja texto ASCII usando GXM y una fuente bitmap embebida
void gxm_draw_text(int x, int y, const char* text, uint32_t color);

#ifdef __cplusplus
}
#endif
#ifndef GXM_HELPER_H
#define GXM_HELPER_H

#include <psp2/gxm.h>
#include <psp2/display.h>
#include <psp2/kernel/sysmem.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GXM_DISPLAY_WIDTH 960
#define GXM_DISPLAY_HEIGHT 544
#define GXM_DISPLAY_STRIDE 1024
#define GXM_DISPLAY_BUFFER_COUNT 2

void gxm_init();
void gxm_term();
void gxm_swap();
void* gxm_get_backbuffer();
SceGxmSyncObject* gxm_get_sync();

#ifdef __cplusplus
}
#endif

#endif // GXM_HELPER_H
