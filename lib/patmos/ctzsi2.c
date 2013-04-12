/* ===-- ctzsi2.c - Implement __ctzsi2 -------------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __ctzsi2 for the compiler_rt library for Patmos.
 *
 * TODO this is untested (and unused) as of now.
 *
 * ===----------------------------------------------------------------------===
 */

#include "int_lib.h"

/* Returns: the number of trailing 0-bits */

COMPILER_RT_ABI si_int
__ctzsi2(si_int a)
{
    /*
    // TODO This gets lowered to ~31 cycles including ret w/o bundles.
    //      We can get rid of 5 cycles if we lower select in bitcode to predicated sub instead of
    //      sub + predicated mov, with bundling this should be lowered to the code below.
    unsigned t = 32;
    if (a) t--;
    a = a & -a;
    if (a & 0x0000FFFF) t -= 16;
    if (a & 0x00FF00FF) t -= 8;
    if (a & 0x0F0F0F0F) t -= 4;
    if (a & 0x33333333) t -= 2;
    if (a & 0x55555555) t -= 1;
    */

    // This is 13 cycles plus 4 cycles prologue+epilogue, the C code should be better as those 4 cycles
    // can be bundled.
    // Note: naked functions still generate a return and it does not work with return values properly.
    unsigned t = 32;
    asm (
        "neg $r11 = %1                 \n\t"
        "cmpneq $p1 = %1, $r0        ; and $r11 = %1, $r11    \n\t" // a = a & -a;
        "and $r10 = $r11, 0xFFFF0000   \n\t"     // 64bit load
        "cmpneq $p1 = $r10, $r0      ; ($p1) sub %0 = %0, 1   \n\t" // if (a) t--;
        "and $r10 = $r11, 0xFF00FF00   \n\t"     // 64bit load
        "cmpneq $p1 =$r10, $r0       ; ($p1) sub %0 = %0, 16  \n\t" // if (a & 0xFFFF0000) t -= 16;
        "and $r10 = $r11, 0xF0F0F0F0   \n\t"     // 64bit load
        "cmpneq $p1 = $r10, $r0      ; ($p1) sub %0 = %0, 8   \n\t" // if (a & 0xFF00FF00) t -= 8;
        "and $r10 = $r11, 0xCCCCCCCC   \n\t"     // 64bit load
        "cmpneq $p1 = $r10, $r0      ; ($p1) sub %0 = %0, 4   \n\t" // if (a & 0xF0F0F0F0) t -= 4;
        "and $r10 = $r11, 0xAAAAAAAA   \n\t"     // 64bit load
        "cmpneq $p1 = $r10, $r0      ; ($p1) sub %0 = %0, 2   \n\t" // if (a & 0xCCCCCCCC) t -= 2;
        "($p1) sub %0 = %0, 1          \n\t"                        // if (a & 0xAAAAAAAA) t -= 1;
        : "=r" (t) : "r" (a), "0" (t) 
        : "$r10", "$r11"
    );

    return t;
}
