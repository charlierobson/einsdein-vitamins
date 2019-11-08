	; no interrupt demo

	.org $100

	ld		sp,$7fff

	call	input.reset
	call	timerint.initerupts

-:	call	vdp.awaitvsync
	call	input.update

	call	input.anykeypressed
	jr		z,{-}

	jp		0					; restart

;
; -------------- includes --------------
;

#include "math.asm"
#include "vdp.asm"
#include "timerint.asm"
#include "input.asm"
