/* nano_quant.cpp -- all the code you write goes in this file.
 *
 * Replace the todo() call in each function with an implementation. What each
 * function must compute is in include/nano_quant.h. Keep the prototypes as
 * they are and add nothing to the header: the framework only calls what the
 * header declares.
 *
 *   make                    build nano-quant, nq2gguf and nq-selftest
 *   ./nq-selftest a q4_0    check parts A and B, group by group
 *   make test               build, make the test files, run tests/run.sh
 *
 * Do the arithmetic in float, never through double, and keep the compiler
 * flags in the Makefile.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nano_quant.h"

static void todo(const char *who) {
    fprintf(stderr, "nano_quant.cpp: %s is not implemented yet\n", who);
    exit(3);
}

/* ================= Part A: reading and assembling bits ================= */

uint64_t rd_u64le(const uint8_t *p) {
    uint64_t result = 0;
    for (int i = 0; i < 8; i++) 
        result |= (uint64_t)p[i] << (8 * i);
    return result;
}

float bf16_to_f32(uint16_t h) {
    uint32_t bits = ((uint32_t) h) << 16;
    float result = 0.0f;
    memcpy(&result, &bits, sizeof(bits));
    return result;
}

float fp16_to_f32(uint16_t h) {
    // First, spilt the bits into 3 parts.
    uint32_t bits = (uint32_t) h;
    uint32_t sign = bits >> 15;
    uint32_t exp = (bits >> 10) & 0x1f;
    uint32_t frac = bits & 0x3ff;
    uint32_t new_bits = 0;

    // Check its type
    uint32_t infinity = (sign << 31) | (0xff << 23);
    uint32_t nan = (sign << 31) | (0xff << 23) | 0x1;
    if (exp == 0x1f) {
        if (frac == 0)  new_bits = infinity;
        else new_bits = nan;
    } else if (exp == 0) {
        if (frac == 0) new_bits = sign << 31;
        else {
            int bias = 15, new_bias = 127;
            int E = 1 - bias;
            while ((frac & 0x400u) == 0) {
                frac <<= 1;
                E--;
            }
            frac &= 0x3ffu;
            int new_e = (E + new_bias) & 0xff;
            new_bits = (sign << 31) | (new_e << 23) | (frac << 13);
        }
    } else {
        int e = exp, bias = 15;
        int E = e - bias;
        int new_bias = 127;
        int new_e = E + new_bias;
        new_bits = (sign << 31) | (new_e << 23) | (frac << 13);
    }

    float result = 0;
    memcpy(&result, &new_bits, sizeof(new_bits));
    return result;
}

uint16_t f32_to_fp16(float f) {
    uint32_t bits = 0;
    memcpy(&bits, &f, sizeof(f));
    uint32_t sign = bits >> 31;
    uint32_t exp = (bits >> 23) & 0xff;
    uint32_t frac = (bits << 9) >> 9;
    uint16_t new_bits = 0;

    // Check overflow:
    uint16_t infinity = ((uint16_t)sign << 15) | (0x1f << 10);
    uint16_t nan = infinity + 1;
    if (exp == 0xff)
        if (frac != 0) return nan;
    int limit = 0x1f;
    int e = exp, bias = 127, new_bias = 15;
    int E = e - bias, new_e = E + new_bias;
    if (new_e >= limit) {
        new_bits = infinity;
    } else if (new_e <= 0) {
        if (E < -25) {
            return (uint16_t)(sign << 15);
        }
        new_e = 0;
        uint32_t full_frac = 0x800000u | frac;

        int shift = 13 + ((1 - new_bias) - E);
        uint16_t new_frac = full_frac >> shift;

        uint32_t dropped = full_frac & ((1u << shift) - 1);
        uint32_t halfway = 1u << (shift - 1);

        bool isCarry = false;
        if (dropped > halfway) {
            new_frac += 1;
            isCarry = true;
        }
        if (dropped == halfway) {
            if (new_frac & 0x1) {
                new_frac += 1;
                isCarry = true;
            }
        }

        if (isCarry && new_frac == 0x400u) {
            new_e = 1;
            new_frac = 0;
        }

        new_bits = (uint16_t)((sign << 15) | (new_e << 10) | new_frac);
    } else {
        uint16_t new_frac = frac >> 13;
        bool isCarry = false;
        if ((frac & 0x1fff) > 0x1000) {
            new_frac += 1;
            isCarry = true;
        }
        if ((frac & 0x1fff) == 0x1000)
            if (new_frac & 0x1) {
                new_frac += 1;
                isCarry = true;
            }
        if (isCarry && new_frac == 0x400u) {
            new_frac = 0;
            new_e += 1;
        }
        if (new_e >= limit) {
            new_bits = infinity;
        } else {
            new_bits = ((uint16_t)sign << 15) | (new_e << 10) | new_frac;
        }
    }

    return new_bits;
}

