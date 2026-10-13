/* 
 * CS:APP Data Lab 
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#include "btest.h"
#include <limits.h>

/*
 * Instructions to Students:
 *
 * STEP 1: Fill in the following struct with your identifying info.
 */
team_struct team =
{
   /* Team name: Replace with either:
      Your login ID if working as a one person team
      or, ID1+ID2 where ID1 is the login ID of the first team member
      and ID2 is the login ID of the second team member */
    "111", 
   /* Student name 1: Replace with the full name of first team member */
   "111",
   /* Login ID 1: Replace with the login ID of first team member */
   "111",

   /* The following should only be changed if there are two team members */
   /* Student name 2: Full name of the second team member */
   "",
   /* Login ID 2: Login ID of the second team member */
   ""
};

#if 0
/*
 * STEP 2: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.
#endif

/*
 * STEP 3: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the btest test harness to check that your solutions produce 
 *      the correct answers. Watch out for corner cases around Tmin and Tmax.
 */
/* Copyright (C) 1991-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */

/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
    /**
     * The core formula: x = (x & mask) + ((x >> shift) & mask)
     */

    // Before calculate '1', we need to set some masks.
    int m1 = 0x55 | (0x55 << 8);    
    int m2 = 0x33 | (0x33 << 8);
    int m4 = 0x0F | (0x0F << 8);
    int m8 = 0xFF | (0xFF << 16);
    int m16 = 0xFF | (0xFF << 8);   // 0X0000FFFF
    /* Select the low bit of each 2-bit group */
    m1 = m1 | (m1 << 16);           // 0x55555555
    /* Select the low 2 bits of each 4-bit group */
    m2 = m2 | (m2 << 16);           // 0x33333333
    /* Select the low 4 bits of each 8-bit group */
    m4 = m4 | (m4 << 16);           // 0x0F0F0F0F
    /* Select the low 8 bits of each 16-bit group */
    m8 = m8 | (m8 << 16);           // 0X00FF00FF
    /* Select the low 16 bits of the word. */

    /**
     * Count set bits in each 2-bit group, then add adjecent
     * counts to form totals for 4-, 8-, 16-, and 32-bit groups.
     * Each group has enough space to hold its count, so no 
     * carry can propagate into the next group.
     */
    x = (x & m1) + ((x >> 1) & m1);
    x = (x & m2) + ((x >> 2) & m2);
    x = (x & m4) + ((x >> 4) & m4);
    x = (x & m8) + ((x >> 8) & m8);
    x = (x & m16) + ((x >> 16) & m16);

    return x;
}
/* 
 * copyLSB - set all bits of result to least significant bit of x
 *   Example: copyLSB(5) = 0xFFFFFFFF, copyLSB(6) = 0x00000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int copyLSB(int x) {
    return x << 31 >> 31;
}
/* 
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 2
 */
int evenBits(void) {
    int unit = 0x55, result;
    unit = unit | (unit << 8);
    result = unit | (unit << 16);
    return result;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
    int sign = x >> 31;
    /**
     * Turn the negetive number to positive:
     * -4 -> 4
     */
    x = x ^ sign;
    return !(x >> (n + ~0));
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
    return (x >> (n << 3)) & 0xFF;
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
    /**
     * We can express x <= y firstly, then NOT it.
     */
    int signX = x >> 31, signY = y >> 31;
    /**
     * case1: the sign of x and sign of y are different.
     */
    int diffSign = (signX & ~signY) | (signY & ~signX);
    /**
     * case2: the signs are same, we need to calculate x - y
     * Notice: we need to judge whether overflow.
     */
    int negX = ~x + 1;
    int signOfDiff = (y + negX) >> 31;
    int LessOrEqual = ~diffSign & ~signOfDiff;
    /**
     * case3: if the x is the TMin, x <= y.
     */
    int isTMin = !(x ^ (1 << 31));

    return !((isTMin | (diffSign & signX) | LessOrEqual) & 0x1);
}
/* 
 * isNonNegative - return 1 if x >= 0, return 0 otherwise 
 *   Example: isNonNegative(-1) = 0.  isNonNegative(0) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 3
 */
