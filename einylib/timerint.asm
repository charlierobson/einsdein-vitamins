;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; use BRASS assembler: http://www.benryves.com/bin/brass/
;
.module timerint

; install 20ms timer interrupt
;
; the hardware
; the 4 ctc channels can work in two modes, timer and counter.
; in timer mode a channel's value is decremented once per machine
; (clock) cycle.
; in counter mode a channel's value is decremented once when its
; clk/trg input is asserted.
; on the einy channel 2's zero count/timeout output is hardwired to
; channel 3's clk/trigger input. so channel 2 can be used to clock
; channel 3. that's precisely what happens to get the 1s 'realtime'
; clock.
; channel 2 is operated in timer mode, and 3 in counter mode.
; channel 3 counts the number of times channel 2 crosses zero.
;
; toward 20ms
; the timers are ticked at 4mhz. we want a tick of 20ms, or 0.02s,
; or 80000/4000000.
; to get to this rate the /16 prescaler comes out to play:
;
; 80000 = (50 * 16) * 100
;
initerupts:
	di

	ld		hl,_irqhandler		; install own handler over the system's timer3 vector
	ld		($fb06),hl

	ld		a,$1f				; disable interrupt + timer mode + prescaler 16 + rising edge + clk starts + time constant follows + reset + control
	out		($2a),a				; io port for ctc channel 2, write timer config
	ld		a,50
	out		($2a),a				; timer time constant
	ld		a,$df				; enable interrupt + counter mode + (n/a) + rising edge + clk starts + time constant follows + reset + control
	out		($2b),a				; io port for ctc channel 3, timer config
	ld		a,100
	out		($2b),a				; timer time constant
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
