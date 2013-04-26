/*===-- udivmodsi4.c - Implement __udivmodsi4 ------------------------------===
 *
 *                    The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __udivmodsi4 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */

#include "int_lib.h"

/* Returns: n / d, *rem = n % d  */

COMPILER_RT_ABI su_int
__udivmodsi4(su_int n, su_int d, su_int* rem)
{
    unsigned r;
    unsigned q = 0;

    /* This has ~430 cycles, bundling compiler should be able to bring this down to ~230 cycles 
     * Should still be better than original C without branches due to lower data dependencies 
     * TODO The compiler should be able to lower this to the code below in the future. */
    /*
    r = 0;
    for (int i = 31; i >= 0; i--) {
	r <<= 1;
	// r[0] = n[i] -> should be p1 = btest n, i; (p1) or r = r, 1
	r |= (n >> i) & 1;
	if (r >= d) {
	    r -= d;
	    // should be bset!
	    q |= (1 << i);
	}
    }
    */

    /* This implementation should have ~170 cycles (including ret), can be inlined */
    asm (
	"{ li  $r10 = 31              ; li $r12 = 1             }\n\t"  // i = 31, m = 1
	"{ clr %1                     ; sl $r12 = $r12, 31      }\n\t"  // r = 0,  m = (1<<i)
	
	"{ sl %1 = %1, 1              ; btest $p1 = %2, $r10    }\n\t"  // r <<= 1, p1 = n[i]
	"{ ($p1) or %1 = %1, 1        ; cmplt $p2 = $r10, $r0   }\n\t"  // r(0) = n(i), !p2 = i >= 0
	"{ (!$p2) br -4               ; cmpult $p1 = %1, %3     }\n\t"  // !p1 = r >= d, loop
	"{ (!$p1) sub %1 = %1, %3     ; (!$p1) or %0 = %0, $r12 }\n\t"  // if (r >= d): r -= d, q |= (1<<i)
	"{ sub $r10 = $r10, 1         ; sr $r12 = $r12, 1       }\n\t"  // i--, m = (1 << i)
	
	: "=r" (q), "=r" (r) : "r" (n), "r" (d), "0" (q), "1" (r)
	: "$r10", "$r12", "$p1", "$p2"
    );

    *rem = r;
    return q;
}