/* ================= Part B: three block formats ================= */

void q4_0_quantize(const float *x, uint8_t *blk) {
    float amax_elem = 0, amax = 0, abs = 0;
    for (int i = 0; i < NQ_Q4_0_BLOCK_ELEMS; i++) {
        abs = (x[i] >= 0) ? x[i] : -x[i];
        if (abs > amax) {
            amax = abs;
            amax_elem = x[i];
        }
    }
    float d = amax_elem / -8.0f;
    float id = (d != 0) ? 1.0f / d : 0.0f;
    int q[NQ_Q4_0_BLOCK_ELEMS] = {0};
    for (int i = 0; i < NQ_Q4_0_BLOCK_ELEMS; i++) {
        int elem = (int)(x[i] * id + 8.5f);
        q[i] = (elem < 15) ? elem : 15;
    }
    uint16_t code_d = f32_to_fp16(d);
    blk[0] = (uint8_t)(code_d & 0xff);
    blk[1] = (uint8_t)((code_d >> 8) & 0xff);
    for (int i = 0; i < (NQ_Q4_0_BLOCK_BYTES - 2); i++)
        blk[2 + i] = q[i] | q[i + 16] << 4;
}

void q4_0_dequantize(const uint8_t *blk, float *x) {
    uint16_t code_d = (uint16_t)blk[0] + ((uint16_t)blk[1] << 8);
    float d = fp16_to_f32(code_d);
    int q[NQ_Q4_0_BLOCK_ELEMS] = {0};
    for (int i = 0; i < NQ_Q4_0_BLOCK_BYTES - 2; i++) {
        q[i] = blk[2 + i] & 0xf;
        q[i + 16] = (blk[2 + i] >> 4) & 0xf;
    }
    for (int i = 0; i < NQ_Q4_0_BLOCK_ELEMS; i++) 
        x[i] = d * (float)(q[i] - 8);
}

void q4_1_quantize(const float *x, uint8_t *blk) {
    float lo = x[0], hi = x[0];
    for (int i = 0; i < NQ_Q4_1_BLOCK_ELEMS; i++) {
        if (lo > x[i]) lo = x[i];
        if (hi < x[i]) hi = x[i];
    }
    float d = (hi - lo) / 15.0f;
    float id = (d != 0) ? (1.0f / d) : 0.0f;
    int q[NQ_Q4_1_BLOCK_ELEMS] = {0};
    for (int i = 0; i < NQ_Q4_1_BLOCK_ELEMS; i++) {
        int elem = (int)((x[i] - lo) * id + 0.5f);
        q[i] = (elem < 15) ? elem : 15;
    }
    uint16_t code_d = f32_to_fp16(d);
    blk[0] = (uint8_t)(code_d & 0xff);
    blk[1] = (uint8_t)((code_d >> 8) & 0xff);
    uint16_t code_lo = f32_to_fp16(lo);
    blk[2] = (uint8_t)(code_lo & 0xff);
    blk[3] = (uint8_t)((code_lo >> 8) & 0xff);
    for (int i = 0; i < (NQ_Q4_1_BLOCK_BYTES - 4); i++)
        blk[4 + i] = q[i] | q[i + 16] << 4;
}

void q4_1_dequantize(const uint8_t *blk, float *x) {
    uint16_t code_d = (uint16_t)blk[0] + ((uint16_t)blk[1] << 8);
    float d = fp16_to_f32(code_d);
    uint16_t code_m = (uint16_t)blk[2] + ((uint16_t)blk[3] << 8);
    float m = fp16_to_f32(code_m);
    int q[NQ_Q4_1_BLOCK_ELEMS] = {0};
    for (int i = 0; i < NQ_Q4_1_BLOCK_BYTES - 4; i++) {
        q[i] = blk[4 + i] & 0xf;
        q[i + 16] = (blk[4 + i] >> 4) & 0xf;
    }
    for (int i = 0; i < NQ_Q4_1_BLOCK_ELEMS; i++) 
        x[i] = d * (float)(q[i]) + m;
}

/* Given. Write put_scale_min so that this function reads back what it wrote. */
void get_scale_min(int j, const uint8_t *q, uint8_t *sc, uint8_t *m) {
    if (j < 4) {
        *sc = q[j] & 63;
        *m  = q[j + 4] & 63;
    } else {
        *sc = (uint8_t)((q[j + 4] & 0xf) | ((q[j - 4] >> 6) << 4));
        *m  = (uint8_t)((q[j + 4] >>  4) | ((q[j    ] >> 6) << 4));
    }
}

