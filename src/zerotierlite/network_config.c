#include "network_config.h"
#include <string.h>

int ztl_network_config_init(ztl_network_config_t* cfg, const char* network_id) {
    memset(cfg, 0, sizeof(ztl_network_config_t));
    if (network_id)
        strncpy(cfg->network_id, network_id, sizeof(cfg->network_id)-1);
    return 0;
}
