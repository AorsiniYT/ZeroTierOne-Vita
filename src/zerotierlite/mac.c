#include "mac.h"
#include <psp2/net/net.h>
#include <string.h>
#include "../debug.h"

int ztl_get_mac_address(unsigned char* mac_out) {
    SceNetEtherAddr mac;
    int ret = sceNetGetMacAddress(&mac, 0); // 0: interfaz activa
    if (ret < 0) {
        vita_debug_log("[MAC] Error obteniendo MAC con sceNetGetMacAddress: ret=%d", ret);
        memset(mac_out, 0, 8);
        return ret;
    }
    memcpy(mac_out, mac.data, 6);
    memset(mac_out+6, 0, 2); // Rellenar los 2 bytes restantes con ceros
    vita_debug_log("[MAC] MAC obtenida: %02X:%02X:%02X:%02X:%02X:%02X", mac_out[0], mac_out[1], mac_out[2], mac_out[3], mac_out[4], mac_out[5]);
    return 0;
}