void put_scale_min(int j, uint8_t *q, uint8_t sc, uint8_t m) {
    if (j < 4) {
        q[j] = (q[j] >> 6 << 6) | sc;
        q[j + 4] = (q[j + 4] >> 6 << 6) | m;
    }
    if (j >= 4) {
        q[j - 4] = (q[j - 4] & 0x3f) | (sc >> 4 << 6);
        q[j] = (q[j] & 0x3f) | (m >> 4 << 6);
        q[j + 4] = (q[j + 4] & 0xf0) | (sc & 0xf);
        q[j + 4] = (q[j + 4] & 0xf) | ((m & 0xf) << 4);
    }
}

void q4_k_quantize(const float *x, uint8_t *blk) {
    // Part1:
    float s[8] = {0.0f}, o[8] = {0.0f};
    for (int j = 0; j < 8; j++) 
        nq_q4_k_fit(x + 32*j, &s[j], &o[j]);
    float S = 0.0f, O = 0.0f;
    for (int j = 0; j < 8; j++) {
        if (s[j] > S) S = s[j];
        if (o[j] > O) O = o[j];
    }

    // Part2:
    float d = S / 63.0f;
    float dmin = O / 63.0f;

    float is = (S > 0) ? 63.0f / S : 0.0f;
    float io = (O > 0) ? 63.0f / O : 0.0f;

    float sc[8] = {0.0f}, m[8] = {0.0f};
    for (int j = 0; j < 8; j++) {
        sc[j] = nq_round(is * s[j]);
        m[j] = nq_round(io * o[j]);
    }

    // Part3:
    int i = 0, q[NQ_Q4_K_BLOCK_ELEMS] = {0};
    for (int j = 0; j < 8; j++) {
        float D = fp16_to_f32(f32_to_fp16(d)) * (float)sc[j];
        float M = fp16_to_f32(f32_to_fp16(dmin)) * (float)m[j];
        for (int k = 0; k < 32; k++) {
            int v, res = 0;
            if (D == 0) v = 0;
            else v = nq_round((x[i] + M) / D);
            if (v < 0) res = 0;
            else if (v > 15) res = 15;
            else res = v;
            q[i] = (D != 0) ? res : 0;
            i += 1;
        }
    }
    uint16_t code_d = f32_to_fp16(d);
    uint16_t code_dmin = f32_to_fp16(dmin);
    blk[0] = (uint8_t)(code_d & 0xff);
    blk[1] = (uint8_t)(code_d >> 8);
    blk[2] = (uint8_t)(code_dmin & 0xff);
    blk[3] = (uint8_t)(code_dmin >> 8);
    uint8_t tmp_blk[12] = {0};
    for (int j = 0; j < 8; j++) 
        put_scale_min(j, tmp_blk, sc[j], m[j]);
    for (int i = 0; i < 12; i++) 
        blk[4 + i] = tmp_blk[i];
    for (int g = 0; g < 4; g++)
        for (int l = 0; l < 32; l++)
            blk[16 + 32*g +l] = q[64*g + l] | (q[64*g + 32 + l] << 4);
}

void q4_k_dequantize(const uint8_t *blk, float *x) {
    int q[NQ_Q4_K_BLOCK_ELEMS] = {0};
    for (int g = 0; g < 4; g++){
        for (int l = 0; l < 32; l++) {
            q[64*g + l] = blk[16 + 32*g + l] & 0xf;
            q[64*g + 32 + l] = blk[16 + 32*g + l] >> 4;
        }
    }
    uint8_t tmp_blk[12] = {0};
    for (int i = 0; i < 12; i++)
        tmp_blk[i] = blk[4 + i];
    uint8_t sc[8], m[8];
    for (int j = 0; j < 8; j++)
        get_scale_min(j , tmp_blk, &sc[j], &m[j]);

    uint16_t code_d = (uint16_t)blk[0] | ((uint16_t)blk[1] << 8);
    uint16_t code_dmin = (uint16_t)blk[2] | ((uint16_t)blk[3] << 8);
    float d = fp16_to_f32(code_d);
    float dmin = fp16_to_f32(code_dmin);
    int i = 0;
    for (int j = 0; j < 8; j++) {
        for(int k = 0; k < 32; k++){
            x[i] = (d * sc[j]) * q[i] - (dmin * m[j]);
            i++;
        }
    }
}
