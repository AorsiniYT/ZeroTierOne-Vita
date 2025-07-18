// Implementación portable de RIPEMD-160 (versión compacta, dominio público)
// Basada en https://github.com/ogay/RIPEMD160

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "ripemd160.h"

#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static const uint32_t K[5] = {0x00000000,0x5A827999,0x6ED9EBA1,0x8F1BBCDC,0xA953FD4E};
static const uint32_t KK[5] = {0x50A28BE6,0x5C4DD124,0x6D703EF3,0x7A6D76E9,0x00000000};

static void ripemd160_compress(uint32_t* h, const uint8_t* block) {
    uint32_t A,B,C,D,E,AA,BB,CC,DD,EE,T;
    uint32_t X[16];
    for (int i=0; i<16; ++i) X[i] = ((uint32_t)block[i*4]) | ((uint32_t)block[i*4+1]<<8) | ((uint32_t)block[i*4+2]<<16) | ((uint32_t)block[i*4+3]<<24);
    A=AA=h[0]; B=BB=h[1]; C=CC=h[2]; D=DD=h[3]; E=EE=h[4];
    // Rondas (compacto, ver referencia para detalles)
    for (int j=0; j<80; ++j) {
        int r = j<16?0:j<32?1:j<48?2:j<64?3:4;
        int s[5][16] = {{11,14,15,12,5,8,7,9,11,13,14,15,6,7,9,8},{7,6,8,13,11,9,7,15,7,12,15,9,11,7,13,12},{11,13,6,7,14,9,13,15,14,8,13,6,5,12,7,5},{11,12,14,15,14,15,9,8,9,14,5,6,8,6,5,12},{9,15,5,11,6,8,13,12,5,12,13,14,11,8,5,6}};
        int o[5][16] = {{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},{7,4,13,1,10,6,15,3,12,0,9,5,2,14,11,8},{3,10,14,4,9,15,8,1,2,7,0,6,13,11,5,12},{1,9,11,10,0,8,12,4,13,3,7,15,14,5,6,2},{4,0,5,9,7,12,2,10,14,1,3,8,11,6,15,13}};
        int rr[5][16] = {{5,14,7,0,9,2,11,4,13,6,15,8,1,10,3,12},{6,11,3,7,0,13,5,10,14,15,8,12,4,9,1,2},{15,5,1,3,7,14,6,9,11,8,12,2,10,0,13,4},{8,6,4,1,3,11,15,0,5,12,2,13,9,7,10,14},{12,15,10,4,1,5,8,7,6,2,13,14,0,3,9,11}};
        int ss[5][16] = {{8,9,9,11,13,15,15,5,7,7,8,11,14,14,12,6},{9,13,15,7,12,8,9,11,7,7,12,7,6,15,13,11},{9,7,15,11,8,6,6,14,12,13,5,14,13,13,7,5},{15,5,8,11,14,14,6,14,6,9,12,9,12,5,15,8},{8,5,12,9,12,5,14,6,8,13,6,5,15,13,11,11}};
        int f;
        if (j<16) f = (B^C^D);
        else if (j<32) f = (B&(C|~D));
        else if (j<48) f = (C^(B|~D));
        else if (j<64) f = (D^(B&C));
        else f = (C^(B|~D));
        T = ROL(A + f + X[o[r][j%16]] + K[r], s[r][j%16]) + E; A=E; E=D; D=ROL(C,10); C=B; B=T;
        if (j<16) f = (BB^(CC|~DD));
        else if (j<32) f = ((BB&DD) | (CC&~DD));
        else if (j<48) f = (CC^BB^DD);
        else if (j<64) f = (DD^(BB&CC));
        else f = (CC^(BB|~DD));
        T = ROL(AA + f + X[rr[r][j%16]] + KK[r], ss[r][j%16]) + EE; AA=EE; EE=DD; DD=ROL(CC,10); CC=BB; BB=T;
    }
    T = h[1] + C + DD; h[1] = h[2] + D + EE; h[2] = h[3] + E + AA; h[3] = h[4] + A + BB; h[4] = h[0] + B + CC; h[0] = T;
}

void ripemd160(const uint8_t* msg, size_t msg_len, uint8_t out[20]) {
    uint32_t h[5] = {0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476,0xC3D2E1F0};
    uint8_t block[64] = {0};
    size_t i = 0;
    while (msg_len - i >= 64) {
        ripemd160_compress(h, msg + i);
        i += 64;
    }
    size_t rem = msg_len - i;
    memcpy(block, msg + i, rem);
    block[rem] = 0x80;
    if (rem >= 56) {
        ripemd160_compress(h, block);
        memset(block, 0, 64);
    }
    uint64_t bits = (uint64_t)msg_len * 8;
    memcpy(block + 56, &bits, 8);
    ripemd160_compress(h, block);
    for (int j=0; j<5; ++j) {
        out[j*4+0] = (h[j] >> 0) & 0xFF;
        out[j*4+1] = (h[j] >> 8) & 0xFF;
        out[j*4+2] = (h[j] >> 16) & 0xFF;
        out[j*4+3] = (h[j] >> 24) & 0xFF;
    }
}
