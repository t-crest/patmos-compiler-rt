
unsigned __udivsi3(unsigned n, unsigned d) {
    /* This has ~430 cycles, bundling compiler should be able to bring this down to ~230 cycles 
     * Should still be better than original C without branches due to lower data dependencies 
     * TODO The compiler should be able to lower this to the code below in the future. */
    /*
    unsigned r = 0;
    unsigned q = 0;
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

    /* This implementation should have 167 cycles (including ret), can be inlined */
    /* TODO The compiler does not generate spill code for s0, since predicate clobbers are not checked.
     *      when it does, the mfs/mts and r9 clobber should be removed, the compiler should bundle this 
     *      with q = 0 / put it in the ret delay slot or eliminate it. */
    unsigned q = 0;
    asm (
	"mfs $r9 = $s0              ;; \n\t"
	"li  $r10 = 31              ; li $r12 = 1           \n\t"  // i = 31, m = 1
	"clr $r11                   ; sl $r12 = $r12, 31    \n\t"  // r = 0,  m = (1<<i)
	
	"sl $r11 = $r11, 1          ; btest $p1 = %1, $r10  \n\t"  // r <<= 1, p1 = n[i]
	"($p1) or $r11 = $r11, 1    ; cmplt $p2 = $r10, $r0 \n\t"  // r(0) = n(i), !p2 = i >= 0
	"(!$p2) br -4               ; cmpult $p1 = $r11, %2 \n\t"  // !p1 = r >= d, loop
	"(!$p1) sub $r11 = $r11, %2 ; (!$p1) or %0 = %0, $r12 \n\t"  // if (r >= d): r -= d, q |= (1<<i)
	"sub $r10 = $r10, 1         ; sr $r12 = $r12, 1     \n\t"  // i--, m = (1 << i)
	
	"mts  $s0 = $r9             ;; \n\t"

	: "=r" (q) : "r" (n), "r" (d), "0" (q)
	: "$r10", "$r11", "$r12", "$p1", "$p2",
	  "$r9"
    );
    return q;
}

