/* Lab 1: the ModRM byte, with its SIB byte and displacement.
 *
 * An instruction with operands names them in the byte after the opcode:
 *
 *     ModRM   bit  7 6    5 4 3    2 1 0
 *                  mod    reg      rm
 *
 *   reg     a register operand, or three more opcode bits (m->sub)
 *   mod 11  the other operand is the register rm
 *   else    the other operand is in memory; rm names the base register,
 *           with two exceptions:
 *             rm 100           a SIB byte follows and gives base and index
 *             rm 101, mod 00   no base: 4 bytes of displacement follow, and
 *                              the address is relative to the next
 *                              instruction (%rip)
 *
 *     SIB     bit  7 6    5 4 3    2 1 0
 *                  scale  index    base
 *
 *   scale   0, 1, 2, 3 for a factor of 1, 2, 4, 8
 *   index   register number 100 (after REX.X is added) means no index
 *   base    field 101 with mod 00 means no base: 4 bytes of displacement
 *
 *   displacement   mod 01: 1 byte; mod 10: 4 bytes; mod 00: none, except
 *                  the two cases without a base; always sign-extended
 *
 * A REX prefix adds a fourth bit to three of the fields: REX.R (bit 2 of
 * rex) to reg, REX.X (bit 1) to index, REX.B (bit 0) to rm and to base.
 * The exceptions above test the 3-bit field of rm and of base, before the
 * REX bit is added: with REX.B, rm 100 still has a SIB byte (%r12 as a
 * base needs one), and rm 101 with mod 00 is still %rip. The index is
 * tested after REX.X is added: %r12 is a valid index.
 *
 *     bytes              rex   operand            fields of *m
 *     c3                 0     %ebx (mod 11)      rm 3
 *     00                 0     (%rax)             base 0
 *     40 f8              0     -0x8(%rax)         base 0, dsize 1, disp -8
 *     45 00              0     0x0(%rbp)          base 5, dsize 1, disp 0
 *     80 00 01 00 00     0     0x100(%rax)        base 0, dsize 4
 *     05 10 00 00 00     0     0x10(%rip)         rip 1, dsize 4, disp 0x10
 *     04 8b              0     (%rbx,%rcx,4)      base 3, index 1, scale 4
 *     04 24              0     (%rsp)             base 4, no index
 *     04 20              0x42  (%rax,%r12,1)      base 0, index 12, scale 1
 *     04 c5 10 00 00 00  0     0x10(,%rax,8)      no base, index 0, scale 8
 *
 * mcpu.h has the contract: modrm_t and modrm_decode. Three parts of the
 * machine call this function: the interpreter (frame/x86.c), the
 * disassembler (frame/translate.c) and the decoder of the CPU of lab 2
 * (frame/decode.c).
 */
#include "mcpu.h"