int isNonNegative(int x) {
    return !(x >> 31);
}
/* 
 * isNotEqual - return 0 if x == y, and 1 otherwise 
 *   Examples: isNotEqual(5,5) = 0, isNotEqual(4,5) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int isNotEqual(int x, int y) {
    return !!(x ^ y);
}
/* 
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 4 
 */
int leastBitPos(int x) {
    return x & (~x + 1);
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 1 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
    /**
     * If the x >= 0, we need no operation.
     * else, we need to mask the high n bits.
     */
    int mask = (~0x1 + 1) << (32 + (~n + 1));
    int sign = x >> 31;
    return (x >> n) ^ (mask & sign);
}
/*
 * satAdd - adds two numbers but when positive overflow occurs, returns
 *          the maximum value, and when negative overflow occurs,
 *          it returns the minimum value.
 *   Examples: satAdd(0x40000000,0x40000000) = 0x7fffffff
 *             satAdd(0x80000000,0xffffffff) = 0x80000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 30
 *   Rating: 4
 */
int satAdd(int x, int y) {
    int signX = x >> 31, signY = y >> 31;
    int signDiff = (signX & ~signY) | (signY & ~signX);
    /**
     * If the sign is different, the overflow is impossible.
     */
    int normalRes = x + y;
    int normalSign = normalRes >> 31;

    int TMin = 0x1 << 31, TMax = ~TMin;
    /**
     * If potive overflow: return TMax
     */
    int posOverflow = ~signDiff & ~signX & normalSign;   // if true, get 0xFFFFFFFF. Otherwise, get 0
    /**
     * If negetive overflow: return TMin
     */
    int negOverflow = ~signDiff & signX & ~normalSign;   // if true, get 0xFFFFFFFF, Otherwise, get 0

    return (normalRes & (~posOverflow & ~negOverflow)) | (TMax & posOverflow) | (TMin & negOverflow);
}

/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
    /**
     * Remove the redundant bits and find the MSB
     */
    int sign = x >> 31;
    int b16, b8, b4, b2, b1, b0;
    // Save the positive and NOT the negetive.
    // At the end, save 1 bit for sign bit.
    x = x ^ sign;

    // Find the high 16 bits.
    b16 = !!(x >> 16) << 4;
    x = x >> b16;
    // Find the next 8 bits.
    b8 = !!(x >> 8) << 3;
    x = x >> b8;
    // Find the next 4 bits.
    b4 = !!(x >> 4) << 2;
    x = x >> b4;
    // Find the next 2 bits.
    b2 = !!(x >> 2) << 1;
    x = x >> b2;
    // Next
    b1 = !!(x >> 1);
    x = x >> b1;
    // Save the now x (1 or 0)
    b0 = x;
    return b16 + b8 + b4 + b2 + b1 + b0 + 1;    // The last '1' is the sign bit.
}

/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
    /**
     * Core: check whether exist 1.
     */
    x |= x >> 16;
    x |= x >> 8;
    x |= x >> 4;
    x |= x >> 2;
    x |= x >> 1;
    return ~x & 0x1;
}

/* 
 * dividePower2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: dividePower2(15,1) = 7, dividePower2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int dividePower2(int x, int n) {
    int sign = x >> 31;
    int left = sign & 0x1;
    return (x + (left << n) + (~left + 1)) >> n;
}

/* 
 * bang - Convert from two's complement to sign-magnitude 
 *   where the MSB is the sign bit
 *   You can assume that x > TMin
 *   Example: bang(-5) = 0x80000005.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int bang(int x) {
    // Get the sign bit.
    int sign = x >> 31;
    // get the absolute value of x.
    x = (x ^ sign) + (sign & 0x1);
    // Set the sign bit and return the result.
    return x + (sign << 31);
}

