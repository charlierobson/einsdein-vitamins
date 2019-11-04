; tight vsync demo
;
; assemble with BRASS - http://www.benryves.com/bin/brass/
;
; tabs = 4

VDP_DATA		.equ $08	; read/write data
VDP_REG			.equ $09	; write/address
VDP_STAT		.equ $09	; read

CTC_TMR2		.equ $2a
CTC_TMR3		.equ $2b

COL_BLACK		.equ $01
COL_DBLUE		.equ $04

; time constants to give us a 19.9 millisecond timer period
TC2				.equ 92
TC3				.equ 54


	.org	$100


	ld		sp,$7fff

	di

	ld		hl,_irqhandler		; install own handler over the system's timer3 vector
	ld		($fb06),hl

	ld		a,$1f				; disable interrupt + timer mode + prescaler 16 + rising edge + clk starts + time constant follows + reset + control
	out		(CTC_TMR2),a		; ctc channel 2 write timer config
	ld		a,TC2
	out		(CTC_TMR2),a		; write time constant

	ld		a,$df				; enable interrupt + counter mode + (n/a) + rising edge + clk starts + time constant follows + reset + control
	out		(CTC_TMR3),a		; ctc channel 3 write timer config
	ld		a,TC3
	out		(CTC_TMR3),a		; write time constant

	ei

	; awaaay we go.

	; result should be thin blue line. the line starts at the time the
	; timer irq fires, and ends when vsync bit is set in VDP.

	; the aim is to tighten the timer such that absolute minimum of time
	; is wasted waiting for the vblank.

loop:
	; can't use the vdp vsync as it's read (and thus reset) in the interrupt
	ld		hl,frames
	ld		a,(hl)
-:	cp		(hl)
	jr		z,{-}

	call	input.update
	ld		hl,teecee3

	ld		a,(kup)
	and		3
	cp		1
	jr		nz,{+}

	inc		(hl)

+:	ld		a,(kdown)
	and		3
	cp		1
	jr		nz,{+}

	dec		(hl)

+:	ld		a,(kexit)
	and		3
	cp		1
	jr		nz,loop

	jp		0



_irqhandler:
	di
	exx							; doesn't save AF. ask me how i know.
	push	af

	ld		a,COL_DBLUE			; set border blue
	out		(VDP_REG),a
	ld		a,$87
	out		(VDP_REG),a

-:	in		a,(VDP_STAT)		; wait for VDP VSYNC bit
	rla
	jr		nc,{-}

	ld		a,COL_BLACK			; set border black
	out		(VDP_REG),a
	ld		a,$87
	out		(VDP_REG),a

	; if we had a stable timer config that didn't drift then
	; we wouldn't need to reset the timer. but life's too short
	; to wast finding one :D so...
	;
	; reset timer to fire in 19.9 milliseconds time
	; (92*16)*54 = 79488 / 4000000 = 0.0199 sec

	ld		a,$1f
	out		(CTC_TMR2),a
	ld		a,TC2
	out		(CTC_TMR2),a

	ld		a,$df
	out		(CTC_TMR3),a
teecee3 = $+1
	ld		a,TC3
	out		(CTC_TMR3),a

	call	input.readrawbits

	ld		hl,(frames)
	inc		hl
	ld		(frames),hl

	pop		af
	exx
	ei
	reti


frames:
	.dw		0


#include "input.asm"
#include "math.asm"
