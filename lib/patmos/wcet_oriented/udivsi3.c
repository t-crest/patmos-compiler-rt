/* ===-- udivsi3.c - Implement __udivsi3 -----------------------------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __udivsi3 for the compiler_rt library,
 * in a WCET-oriented variant.
 *
 * ===----------------------------------------------------------------------===
 */

#include "int_lib.h"

/* Returns: a / b */
COMPILER_RT_ABI su_int
__udivsi3(su_int n, su_int d)
{
    const unsigned n_uword_bits = sizeof(su_int) * 8;
    su_int q;
    su_int r;
    unsigned sr = n_uword_bits - 1;
    q = n << 1;
    r = n >> sr;
    su_int carry = 0;
    for (; sr > 0; --sr)
    {
        /* r:q = ((r:q)  << 1) | carry */
        r = (r << 1) | (q >> (n_uword_bits - 1));
        q = (q << 1) | carry;
        /* carry = 0;
         * if (r.all >= d.all)
         * {
         *      r.all -= d.all;
         *      carry = 1;
         * }
         */
        const int s = (int)(d - r - 1) >> (n_uword_bits - 1);
        carry = s & 1;
        r -= d & s;
    }
    q = (q << 1) | carry;
    if(d > (n>>1)) {
      return (d > n) ? 0 : 1;
    } else if (d == 1) {
      return n;
    }
    return q;
}
