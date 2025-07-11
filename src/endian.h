// Header portable para macros y funciones de endianness en PS Vita
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htole16(x) (x)
#define htole32(x) (x)
#define htole64(x) (x)
#define le16toh(x) (x)
#define le32toh(x) (x)
#define le64toh(x) (x)
#else
#define htole16(x) __builtin_bswap16(x)
#define htole32(x) __builtin_bswap32(x)
#define htole64(x) __builtin_bswap64(x)
#define le16toh(x) __builtin_bswap16(x)
#define le32toh(x) __builtin_bswap32(x)
#define le64toh(x) __builtin_bswap64(x)
#endif

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htobe16(x) (x)
#define htobe32(x) (x)
#define htobe64(x) (x)
#define be16toh(x) (x)
#define be32toh(x) (x)
#define be64toh(x) (x)
#else
#define htobe16(x) __builtin_bswap16(x)
#define htobe32(x) __builtin_bswap32(x)
#define htobe64(x) __builtin_bswap64(x)
#define be16toh(x) __builtin_bswap16(x)
#define be32toh(x) __builtin_bswap32(x)
#define be64toh(x) __builtin_bswap64(x)
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if !__has_builtin(__builtin_bswap16)
static inline uint16_t bswap16(uint16_t x) {
    return (x >> 8) | (x << 8);
}
#endif
#if !__has_builtin(__builtin_bswap32)
static inline uint32_t bswap32(uint32_t x) {
    return ((x >> 24) & 0xff) | ((x >> 8) & 0xff00) |
           ((x << 8) & 0xff0000) | ((x << 24) & 0xff000000);
}
#endif
#if !__has_builtin(__builtin_bswap64)
static inline uint64_t bswap64(uint64_t x) {
    return ((uint64_t)bswap32(x & 0xFFFFFFFF) << 32) | (bswap32(x >> 32));
}
#endif

#ifdef __cplusplus
}
#endif
