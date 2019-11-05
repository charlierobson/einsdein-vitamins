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

; time constants to give us a 20 millisecond timer period
TC2				.equ 50
TC3				.equ 100


	.org	$100


	ld		sp,$7fff

	call	START

	di

	ld		b,3					; 3 buttons
	ld		hl,buttons			; starting here
	call	input.reset			; reset!

	ld		hl,_irqhandler		; install own handler over the system's timer3 vector
	ld		($fb06),hl

	in		a,(VDP_STAT)		; clear existing vsync flag
-:	in		a,(VDP_STAT)		; wait for next one
	rla
	jr		nc,{-}

	ld		a,$1f				; disable interrupt + timer mode + prescaler 16 + rising edge + clk starts + time constant follows + reset + control
	out		(CTC_TMR2),a		; ctc channel 2 write timer config
	ld		a,TC2
	out		(CTC_TMR2),a		; write time constant

	ld		a,$df				; enable interrupt + counter mode + (n/a) + rising edge + clk starts + time constant follows + reset + control
	out		(CTC_TMR3),a		; ctc channel 3 write timer config
	ld		a,TC3
	out		(CTC_TMR3),a		; write time constant

	ei

	; and awaaay we go.

	; result should be thin blue line. the line starts at the time the
	; timer irq fires, and ends when vsync bit is set in VDP.

	; the aim is to tighten the timer such that absolute minimum of time
	; is wasted waiting for the vblank.

	; in MESS line is a single pixel thick, one scanline.
	; on real hardware line is 2..3 scanlines and background colour glitch

loop:
	; can't use the vdp vsync as it's read (and thus reset) in the interrupt
	ld		hl,frames
	ld		a,(hl)
-:	cp		(hl)
	jr		z,{-}

	ld		b,3					; 3 buttons
	ld		hl,buttons			; starting here
	call	input.update		; update!

	ld		a,(kexit)
	and		3
	cp		1
	jr		nz,loop

	jp		0




_irqhandler:
	di
	exx							; doesn't save AF. ask me how i know.
	push	af

-:	in		a,(VDP_STAT)		; wait for VDP VSYNC bit
	rla
	jr		nc,{-}

	ld		a,COL_DBLUE			; set border black
	out		(VDP_REG),a
	ld		a,$87
	out		(VDP_REG),a

	; if we had a stable timer config that didn't drift then
	; we wouldn't need to reset the timer. but life's too short
	; to waste finding one :D so...
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

	call	input.resetpsgio
	call	input.readrawbits

	ld		hl,(frames)
	inc		hl
	ld		(frames),hl

	call	START+5

	ld		a,COL_BLACK			; set border black
	out		(VDP_REG),a
	ld		a,$87
	out		(VDP_REG),a

	pop		af
	exx
	ei
	reti


frames:
	.dw		0


buttons:
	.byte	0,%01000000,0		; exit	(space)
	.byte	5,%01000000,0		; up	(Q)
	.byte	6,%01000000,0		; down	(A)

kexit = buttons + 2
kup   = buttons + 5
kdown = buttons + 8



#include "input.asm"
#include "math.asm"
#include "PT3.asm"
#incbin "cafe.pt3"
