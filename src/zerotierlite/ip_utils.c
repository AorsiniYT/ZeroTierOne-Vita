#include <psp2/net/netctl.h>
#include <string.h>
#include <stdio.h>
#include "ip_utils.h"

int ztl_get_local_ip(char* out, size_t outlen) {
    SceNetCtlInfo info;
    int ret = sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info);
    if (ret == 0 && out && outlen >= strlen(info.ip_address) + 1) {
        strncpy(out, info.ip_address, outlen);
        out[outlen-1] = '\0';
        return 0;
    }
    if (out && outlen > 0) out[0] = '\0';
    return -1;
}
