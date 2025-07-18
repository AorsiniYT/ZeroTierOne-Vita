#ifndef RIPEMD160_H
#define RIPEMD160_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
void ripemd160(const uint8_t* msg, size_t msg_len, uint8_t out[20]);
#ifdef __cplusplus
}
#endif
#endif // RIPEMD160_H
