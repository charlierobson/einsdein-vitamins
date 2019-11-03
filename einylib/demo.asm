	; no interrupt demo

	.org $100

	ld		sp,$7fff

	call	input.reset

-:	call	vdp.awaitvsync
	call	input.readrawbits
	call	input.update

	ld		a,(key1)			; get space key state
	and		3
	cp		input.JUSTRELEASED	; just released
	jr		nz,{-}

	jp		0					; restart

;
; -------------- includes --------------
;

#include "math.asm"
#include "vdp.asm"
#include "input.asm"
