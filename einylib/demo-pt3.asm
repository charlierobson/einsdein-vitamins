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

BDOS			.equ 5


	.org	$100

	ld		(oldstack),sp
	ld		sp,stack

	; upon entry $80 is command buffer
	; $80 holds number of characters found after command name 0..n
	; $81.. characters typed after command, including space
	; $81+n 0 terminating byte

	ld		hl,$80					; start of command line in memory, contains length, then all chars following cmd name incl. space
	ld		a,(hl)
	cp		0
	jp		z,_invalidname			; no parameters

	inc		hl
	call	mem.szfindfirstnonspace	; find start of filename, skipping spaces in command line
	jp		z,_invalidname			; z set if only spaces ...

	inc		hl						; does filename have a drive spec?
	ld		a,(hl)
	dec		hl
	cp		':'
	jp		nz,_nodrivespec			; probably not

	ld		a,(hl)					; see if drive is in range '0'..'3' incl
	cp		'0'
	jp		c,_invalidname			; char is less than '0',

	cp		'3'+1
	jp		nc,_invalidname			; char is greater than '3'

	sub		'0'-1					; convert to 1..4 to match drive drive number spec
	ld		(fcb),a

	inc		hl						; skip drive spec
	inc		hl

_nodrivespec:
	ld		de,fcb+1
	call	mem.copyword			; copy word to fcb. word is sequence of alphanum chars. c is count of chars copied

	dec		c						; dec c will handle the 0 length case for us
	ld		a,7						; c should be in range 0..7 - if it is 8 or more then we've copied too much
	sub		c						; if we copied 0 or more than 8 characters it's an illegal name
	jp		c,_invalidname

	ld		a,(hl)					; check for extension
	and		a
	jr		z,_done

	cp		'.'						; was terminated by something other than '.' .. 'baint roite
	jp		nz,_invalidname

	inc		hl						; skip '.'
	ld		de,fcb+9				; finally ready for extension if there is one
	call	mem.copyword
	dec		c
	ld		a,3
	sub		c
	jp		c,_invalidname			; if we copied 0 or more than 3 characters it's an illegal name

_done:
	ld		de,fcb					; open file
	ld		c,15
	call	BDOS

	inc		a						; a = 255 if error
	jp		z,_invalidname

	ld		hl,tune					; set load buffer address
	ld		(selfmodaddr),hl

selfmodaddr = $+1
-:	ld		de,0					; retrieve modified buffer address
	ld		hl,128
	add		hl,de
	ld		(selfmodaddr),hl		; stash buffer address + 128 for next cycle
	ld		c,26
	call	BDOS					; set buffer address

	ld		de,fcb					; read data
	ld		c,20
	call	BDOS
	or		a
	jr		z,{-}

	ld		e,10					; crlf
	ld		c,2
	call	BDOS
	ld		e,13
	ld		c,2
	call	BDOS

_okreadyletsdoit:
	ld		hl,tune
	call	START+3				; initialise song

	di

	ld		b,3					; 3 buttons
	ld		hl,buttons			; starting here
	call	input.reset			; reset!

	ld		hl,($fb06)	
	ld		(oldirq),hl			; stash old vector for tidy-up

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

	; and awaaay we go.

loop:
	; can't use the vdp vsync as it's read (and thus reset) in the interrupt
	ld		hl,frames
	ld		a,(hl)
-:	cp		(hl)
	jr		z,{-}

	ld		a,(SETUP)
	rlca
	jr		c,_ended

	ld		b,3					; 3 buttons
	ld		hl,buttons			; starting here
	call	input.update		; update!

	ld		a,(kexit)
	and		3
	cp		1
	jr		nz,loop

_ended:
	di

	ld		hl,(oldirq)
	ld		($fb06),hl

	ld		a,$3f				; disable interrupt + timer mode + prescaler 16 + rising edge + clk starts + time constant follows + reset + control
	out		(CTC_TMR2),a		; ctc channel 2 write timer config
	ld		a,$7d
	out		(CTC_TMR2),a		; write time constant

	ld		a,$df				; enable interrupt + counter mode + (n/a) + rising edge + clk starts + time constant follows + reset + control
	out		(CTC_TMR3),a		; ctc channel 3 write timer config
	ld		a,$7d
	out		(CTC_TMR3),a		; write time constant

	call	START+8				; reset PSG

	ld		sp,(oldstack)

	ei
	rst		0					; warm start


_invalidname:
    .db     $cf,$cf
	.db		"Invalid filename",'.'+$80
	ret



_irqhandler:
	di
	exx							; doesn't save AF. ask me how i know.
	push	af

	call	input.resetpsgio
	call	input.readrawbits

	ld		hl,(frames)
	inc		hl
	ld		(frames),hl

	call	START+5				; play a quark

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

oldirq:
	.dw		0

#include "input.asm"
#include "math.asm"
#include "mem.asm"
#include "PT3.asm"

oldstack:
	.dw		0
stack:
	.ds		512

; put this here so if any filename copying goes horribly wrong then we overwrite data and not code
fcb:
	.db		0
	.db		"           "
	.ds		36

tune:
