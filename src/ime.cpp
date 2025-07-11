extern "C" {
    #include "ime.h"
}
// Vita IME y dependencias
#include <psp2/ime_dialog.h>
#include <psp2/common_dialog.h>
#include <psp2/apputil.h>
#include <psp2/kernel/threadmgr.h>
#include <string.h>
#include "common/gxm_helper.h"

extern "C" int ime_get_text(char* out, int max_len, const char* title, const char* initial) {
    uint16_t input[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1] = {0};
    SceImeDialogParam param;
    sceImeDialogParamInit(&param);
    param.supportedLanguages = SCE_IME_LANGUAGE_ENGLISH;
    param.languagesForced = SCE_TRUE;
    param.type = SCE_IME_TYPE_DEFAULT;
    param.option = 0;
    param.textBoxMode = SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT;
    uint16_t title_utf16[SCE_IME_DIALOG_MAX_TITLE_LENGTH + 1] = {0};
    uint16_t initial_utf16[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1] = {0};
    for (int i = 0; i < SCE_IME_DIALOG_MAX_TITLE_LENGTH && title[i]; ++i) title_utf16[i] = (uint16_t)title[i];
    for (int i = 0; i < SCE_IME_DIALOG_MAX_TEXT_LENGTH && initial[i]; ++i) initial_utf16[i] = (uint16_t)initial[i];
    param.title = title_utf16;
    param.maxTextLength = (max_len < SCE_IME_DIALOG_MAX_TEXT_LENGTH) ? max_len : SCE_IME_DIALOG_MAX_TEXT_LENGTH;
    param.initialText = initial_utf16;
    param.inputTextBuffer = input;
    int res = sceImeDialogInit(&param);
    if (res != SCE_OK) return 0;
    // Bucle de actualización con framebuffer GXM
    while (sceImeDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        // Limpiar el framebuffer (negro)
        memset(gxm_get_backbuffer(), 0x00, GXM_DISPLAY_HEIGHT * GXM_DISPLAY_STRIDE * 4);
        SceCommonDialogUpdateParam updateParam;
        memset(&updateParam, 0, sizeof(updateParam));
        updateParam.renderTarget.colorSurfaceData = gxm_get_backbuffer();
        updateParam.renderTarget.surfaceType = SCE_GXM_COLOR_SURFACE_LINEAR;
        updateParam.renderTarget.colorFormat = SCE_GXM_COLOR_FORMAT_A8B8G8R8;
        updateParam.renderTarget.width = GXM_DISPLAY_WIDTH;
        updateParam.renderTarget.height = GXM_DISPLAY_HEIGHT;
        updateParam.renderTarget.strideInPixels = GXM_DISPLAY_STRIDE;
        updateParam.displaySyncObject = gxm_get_sync();
        sceCommonDialogUpdate(&updateParam);
        gxm_swap();
        sceDisplayWaitVblankStart();
    }
    SceImeDialogResult result = {};
    sceImeDialogGetResult(&result);
    if (result.button == SCE_IME_DIALOG_BUTTON_ENTER) {
        int i = 0;
        for (; i < param.maxTextLength && input[i]; ++i) {
            out[i] = (char)input[i];
        }
        out[i] = '\0';
        sceImeDialogTerm();
        return 1;
    }
    sceImeDialogTerm();
    return 0;
}
