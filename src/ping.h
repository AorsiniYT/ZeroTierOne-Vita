#pragma once
#include <cstdint>

// Ejecuta el test de ping continuo a 8.8.8.8
#ifdef __cplusplus
extern "C" {
#endif

void run_ping_test(const char* ip_addr);

#ifdef __cplusplus
}
#endif

// Checksum RFC1071
uint16_t in_cksum(uint16_t *ptr, int nbytes);
