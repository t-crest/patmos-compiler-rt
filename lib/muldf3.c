//===-- lib/muldf3.c - Double-precision multiplication ------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements double-precision soft-float multiplication
// with the IEEE-754 default rounding (to nearest, ties to even).
//
//===----------------------------------------------------------------------===//

#define DOUBLE_PRECISION
#include "fp_lib.h"

ARM_EABI_FNALIAS(dmul, muldf3)

COMPILER_RT_ABI fp_t
__muldf3(fp_t a, fp_t b) {
    
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
        SEL_OPERATION_REG = 0x2;
        ENABLE_REG = 0x1;
        while((STATUS_REG & 0x1) != 0x1){continue;}
        const union { rep_t b; fp_t f; } hard_res = {.b =((rep_t) RESULT_HI_REG << 32) + (rep_t) RESULT_LO_REG};
        return hard_res.f;
    } else {
        const unsigned int aExponent = toRep(a) >> significandBits & maxExponent;
        const unsigned int bExponent = toRep(b) >> significandBits & maxExponent;
        const rep_t productSign = (toRep(a) ^ toRep(b)) & signBit;
        
        rep_t aSignificand = toRep(a) & significandMask;
        rep_t bSignificand = toRep(b) & significandMask;
        int scale = 0;
        
        // Detect if a or b is zero, denormal, infinity, or NaN.
        if (aExponent-1U >= maxExponent-1U || bExponent-1U >= maxExponent-1U) {
            
            const rep_t aAbs = toRep(a) & absMask;
            const rep_t bAbs = toRep(b) & absMask;
            
            // NaN * anything = qNaN
            if (aAbs > infRep) return fromRep(toRep(a) | quietBit);
            // anything * NaN = qNaN
            if (bAbs > infRep) return fromRep(toRep(b) | quietBit);
            
            if (aAbs == infRep) {
                // infinity * non-zero = +/- infinity
                if (bAbs) return fromRep(aAbs | productSign);
                // infinity * zero = NaN
                else return fromRep(qnanRep);
            }
            
            if (bAbs == infRep) {
                // non-zero * infinity = +/- infinity
                if (aAbs) return fromRep(bAbs | productSign);
                // zero * infinity = NaN
                else return fromRep(qnanRep);
            }
            
            // zero * anything = +/- zero
            if (!aAbs) return fromRep(productSign);
            // anything * zero = +/- zero
            if (!bAbs) return fromRep(productSign);
            
            // one or both of a or b is denormal, the other (if applicable) is a
            // normal number.  Renormalize one or both of a and b, and set scale to
            // include the necessary exponent adjustment.
            if (aAbs < implicitBit) scale += normalize(&aSignificand);
            if (bAbs < implicitBit) scale += normalize(&bSignificand);
        }
        
        // Or in the implicit significand bit.  (If we fell through from the
        // denormal path it was already set by normalize( ), but setting it twice
        // won't hurt anything.)
        aSignificand |= implicitBit;
        bSignificand |= implicitBit;
        
        // Get the significand of a*b.  Before multiplying the significands, shift
        // one of them left to left-align it in the field.  Thus, the product will
        // have (exponentBits + 2) integral digits, all but two of which must be
        // zero.  Normalizing this result is just a conditional left-shift by one
        // and bumping the exponent accordingly.
        rep_t productHi, productLo;
        wideMultiply(aSignificand, bSignificand << exponentBits,
                    &productHi, &productLo);
        
        int productExponent = aExponent + bExponent - exponentBias + scale;
        
        // Normalize the significand, adjust exponent if needed.
        if (productHi & implicitBit) productExponent++;
        else wideLeftShift(&productHi, &productLo, 1);
        
        // If we have overflowed the type, return +/- infinity.
        if (productExponent >= maxExponent) return fromRep(infRep | productSign);
        
        if (productExponent <= 0) {
            // Result is denormal before rounding
            //
            // If the result is so small that it just underflows to zero, return
            // a zero of the appropriate sign.  Mathematically there is no need to
            // handle this case separately, but we make it a special case to
            // simplify the shift logic.
            const unsigned int shift = 1U - (unsigned int)productExponent;
            if (shift >= typeWidth) return fromRep(productSign);
            
            // Otherwise, shift the significand of the result so that the round
            // bit is the high bit of productLo.
            wideRightShiftWithSticky(&productHi, &productLo, shift);
        }
        
        else {
            // Result is normal before rounding; insert the exponent.
            productHi &= significandMask;
            productHi |= (rep_t)productExponent << significandBits;
        }
        
        // Insert the sign of the result:
        productHi |= productSign;
        
        // Final rounding.  The final result may overflow to infinity, or underflow
        // to zero, but those are the correct results in those cases.  We use the
        // default IEEE-754 round-to-nearest, ties-to-even rounding mode.
        if (productLo > signBit) productHi++;
        if (productLo == signBit) productHi += productHi & 1;
        return fromRep(productHi);
    }
}
