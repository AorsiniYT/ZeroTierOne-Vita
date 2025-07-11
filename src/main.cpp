#include <psp2/apputil.h>
#include <psp2/common_dialog.h>
#include <psp2/net/netctl.h>
#include <psp2/net/net.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/sysmodule.h>
#include <malloc.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "common/debugScreen.h"
#include "common/gxm_helper.h"

extern "C" int psvDebugScreenInit();

extern "C" int psvDebugScreenPrintf(const char *format, ...);

extern "C" int psvDebugScreenPuts(const char *text);

#define printf psvDebugScreenPrintf
#include "ping.h"
#include "ime.h"
#include "zerotierone.h"

#ifndef SceNetTimeval_defined
#define SceNetTimeval_defined
typedef struct SceNetTimeval {
    long tv_sec;
    long tv_usec;
} SceNetTimeval;
#endif

#define printf psvDebugScreenPrintf
#define MENU_OPTIONS 3
const char *menu[MENU_OPTIONS] = {"Join", "Ping Test", "Exit"};

// La implementación de in_cksum está en ping.cpp

void draw_menu(int selected) {
    psvDebugScreenClear(0); // Usar color negro como fondo
    printf("ZeroTierOne Vita Test\n\n");
    for (int i = 0; i < MENU_OPTIONS; ++i) {
        if (i == selected) printf("> ");
        else printf("  ");
        printf("%s\n", menu[i]);
    }
    printf("\nUsa arriba/abajo y X para seleccionar.\n");
}

