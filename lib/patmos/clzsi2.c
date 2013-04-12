/* ===-- clzsi2.c - Implement __clzsi2 -------------------------------------===
 *
 *               The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __clzsi2 for the compiler_rt library for Patmos.
 *
 * TODO naked attribute still generates a ret and does not work with return value.
 *
 * ===----------------------------------------------------------------------===
 */

#include "int_lib.h"

/* Returns: the number of leading 0-bits */

COMPILER_RT_ABI si_int
__clzsi2(si_int a)
{
    unsigned t = 0;
    asm (
        "li $r11 = 16                ; li  $r10 = 1 \n\t"            // k = 16, m = 1
        "pclr $p2                    ; sl  $r10 = $r10, $r11 \n"     // p2 = 0, m = 0x10000

        "pnot $p2 = $p2              ; cmpult $p1 = %1, $r10 \n\t"   // p1 = a <= m, p2 = !p2
        "(!$p1) sr %1 = %1, $r11     ; sr  $r11 = $r11, 1 \n\t"      // if !p1: a >>= k; k = k / 2
        "sr  $r10 = $r10, $r11       ; ($p1) add %0 = %0, $r11 \n\t" // m >>= k; if (p1) t += k
        "($p2) br -6                 ; cmpult $p1 = %1, $r10 \n\t"   // p1 = a <= m, loop 3. and 4. iteration
        "(!$p1) sr %1 = %1, $r11     ; sr  $r11 = $r11, 1 \n\t"      // if !p1: a >>= k; k = k / 2
        "sr  $r10 = $r10, $r11       ; ($p1) add %0 = %0, $r11 \n\t" // m >>= k; if (p1) t += k

        "sub $r10 = $r10, %1         ;; \n\t"                        // m = m - a
        "cmplt $p1 = $r10, $r0       ; sl %0 = %0, 1 \n\t"           // p1 = a < 0; t *= 2
        "(!$p1) add %0 = %0, $r10 \n\t"
        : "=r" (t), "=r" (a) : "1" (a), "0" (t)
        : "$r10", "$r11", "$p1", "$p2"
    );
    return t;
}

