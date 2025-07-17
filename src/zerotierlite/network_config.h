#pragma once
// Estructura y parámetros de red virtual

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char network_id[24];
    char assigned_ip[40];
    // Otros parámetros relevantes
} ztl_network_config_t;

int ztl_network_config_init(ztl_network_config_t* cfg, const char* network_id);

#ifdef __cplusplus
}
#endif