int main(int argc, char *argv[]) {
    psvDebugScreenInit();
    gxm_init();
    SceAppUtilInitParam appUtilInitParam;
    SceAppUtilBootParam appUtilBootParam;
    memset(&appUtilInitParam, 0, sizeof(SceAppUtilInitParam));
    memset(&appUtilBootParam, 0, sizeof(SceAppUtilBootParam));
    sceAppUtilInit(&appUtilInitParam, &appUtilBootParam);
    SceCommonDialogConfigParam commonDialogConfigParam;
    memset(&commonDialogConfigParam, 0, sizeof(SceCommonDialogConfigParam));
    sceCommonDialogSetConfigParam(&commonDialogConfigParam);
    sceClibPrintf("[DEBUG] Iniciando ZeroTierOne Vita Test\n");

    int ret;
    void *net_mem = NULL;
    const int NET_PARAM_MEM_SIZE = 256 * 1024;

    sceClibPrintf("[NET] Cargando modulo de red (SCE_SYSMODULE_NET)...\n");
    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    if (ret < 0) {
        sceClibPrintf("[NET] ERROR: sceSysmoduleLoadModule fallo: 0x%08X\n", ret);
        return -1;
    }

    sceClibPrintf("[NET] Inicializando red (sceNetInit)...\n");
    net_mem = malloc(NET_PARAM_MEM_SIZE);
    if (!net_mem) {
        sceClibPrintf("[NET] ERROR: malloc para net_mem fallo\n");
        sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
        return -2;
    }
    SceNetInitParam netInitParam;
    netInitParam.memory = net_mem;
    netInitParam.size = NET_PARAM_MEM_SIZE;
    netInitParam.flags = 0;
    ret = sceNetInit(&netInitParam);
    if (ret < 0) {
        sceClibPrintf("[NET] ERROR: sceNetInit fallo: 0x%08X\n", ret);
        free(net_mem);
        sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
        return -3;
    }

    sceClibPrintf("[NET] Inicializando control de red (sceNetCtlInit)...\n");
    ret = sceNetCtlInit();
    if (ret < 0) {
        sceClibPrintf("[NET] ERROR: sceNetCtlInit fallo: 0x%08X\n", ret);
        sceNetTerm();
        free(net_mem);
        sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
        return -4;
    }

    // Mostrar estado de la red
    int net_state = 0;
    ret = sceNetCtlInetGetState(&net_state);
    if (ret == 0) {
        switch (net_state) {
            case SCE_NETCTL_STATE_DISCONNECTED:
                sceClibPrintf("[NET] Estado: DESCONECTADO\n");
                break;
            case SCE_NETCTL_STATE_CONNECTING:
                sceClibPrintf("[NET] Estado: CONECTANDO\n");
                break;
            case SCE_NETCTL_STATE_FINALIZING:
                sceClibPrintf("[NET] Estado: FINALIZANDO\n");
                break;
            case SCE_NETCTL_STATE_CONNECTED:
                sceClibPrintf("[NET] Estado: CONECTADO\n");
                break;
            default:
                sceClibPrintf("[NET] Estado: DESCONOCIDO (%d)\n", net_state);
        }
    } else {
        sceClibPrintf("[NET] No se pudo obtener el estado de red. ret=0x%08X\n", ret);
    }

    // Imprimir configuración de red actual usando NetCtl
    SceNetCtlInfo info;
    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info);
    if (ret == 0) {
        sceClibPrintf("[NET] IP: %s\n", info.ip_address);
    } else {
        sceClibPrintf("[NET] No se pudo obtener la IP. ret=0x%08X\n", ret);
    }

    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_SSID, &info);
    if (ret == 0) {
        sceClibPrintf("[NET] SSID: %s\n", info.ssid);
    } else {
        sceClibPrintf("[NET] No se pudo obtener el SSID. ret=0x%08X\n", ret);
    }

    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_DEFAULT_ROUTE, &info);
    if (ret == 0) {
        sceClibPrintf("[NET] Gateway: %s\n", info.default_route);
    } else {
        sceClibPrintf("[NET] No se pudo obtener el Gateway. ret=0x%08X\n", ret);
    }

    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_PRIMARY_DNS, &info);
    if (ret == 0) {
        sceClibPrintf("[NET] DNS Primario: %s\n", info.primary_dns);
    } else {
        sceClibPrintf("[NET] No se pudo obtener el DNS primario. ret=0x%08X\n", ret);
    }

    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_SECONDARY_DNS, &info);
    if (ret == 0) {
        sceClibPrintf("[NET] DNS Secundario: %s\n", info.secondary_dns);
    } else {
        sceClibPrintf("[NET] No se pudo obtener el DNS secundario. ret=0x%08X\n", ret);
    }

    SceCtrlData ctrl;
    int selected = 0;
    int running = 1;
    char ip_buffer[32] = {0};
    while (running) {
        draw_menu(selected);
        sceCtrlPeekBufferPositive(0, &ctrl, 1);
        if (ctrl.buttons & SCE_CTRL_UP) {
            selected = (selected - 1 + MENU_OPTIONS) % MENU_OPTIONS;
            sceKernelDelayThread(200*1000);
        }
        if (ctrl.buttons & SCE_CTRL_DOWN) {
            selected = (selected + 1) % MENU_OPTIONS;
            sceKernelDelayThread(200*1000);
        }
        if (ctrl.buttons & SCE_CTRL_CROSS) {
            switch (selected) {
                case 0: {
                    // Esperar a que el usuario suelte todos los botones antes de mostrar el submenú
                    do {
                        sceCtrlPeekBufferPositive(0, &ctrl, 1);
                        sceKernelDelayThread(50*1000);
                    } while (ctrl.buttons);

                    // Usar teclado IME para pedir el Network ID
                    char network_id[32] = {0};
                    int ok = ime_get_text(network_id, 31, "Network ID", "");
                    gxm_term(); // Finaliza GXM tras el IME para restaurar debugScreen
                    psvDebugScreenInit(); // Reinicia debugScreen para restaurar la consola
                    psvDebugScreenClear(0);
                    printf("[IME] Texto ingresado: %s\n", network_id);
                    sceClibPrintf("[IME] Texto ingresado: %s\n", network_id);
                    // ...solo salida por consola para pruebas...
                    if (ok && strlen(network_id) > 0) {
                        printf("\nUniéndose a la red ZeroTier: %s...\n", network_id);
                        zerotierone_join(network_id);
                        printf("\nOperación finalizada. Pulsa cualquier botón para volver al menú.\n");
                        // Esperar a que se pulse y suelte cualquier botón
                        do {
                            sceCtrlPeekBufferPositive(0, &ctrl, 1);
                            sceKernelDelayThread(100*1000);
                        } while (!(ctrl.buttons));
                        do {
                            sceCtrlPeekBufferPositive(0, &ctrl, 1);
                            sceKernelDelayThread(50*1000);
                        } while (ctrl.buttons);
                    }
                    break;
                }
                case 1: {
                    // Esperar a que el usuario suelte todos los botones antes de mostrar el submenú
                    do {
                        sceCtrlPeekBufferPositive(0, &ctrl, 1);
                        sceKernelDelayThread(50*1000);
                    } while (ctrl.buttons);

                    // Submenú de selección de IPs predefinidas
                    const char *ip_options[] = {
                        "127.0.0.1", // Localhost
                        "8.8.8.8",   // Google DNS
                        "1.1.1.1",   // Cloudflare DNS
                        "192.168.1.1", // Gateway típico
                        "Cancelar"
                    };
                    const int ip_count = sizeof(ip_options)/sizeof(ip_options[0]);
                    int ip_selected = 0;
                    int ip_menu = 1;
                    while (ip_menu) {
                        psvDebugScreenClear(0);
                        printf("Ping Test\n\nSelecciona IP destino:\n\n");
                        for (int i = 0; i < ip_count; ++i) {
                            if (i == ip_selected) printf("> %s\n", ip_options[i]);
                            else printf("  %s\n", ip_options[i]);
                        }
                        printf("\nArriba/abajo para mover, X para confirmar, O para cancelar\n");
                        sceCtrlPeekBufferPositive(0, &ctrl, 1);
                        if (ctrl.buttons & SCE_CTRL_UP) {
                            ip_selected = (ip_selected - 1 + ip_count) % ip_count;
                            // Esperar a que se suelte el botón
                            do {
                                sceCtrlPeekBufferPositive(0, &ctrl, 1);
                                sceKernelDelayThread(50*1000);
                            } while (ctrl.buttons & SCE_CTRL_UP);
                        }
                        if (ctrl.buttons & SCE_CTRL_DOWN) {
                            ip_selected = (ip_selected + 1) % ip_count;
                            do {
                                sceCtrlPeekBufferPositive(0, &ctrl, 1);
                                sceKernelDelayThread(50*1000);
                            } while (ctrl.buttons & SCE_CTRL_DOWN);
                        }
                        if (ctrl.buttons & SCE_CTRL_CIRCLE) {
                            // Esperar a que se suelte el botón
                            do {
                                sceCtrlPeekBufferPositive(0, &ctrl, 1);
                                sceKernelDelayThread(50*1000);
                            } while (ctrl.buttons & SCE_CTRL_CIRCLE);
                            ip_menu = 0; // Cancelar
                            break;
                        }
                        if (ctrl.buttons & SCE_CTRL_CROSS) {
                            // Esperar a que se suelte el botón
                            do {
                                sceCtrlPeekBufferPositive(0, &ctrl, 1);
                                sceKernelDelayThread(50*1000);
                            } while (ctrl.buttons & SCE_CTRL_CROSS);
                            if (strcmp(ip_options[ip_selected], "Cancelar") != 0) {
                                printf("\nIniciando ping a %s...\n", ip_options[ip_selected]);
                                run_ping_test(ip_options[ip_selected]);
                                printf("\nTest finalizado. Pulsa cualquier botón para volver al menú.\n");
                                // Esperar a que se pulse y suelte cualquier botón
                                do {
                                    sceCtrlPeekBufferPositive(0, &ctrl, 1);
                                    sceKernelDelayThread(100*1000);
                                } while (!(ctrl.buttons));
                                do {
                                    sceCtrlPeekBufferPositive(0, &ctrl, 1);
                                    sceKernelDelayThread(50*1000);
                                } while (ctrl.buttons);
                            }
                            ip_menu = 0;
                            break;
                        }
                        sceKernelDelayThread(50*1000);
                    }
                    break;
                }
                case 2:
                    running = 0;
                    break;
            }
        }
        sceKernelDelayThread(50*1000);
    }
    if (net_mem) {
        free(net_mem);
    }
    sceNetCtlTerm();
    sceNetTerm();
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
    gxm_term();
    sceKernelExitProcess(0);
    return 0;
}
