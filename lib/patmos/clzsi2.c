/**
 * Fast implementation of clz.
 * TODO naked attribute still generates a ret and does not work with return value.
 * TODO if we assume a != 0, then we can just test with m and add 1 to t in the last iteration, 
 *      saves one cycle.
 */
//int clz(int a) __attribute__((naked));
unsigned __clzsi2(int a) {
    unsigned t = 0;
    asm (
	"li $r10 = 1                 ; li  $r11 = 16 \n\t"          // t = 0, k = 16
	"pclr $p2                    ; sl  $r10 = $r10, $r11 \n"    // p2 = 0, m = 0x10000

	"pnot $p2 = $p2              ; cmpult $p1 = %1, $r10 \n\t" // p1 = a <= m, p2 = !p2
	"(!$p1) sr %1 = %1, $r11     ; sr  $r11 = $r11, 1 \n\t"     // if !p1: a >>= k; k = k / 2
	"sr  $r10 = $r10, $r11       ; ($p1) add %0 = %0, $r11 \n\t"  // m >>= k; if (p1) t += k
	"($p2) br -6                 ; cmpult $p1 = %1, $r10 \n\t" // p1 = a <= m, loop 3. and 4. iteration
	"(!$p1) sr %1 = %1, $r11     ; sr  $r11 = $r11, 1 \n\t"     // if !p1: a >>= k; k = k / 2
	"sr  $r10 = $r10, $r11       ; ($p1) add %0 = %0, $r11 \n\t"  // m >>= k; if (p1) t += k
	
	"sub $r10 = $r10, %1         ;; \n\t"	                    // m = m - a
	"cmplt $p1 = $r10, $r0       ; sl %0 = %0, 1 \n\t"	    // p1 = a < 0; t *= 2
	"(!$p1) add %0 = %0, $r10 \n\t"
	: "=r" (t), "=r" (a) : "0" (t), "1" (a) 
	: "$r10", "$r22", "$p1", "$p2"
    );
    return t;
}