unsigned modrm_decode(const uint8_t *p, unsigned n, uint8_t rex, modrm_t *m)
{
    /* TODO: decode the ModRM byte p[0], then the SIB byte and the
     * displacement if the operand has them, and fill every field of *m.
     *
     *   1. mod, sub and reg from p[0]. With mod 3: rm, and base = index =
     *      -1, scale = 1, rip = dsize = 0, disp = 0; return 1.
     *   2. A memory operand: rm = -1, then base, index, scale and rip from
     *      the rm field, or from the SIB byte when rm is 100. Without an
     *      index, scale is 1.
     *   3. dsize from mod and from whether there is a base; disp from the
     *      next dsize bytes, little endian, sign-extended.
     *   4. Return the number of bytes used.
     *
     * Before reading a byte check that it is one of the n bytes at p;
     * return 0 if it is not.
     */

    // First, if n = 0, return 0 directly.
    if (n == 0)
        return 0;
    // Calculate the number of efficient bytes.
    unsigned byte_num = 0;
    uint8_t ModRM = p[0];
    uint8_t mod = ModRM >> 6, reg = (ModRM >> 3) & 0x7, rm = ModRM & 0x7;
    uint8_t REX_R = (rex >> 2) & 0x1, REX_X = (rex >> 1) & 0x1, REX_B = rex & 0x1;
    // Decide the mod.
    if (mod == 0x3)
    {
        m->mod = mod, m->sub = reg;
        m->reg = reg | (REX_R << 3);
        m->rm = rm | (REX_B << 3);
        m->base = m->index = -1;
        m->scale = 1;
        m->rip = m->dsize = m->disp = 0;
        byte_num = 1;
    }
    else
    {
        if (rm == 4)
        {
            if (n <= 1)
                return 0;
            uint8_t SIB = p[1];
            uint8_t scale = SIB >> 6, index = (SIB >> 3) & 0x7, base = SIB & 0x7;
            switch (mod)
            {
                long data, mask;
            case 0:
                if (base == 0x5)
                {
                    if (n <= 5)
                        return 0;
                    m->base = -1, m->dsize = 4, m->rip = 0;
                    m->dsize = 4;
                    data = 0;
                    unsigned int offset[4];
                    for (int i = 0; i < 4; i++)
                    {
                        offset[i] = p[i + 2];
                        data |= offset[i] << (8 * i);
                    }
                    mask = -((data >> 31) & 0x1);
                    mask = mask >> 32 << 32;
                    data |= mask;
                    m->disp = data;
                    byte_num = 6;
                }
                else
                {
                    m->base = base | (REX_B << 3), m->dsize = 0, m->rip = 0;
                    m->disp = 0;
                    byte_num = 2;
                }
                m->mod = mod, m->sub = reg, m->rm = -1;
                m->reg = reg | (REX_R << 3);
                if (index == 4 && REX_X == 0)
                {
                    m->index = -1;
                    m->scale = 1;
                }
                else
                {
                    m->index = index | (REX_X << 3);
                    m->scale = 1 << scale;
                }
                break;
            case 1:
                if (n <= 2)
                    return 0;
                m->base = base | (REX_B << 3);
                m->rip = 0;
                m->dsize = 1;
                data = (char)p[2];
                m->disp = data;
                m->mod = mod, m->sub = reg, m->rm = -1;
                m->reg = reg | (REX_R << 3);
                if (index == 4 && REX_X == 0)
                {
                    m->index = -1;
                    m->scale = 1;
                }
                else
                {
                    m->index = index | (REX_X << 3);
                    m->scale = 1 << scale;
                }
                byte_num = 3;
                break;
            case 2:
                if (n <= 5)
                    return 0;
                m->base = base | (REX_B << 3);
                m->rip = 0;
                m->dsize = 4;
                data = 0;
                unsigned int offset[4];
                for (int i = 0; i < 4; i++)
                {
                    offset[i] = p[i + 2];
                    data |= offset[i] << (8 * i);
                }
                mask = -((data >> 31) & 0x1);
                mask = mask >> 32 << 32;
                data |= mask;
                m->disp = data;
                m->mod = mod, m->sub = reg, m->rm = -1;
                m->reg = reg | (REX_R << 3);
                if (index == 4 && REX_X == 0)
                {
                    m->index = -1;
                    m->scale = 1;
                }
                else
                {
                    m->index = index | (REX_X << 3);
                    m->scale = 1 << scale;
                }
                byte_num = 6;
                break;
            }
        }
        else if (rm == 5)
        {
            unsigned int offset[4];
            switch (mod)
            {
                long data, mask;
            case 0:
                if (n <= 4)
                    return 0;
                m->mod = mod, m->sub = reg, m->rm = -1;
                m->reg = reg | (REX_R << 3);
                m->base = -1, m->index = -1, m->scale = 1;
                m->rip = 1;
                m->dsize = 4;
                data = 0;
                for (int i = 0; i < 4; i++)
                {
                    offset[i] = p[i + 1];
                    data |= offset[i] << (8 * i);
                }
                mask = -((data >> 31) & 0x1);
                mask = mask >> 32 << 32;
                data |= mask;
                m->disp = data;
                byte_num = 5;
                break;
            case 1:
                if (n <= 1)
                    return 0;
                m->mod = mod, m->sub = reg, m->rm = -1;
                m->reg = reg | (REX_R << 3);
                m->base = rm | (REX_B << 3);
                m->index = -1, m->scale = 1;
                m->rip = 0;
                m->dsize = 1;
                data = (char)p[1];
                m->disp = data;
                byte_num = 2;
                break;
            case 2:
                if (n <= 4)
                    return 0;
                m->mod = mod, m->sub = reg, m->rm = -1;
                m->reg = reg | (REX_R << 3);
                m->base = rm | (REX_B << 3);
                m->index = -1, m->scale = 1;
                m->rip = 0;
                m->dsize = 4;
                data = 0;
                for (int i = 0; i < 4; i++)
                {
                    offset[i] = p[i + 1];
                    data |= offset[i] << (8 * i);
                }
                mask = -((data >> 31) & 0x1);
                mask = mask >> 32 << 32;
                data |= mask;
                m->disp = data;
                byte_num = 5;
                break;
            }
        }
        else
        {
            switch (mod)
            {
                long data, mask;
            case 0:
                m->mod = mod, m->sub = reg, m->rm = -1;
                m->reg = reg | (REX_R << 3);
                m->base = rm | (REX_B << 3);
                m->index = -1, m->scale = 1;
                m->rip = 0;
                m->dsize = 0;
                m->disp = 0;
                byte_num = 1;
                break;
            case 1:
                if (n <= 1)
                    return 0;
                m->mod = mod, m->sub = reg, m->rm = -1;
                m->reg = reg | (REX_R << 3);
                m->base = rm | (REX_B << 3);
                m->index = -1, m->scale = 1;
                m->rip = 0;
                m->dsize = 1;
                data = (char)p[1];
                m->disp = data;
                byte_num = 2;
                break;
            case 2:
                if (n <= 4)
                    return 0;
                m->mod = mod, m->sub = reg, m->rm = -1;
                m->reg = reg | (REX_R << 3);
                m->base = rm | (REX_B << 3);
                m->index = -1, m->scale = 1;
                m->rip = 0;
                m->dsize = 4;
                data = 0;
                unsigned int offset[4];
                for (int i = 0; i < 4; i++)
                {
                    offset[i] = p[i + 1];
                    data |= offset[i] << (8 * i);
                }
                mask = -((data >> 31) & 0x1);
                mask = mask >> 32 << 32;
                data |= mask;
                m->disp = data;
                byte_num = 5;
                break;
            }
        }
    }
    return byte_num;
}
