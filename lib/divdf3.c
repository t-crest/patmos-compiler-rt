//===-- lib/divdf3.c - Double-precision division ------------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements double-precision soft-float division
// with the IEEE-754 default rounding (to nearest, ties to even).
//
//===----------------------------------------------------------------------===//

#define DOUBLE_PRECISION
#include "fp_lib.h"

ARM_EABI_FNALIAS(ddiv, divdf3)

fp_t __divdf3(fp_t a, fp_t b) {

    if (__USE_HWFPU__) {
        #define _SPM __attribute__((address_space(1)))
        #define FPU_BASE          0xf0040000
        #define OPERAND_A_LO_REG  *((volatile _SPM unsigned int *) (FPU_BASE + 0x0))
        #define OPERAND_A_HI_REG  *((volatile _SPM unsigned int *) (FPU_BASE + 0x4))
        #define OPERAND_B_LO_REG  *((volatile _SPM unsigned int *) (FPU_BASE + 0x8))
        #define OPERAND_B_HI_REG  *((volatile _SPM unsigned int *) (FPU_BASE + 0xC))
        #define ENABLE_REG        *((volatile _SPM unsigned int *) (FPU_BASE + 0x10))
        #define SEL_OPERATION_REG *((volatile _SPM unsigned int *) (FPU_BASE + 0x14))
        #define ROUND_MODE_REG    *((volatile _SPM unsigned int *) (FPU_BASE + 0x18))
        #define STATUS_REG        *((volatile _SPM unsigned int *) (FPU_BASE + 0x1C))
        #define RESULT_LO_REG     *((volatile _SPM unsigned int *) (FPU_BASE + 0x20))
        #define RESULT_HI_REG     *((volatile _SPM unsigned int *) (FPU_BASE + 0x24))

        OPERAND_A_LO_REG = loWord(toRep(a));
        OPERAND_A_HI_REG = hiWord(toRep(a));
        OPERAND_B_LO_REG = loWord(toRep(b));
        OPERAND_B_HI_REG = hiWord(toRep(b));
        ROUND_MODE_REG = 0x0;
        SEL_OPERATION_REG = 0x3;
        ENABLE_REG = 0x1;
        while((STATUS_REG & 0x1) != 0x1){continue;}
        const union { rep_t b; fp_t f; } hard_res = {.b =((rep_t) RESULT_HI_REG << 32) + (rep_t) RESULT_LO_REG};
        return hard_res.f;
    } else {
        const unsigned int aExponent = toRep(a) >> significandBits & maxExponent;
        const unsigned int bExponent = toRep(b) >> significandBits & maxExponent;
        const rep_t quotientSign = (toRep(a) ^ toRep(b)) & signBit;
        
        rep_t aSignificand = toRep(a) & significandMask;
        rep_t bSignificand = toRep(b) & significandMask;
        int scale = 0;
        
        // Detect if a or b is zero, denormal, infinity, or NaN.
        if (aExponent-1U >= maxExponent-1U || bExponent-1U >= maxExponent-1U) {
            
            const rep_t aAbs = toRep(a) & absMask;
            const rep_t bAbs = toRep(b) & absMask;
            
            // NaN / anything = qNaN
            if (aAbs > infRep) return fromRep(toRep(a) | quietBit);
            // anything / NaN = qNaN
            if (bAbs > infRep) return fromRep(toRep(b) | quietBit);
            
            if (aAbs == infRep) {
                // infinity / infinity = NaN
                if (bAbs == infRep) return fromRep(qnanRep);
                // infinity / anything else = +/- infinity
                else return fromRep(aAbs | quotientSign);
            }
            
            // anything else / infinity = +/- 0
            if (bAbs == infRep) return fromRep(quotientSign);
            
            if (!aAbs) {
                // zero / zero = NaN
                if (!bAbs) return fromRep(qnanRep);
                // zero / anything else = +/- zero
                else return fromRep(quotientSign);
            }
            // anything else / zero = +/- infinity
            if (!bAbs) return fromRep(infRep | quotientSign);
            
            // one or both of a or b is denormal, the other (if applicable) is a
            // normal number.  Renormalize one or both of a and b, and set scale to
            // include the necessary exponent adjustment.
            if (aAbs < implicitBit) scale += normalize(&aSignificand);
            if (bAbs < implicitBit) scale -= normalize(&bSignificand);
        }
        
        // Or in the implicit significand bit.  (If we fell through from the
        // denormal path it was already set by normalize( ), but setting it twice
        // won't hurt anything.)
        aSignificand |= implicitBit;
        bSignificand |= implicitBit;
        int quotientExponent = aExponent - bExponent + scale;
        
        // Align the significand of b as a Q31 fixed-point number in the range
        // [1, 2.0) and get a Q32 approximate reciprocal using a small minimax
        // polynomial approximation: reciprocal = 3/4 + 1/sqrt(2) - b/2.  This
        // is accurate to about 3.5 binary digits.
        const uint32_t q31b = bSignificand >> 21;
        uint32_t recip32 = UINT32_C(0x7504f333) - q31b;
        
        // Now refine the reciprocal estimate using a Newton-Raphson iteration:
        //
        //     x1 = x0 * (2 - x0 * b)
        //
        // This doubles the number of correct binary digits in the approximation
        // with each iteration, so after three iterations, we have about 28 binary
        // digits of accuracy.
        uint32_t correction32;
        correction32 = -((uint64_t)recip32 * q31b >> 32);
        recip32 = (uint64_t)recip32 * correction32 >> 31;
        correction32 = -((uint64_t)recip32 * q31b >> 32);
        recip32 = (uint64_t)recip32 * correction32 >> 31;
        correction32 = -((uint64_t)recip32 * q31b >> 32);
        recip32 = (uint64_t)recip32 * correction32 >> 31;
        
        // recip32 might have overflowed to exactly zero in the preceeding
        // computation if the high word of b is exactly 1.0.  This would sabotage
        // the full-width final stage of the computation that follows, so we adjust
        // recip32 downward by one bit.
        recip32--;
        
        // We need to perform one more iteration to get us to 56 binary digits;
        // The last iteration needs to happen with extra precision.
        const uint32_t q63blo = bSignificand << 11;
        uint64_t correction, reciprocal;
        correction = -((uint64_t)recip32*q31b + ((uint64_t)recip32*q63blo >> 32));
        uint32_t cHi = correction >> 32;
        uint32_t cLo = correction;
        reciprocal = (uint64_t)recip32*cHi + ((uint64_t)recip32*cLo >> 32);
        
        // We already adjusted the 32-bit estimate, now we need to adjust the final
        // 64-bit reciprocal estimate downward to ensure that it is strictly smaller
        // than the infinitely precise exact reciprocal.  Because the computation
        // of the Newton-Raphson step is truncating at every step, this adjustment
        // is small; most of the work is already done.
        reciprocal -= 2;
        
        // The numerical reciprocal is accurate to within 2^-56, lies in the
        // interval [0.5, 1.0), and is strictly smaller than the true reciprocal
        // of b.  Multiplying a by this reciprocal thus gives a numerical q = a/b
        // in Q53 with the following properties:
        //
        //    1. q < a/b
        //    2. q is in the interval [0.5, 2.0)
        //    3. the error in q is bounded away from 2^-53 (actually, we have a
        //       couple of bits to spare, but this is all we need).
        
        // We need a 64 x 64 multiply high to compute q, which isn't a basic
        // operation in C, so we need to be a little bit fussy.
        rep_t quotient, quotientLo;
        wideMultiply(aSignificand << 2, reciprocal, &quotient, &quotientLo);
        
        // Two cases: quotient is in [0.5, 1.0) or quotient is in [1.0, 2.0).
        // In either case, we are going to compute a residual of the form
        //
        //     r = a - q*b
        //
        // We know from the construction of q that r satisfies:
        //
        //     0 <= r < ulp(q)*b
        // 
        // if r is greater than 1/2 ulp(q)*b, then q rounds up.  Otherwise, we
        // already have the correct result.  The exact halfway case cannot occur.
        // We also take this time to right shift quotient if it falls in the [1,2)
        // range and adjust the exponent accordingly.
        rep_t residual;
        if (quotient < (implicitBit << 1)) {
            residual = (aSignificand << 53) - quotient * bSignificand;
            quotientExponent--;
        } else {
            quotient >>= 1;
            residual = (aSignificand << 52) - quotient * bSignificand;
        }
        
        const int writtenExponent = quotientExponent + exponentBias;
        
        if (writtenExponent >= maxExponent) {
            // If we have overflowed the exponent, return infinity.
            return fromRep(infRep | quotientSign);
        }
        
        else if (writtenExponent < 1) {
            // Result is denormal before rounding
            //
            // If the result is so small that it just underflows to zero, return
            // a zero of the appropriate sign.  Mathematically there is no need to
            // handle this case separately, but we make it a special case to
            // simplify the shift logic.
            const unsigned int shift = 1U - (unsigned int)writtenExponent;
            if (shift >= typeWidth) return fromRep(quotientSign);
            
            // Otherwise, shift the significand of the result so that the round
            // bit is the high bit of residual.
            wideRightShiftWithSticky(&quotient, &residual, shift);

            // Rounding. We use the default IEEE-754 round-to-nearest,
            // ties-to-even rounding mode.
            rep_t absResult = quotient;
            if (residual > signBit) absResult++;
            if (residual == signBit) absResult += absResult & 1;

            // Insert the sign and return
            return fromRep(absResult | quotientSign);
        }
        
        else {
            const bool round = (residual << 1) > bSignificand;
            // Clear the implicit bit
            rep_t absResult = quotient & significandMask;
            // Insert the exponent
            absResult |= (rep_t)writtenExponent << significandBits;
            // Round
            absResult += round;
            // Insert the sign and return
            const double result = fromRep(absResult | quotientSign);
            return result;
        }
    }
}
