;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; use BRASS assembler: http://www.benryves.com/bin/brass/
;
.module timerint

; install 20ms timer interrupt
;
; the system chains timer channels 2 & 3 to make a 1s tick for the
; 'real time' clock. there are probably simpler ways to do this but
; we'll just override what the system does.
;
; the timers are tickes at 4mhz, we want a tick of 20ms, or 0.02s, or 80000/4000000.
; the system uses a 256 prescaler, but we can't use that so the 16x prescaler comes
; out to play
;
; 80000 = (50 * 16) * 100
;
initerupts:
	di
	ld		hl,_irqhandler		; install own handler over the system's timer3 vector
	ld		($fb06),hl

	ld		c,2ah				; io port for ctc channel 2
	ld		a,1fh				; disable interrupt + timer mode + prescaler 16 + rising edge + clk starts + time constant follows + reset + control
	ld		b,32h				; tc = 50
	out		(c),a				; timer config
	nop
	out		(c),b				; timer time constant
	inc		c					; io port for ctc channel 3
	ld		a,0dfh				; enable interrupt + counter mode + (n/a) + rising edge + clk starts + time constant follows + reset + control
	ld		b,64h				; tc = 100
	out		(c),a				; timer config
	nop
	out		(c),b				; timer time constant
	ei
	reti						; do this here in order to flush any pending interrupts. bernie rose knows, don't argue.


_irqhandler:
	di
	exx							; doesn't save AF. ask me how i know.
	push	af

	ld		hl,(framecounter)	; bump a frame counter. it's NOT sync'd to the vertical retrace,
	inc		hl					; but in practice that doesn't really matter
	ld		(framecounter),hl

	call	input.readrawbits	; read the keyboard

; do other shit here, or not. whatever.

	pop		af
	exx
	ei
	reti


framecounter:
	.dw		0


.endmodule
