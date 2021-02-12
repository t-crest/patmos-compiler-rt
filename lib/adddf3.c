//===-- lib/adddf3.c - Double-precision addition ------------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements double-precision soft-float addition with the IEEE-754
// default rounding (to nearest, ties to even).
//
//===----------------------------------------------------------------------===//

#define DOUBLE_PRECISION
#include "fp_lib.h"

ARM_EABI_FNALIAS(dadd, adddf3)

COMPILER_RT_ABI fp_t
__adddf3(fp_t a, fp_t b) {

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
        SEL_OPERATION_REG = 0x0;
        ENABLE_REG = 0x1;
        while((STATUS_REG & 0x1) != 0x1){continue;}
        // const union { rep_t b; fp_t f; } hard_res = {.b =0x12345678FFFFFFFF};
        const union { rep_t b; fp_t f; } hard_res = {.b =((rep_t) RESULT_HI_REG << 32) + (rep_t) RESULT_LO_REG};
        return hard_res.f;
    } else {
        rep_t aRep = toRep(a);
        rep_t bRep = toRep(b);
        const rep_t aAbs = aRep & absMask;
        const rep_t bAbs = bRep & absMask;
        
        // Detect if a or b is zero, infinity, or NaN.
        if (aAbs - 1U >= infRep - 1U || bAbs - 1U >= infRep - 1U) {
            
            // NaN + anything = qNaN
            if (aAbs > infRep) return fromRep(toRep(a) | quietBit);
            // anything + NaN = qNaN
            if (bAbs > infRep) return fromRep(toRep(b) | quietBit);
            
            if (aAbs == infRep) {
                // +/-infinity + -/+infinity = qNaN
                if ((toRep(a) ^ toRep(b)) == signBit) return fromRep(qnanRep);
                // +/-infinity + anything remaining = +/- infinity
                else return a;
            }
            
            // anything remaining + +/-infinity = +/-infinity
            if (bAbs == infRep) return b;
            
            // zero + anything = anything
            if (!aAbs) {
                // but we need to get the sign right for zero + zero
                if (!bAbs) return fromRep(toRep(a) & toRep(b));
                else return b;
            }
            
            // anything + zero = anything
            if (!bAbs) return a;
        }
        
        // Swap a and b if necessary so that a has the larger absolute value.
        if (bAbs > aAbs) {
            const rep_t temp = aRep;
            aRep = bRep;
            bRep = temp;
        }
        
        // Extract the exponent and significand from the (possibly swapped) a and b.
        int aExponent = aRep >> significandBits & maxExponent;
        int bExponent = bRep >> significandBits & maxExponent;
        rep_t aSignificand = aRep & significandMask;
        rep_t bSignificand = bRep & significandMask;
        
        // Normalize any denormals, and adjust the exponent accordingly.
        if (aExponent == 0) aExponent = normalize(&aSignificand);
        if (bExponent == 0) bExponent = normalize(&bSignificand);
        
        // The sign of the result is the sign of the larger operand, a.  If they
        // have opposite signs, we are performing a subtraction; otherwise addition.
        const rep_t resultSign = aRep & signBit;
        const bool subtraction = (aRep ^ bRep) & signBit;
        
        // Shift the significands to give us round, guard and sticky, and or in the
        // implicit significand bit.  (If we fell through from the denormal path it
        // was already set by normalize( ), but setting it twice won't hurt
        // anything.)
        aSignificand = (aSignificand | implicitBit) << 3;
        bSignificand = (bSignificand | implicitBit) << 3;
        
        // Shift the significand of b by the difference in exponents, with a sticky
        // bottom bit to get rounding correct.
        const unsigned int align = aExponent - bExponent;
        if (align) {
            if (align < typeWidth) {
                const bool sticky = bSignificand << (typeWidth - align);
                bSignificand = bSignificand >> align | sticky;
            } else {
                bSignificand = 1; // sticky; b is known to be non-zero.
            }
        }
        
        if (subtraction) {
            aSignificand -= bSignificand;
            
            // If a == -b, return +zero.
            if (aSignificand == 0) return fromRep(0);
            
            // If partial cancellation occured, we need to left-shift the result
            // and adjust the exponent:
            if (aSignificand < implicitBit << 3) {
                const int shift = rep_clz(aSignificand) - rep_clz(implicitBit << 3);
                aSignificand <<= shift;
                aExponent -= shift;
            }
        }
        
        else /* addition */ {
            aSignificand += bSignificand;
            
            // If the addition carried up, we need to right-shift the result and
            // adjust the exponent:
            if (aSignificand & implicitBit << 4) {
                const bool sticky = aSignificand & 1;
                aSignificand = aSignificand >> 1 | sticky;
                aExponent += 1;
            }
        }
        
        // If we have overflowed the type, return +/- infinity:
        if (aExponent >= maxExponent) return fromRep(infRep | resultSign);
        
        if (aExponent <= 0) {
            // Result is denormal before rounding; the exponent is zero and we
            // need to shift the significand.
            const int shift = 1 - aExponent;
            const bool sticky = aSignificand << (typeWidth - shift);
            aSignificand = aSignificand >> shift | sticky;
            aExponent = 0;
        }
        
        // Low three bits are round, guard, and sticky.
        const int roundGuardSticky = aSignificand & 0x7;
        
        // Shift the significand into place, and mask off the implicit bit.
        rep_t result = aSignificand >> 3 & significandMask;
        
        // Insert the exponent and sign.
        result |= (rep_t)aExponent << significandBits;
        result |= resultSign;
        
        // Final rounding.  The result may overflow to infinity, but that is the
        // correct result in that case.
        if (roundGuardSticky > 0x4) result++;
        if (roundGuardSticky == 0x4) result += result & 1;
        return fromRep(result);
    }
}
