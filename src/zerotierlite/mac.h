#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// Obtiene la MAC de la Vita y la guarda en mac_out (8 bytes, rellena con ceros si la MAC es de 6 bytes)
int ztl_get_mac_address(unsigned char* mac_out);
#ifdef __cplusplus
}
#endif
