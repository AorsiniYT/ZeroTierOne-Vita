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
#include "debug.h"
#include "common/debugScreen.h"
#include "common/gxm_helper.h"
// #include "zt_storage.h" // Deshabilitado temporalmente

extern "C" int psvDebugScreenInit();

extern "C" int psvDebugScreenPrintf(const char *format, ...);

extern "C" int psvDebugScreenPuts(const char *text);

#define printf psvDebugScreenPrintf
#include "ping.h"
#include "ime.h"
#include "zerotieronevita.h"

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
    // Eliminar archivo de identidad para pruebas (forzar generación nueva)
    vita_debug_log("[MAIN] Eliminando archivo de identidad para pruebas...");
    remove("ux0:data/zerotierone/identity_secret.conf");
    vita_debug_log("[MAIN] main() iniciado");
    psvDebugScreenInit();
    gxm_init();
    vita_debug_log("[MAIN] Pantalla y GXM inicializados");
    SceAppUtilInitParam appUtilInitParam;
    SceAppUtilBootParam appUtilBootParam;
    memset(&appUtilInitParam, 0, sizeof(SceAppUtilInitParam));
    memset(&appUtilBootParam, 0, sizeof(SceAppUtilBootParam));
    sceAppUtilInit(&appUtilInitParam, &appUtilBootParam);
    vita_debug_log("[MAIN] AppUtil inicializado");
    SceCommonDialogConfigParam commonDialogConfigParam;
    memset(&commonDialogConfigParam, 0, sizeof(SceCommonDialogConfigParam));
    sceCommonDialogSetConfigParam(&commonDialogConfigParam);
    vita_debug_log("[DEBUG] Iniciando ZeroTierOne Vita Test");
    // --- INICIALIZACIÓN DE RED ---
    int ret;
    void *net_mem = NULL;
    const int NET_PARAM_MEM_SIZE = 256 * 1024;
    vita_debug_log("[NET] Cargando modulo de red (SCE_SYSMODULE_NET)...");

    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    vita_debug_log("[NET] sceSysmoduleLoadModule ret=%d", ret);
    if (ret < 0) {
        vita_debug_log("[NET] ERROR: sceSysmoduleLoadModule fallo: 0x%08X", ret);
        return -1;
    }

    vita_debug_log("[NET] Inicializando red (sceNetInit)...");
    net_mem = malloc(NET_PARAM_MEM_SIZE);
    vita_debug_log("[NET] malloc net_mem=%p", net_mem);
    if (!net_mem) {
        vita_debug_log("[NET] ERROR: malloc para net_mem fallo");
        sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
        return -2;
    }
    SceNetInitParam netInitParam;
    netInitParam.memory = net_mem;
    netInitParam.size = NET_PARAM_MEM_SIZE;
    netInitParam.flags = 0;
    ret = sceNetInit(&netInitParam);
    vita_debug_log("[NET] sceNetInit ret=%d", ret);
    if (ret < 0) {
        vita_debug_log("[NET] ERROR: sceNetInit fallo: 0x%08X", ret);
        free(net_mem);
        sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
        return -3;
    }

    vita_debug_log("[NET] Inicializando control de red (sceNetCtlInit)...");
    ret = sceNetCtlInit();
    vita_debug_log("[NET] sceNetCtlInit ret=%d", ret);
    if (ret < 0) {
        vita_debug_log("[NET] ERROR: sceNetCtlInit fallo: 0x%08X", ret);
        sceNetTerm();
        free(net_mem);
        sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
        return -4;
    }

    // Mostrar estado de la red
    int net_state = 0;
    ret = sceNetCtlInetGetState(&net_state);
    vita_debug_log("[NET] sceNetCtlInetGetState ret=%d, state=%d", ret, net_state);
    if (ret == 0) {
        switch (net_state) {
            case SCE_NETCTL_STATE_DISCONNECTED:
                vita_debug_log("[NET] Estado: DESCONECTADO");
                break;
            case SCE_NETCTL_STATE_CONNECTING:
                vita_debug_log("[NET] Estado: CONECTANDO");
                break;
            case SCE_NETCTL_STATE_FINALIZING:
                vita_debug_log("[NET] Estado: FINALIZANDO");
                break;
            case SCE_NETCTL_STATE_CONNECTED:
                vita_debug_log("[NET] Estado: CONECTADO");
                break;
            default:
                vita_debug_log("[NET] Estado: DESCONOCIDO (%d)", net_state);
        }
    } else {
        vita_debug_log("[NET] No se pudo obtener el estado de red. ret=0x%08X", ret);
    }

    // Inicializar ZeroTierOne después de la red
    vita_debug_log("[ZT] Inicializando ZeroTierOne...");
    zerotierone_init();
    vita_debug_log("[ZT] ZeroTierOne inicializado");

    // Imprimir configuración de red actual usando NetCtl
    SceNetCtlInfo info;
    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info);
    vita_debug_log("[NET] IP ret=%d, IP=%s", ret, (ret==0)?info.ip_address:"<error>");
    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_SSID, &info);
    vita_debug_log("[NET] SSID ret=%d, SSID=%s", ret, (ret==0)?info.ssid:"<error>");
    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_DEFAULT_ROUTE, &info);
    vita_debug_log("[NET] Gateway ret=%d, GW=%s", ret, (ret==0)?info.default_route:"<error>");
    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_PRIMARY_DNS, &info);
    vita_debug_log("[NET] DNS Primario ret=%d, DNS=%s", ret, (ret==0)?info.primary_dns:"<error>");
    ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_SECONDARY_DNS, &info);
    vita_debug_log("[NET] DNS Secundario ret=%d, DNS=%s", ret, (ret==0)?info.secondary_dns:"<error>");

    SceCtrlData ctrl;
    int selected = 0;
    int running = 1;
    char ip_buffer[32] = {0};
    while (running) {
        draw_menu(selected);
        sceCtrlPeekBufferPositive(0, &ctrl, 1);
        if (ctrl.buttons & SCE_CTRL_UP) {
            selected = (selected - 1 + MENU_OPTIONS) % MENU_OPTIONS;
            vita_debug_log("[MENU] UP, seleccionado=%d", selected);
            sceKernelDelayThread(200*1000);
        }
        if (ctrl.buttons & SCE_CTRL_DOWN) {
            selected = (selected + 1) % MENU_OPTIONS;
            vita_debug_log("[MENU] DOWN, seleccionado=%d", selected);
            sceKernelDelayThread(200*1000);
        }
        if (ctrl.buttons & SCE_CTRL_CROSS) {
            vita_debug_log("[MENU] CROSS/X presionado, opción=%d", selected);
            switch (selected) {
                case 0: {
                    // Esperar a que el usuario suelte todos los botones antes de mostrar el submenú
                    do {
                        sceCtrlPeekBufferPositive(0, &ctrl, 1);
                        sceKernelDelayThread(50*1000);
                    } while (ctrl.buttons);

                    // Network ID fijo para pruebas
                    const char* network_id = "fada62b015a9a8b1";
                    printf("\nUniéndose a la red ZeroTier: %s...\n", network_id);
                    vita_debug_log("[MAIN] Llamando a zerotierone_join(%s)", network_id);
                    zerotierone_join(network_id);
                    vita_debug_log("[MAIN] zerotierone_join finalizado");
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
                            vita_debug_log("[PING] UP, seleccionado=%d", ip_selected);
                            // Esperar a que se suelte el botón
                            do {
                                sceCtrlPeekBufferPositive(0, &ctrl, 1);
                                sceKernelDelayThread(50*1000);
                            } while (ctrl.buttons & SCE_CTRL_UP);
                        }
                        if (ctrl.buttons & SCE_CTRL_DOWN) {
                            ip_selected = (ip_selected + 1) % ip_count;
                            vita_debug_log("[PING] DOWN, seleccionado=%d", ip_selected);
                            do {
                                sceCtrlPeekBufferPositive(0, &ctrl, 1);
                                sceKernelDelayThread(50*1000);
                            } while (ctrl.buttons & SCE_CTRL_DOWN);
                        }
                        if (ctrl.buttons & SCE_CTRL_CIRCLE) {
                            vita_debug_log("[PING] Cancelar seleccionado");
                            // Esperar a que se suelte el botón
                            do {
                                sceCtrlPeekBufferPositive(0, &ctrl, 1);
                                sceKernelDelayThread(50*1000);
                            } while (ctrl.buttons & SCE_CTRL_CIRCLE);
                            ip_menu = 0; // Cancelar
                            break;
                        }
                        if (ctrl.buttons & SCE_CTRL_CROSS) {
                            vita_debug_log("[PING] CROSS/X presionado, ip=%s", ip_options[ip_selected]);
                            // Esperar a que se suelte el botón
                            do {
                                sceCtrlPeekBufferPositive(0, &ctrl, 1);
                                sceKernelDelayThread(50*1000);
                            } while (ctrl.buttons & SCE_CTRL_CROSS);
                            if (strcmp(ip_options[ip_selected], "Cancelar") != 0) {
                                printf("\nIniciando ping a %s...\n", ip_options[ip_selected]);
                                vita_debug_log("[PING] Iniciando ping a %s", ip_options[ip_selected]);
                                run_ping_test(ip_options[ip_selected]);
                                vita_debug_log("[PING] Test finalizado para %s", ip_options[ip_selected]);
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
                    vita_debug_log("[MENU] Exit seleccionado, saliendo...");
                    running = 0;
                    break;
            }
        }
        sceKernelDelayThread(50*1000);
    }
    if (net_mem) {
        vita_debug_log("[NET] Liberando memoria de red...");
        free(net_mem);
    }
    vita_debug_log("[NET] Terminando NetCtl...");
    sceNetCtlTerm();
    vita_debug_log("[NET] Terminando Net...");
    sceNetTerm();
    vita_debug_log("[NET] Unloading SCE_SYSMODULE_NET...");
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
    vita_debug_log("[MAIN] Terminando GXM...");
    gxm_term();
    vita_debug_log("[MAIN] Saliendo del proceso...");
    sceKernelExitProcess(0);
    vita_debug_log("[MAIN] main() finalizado");
    return 0;
}
