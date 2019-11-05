;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; use BRASS assembler: http://www.benryves.com/bin/brass/
;
.module input

; for kb description & row, col vals see einstein hardware manual, fig 3.5, section 3.8

PSG_SEL			.equ $02		; latch address
PSG_RD			.equ $02		; read from psg
PSG_WR			.equ $03		; write to psg

; interpreted values for the lowest order 2 bits of a key state
;
NOTPRESSED		.equ 0
JUSTPRESSED		.equ 1
HELD			.equ 3
JUSTRELEASED	.equ 2


; raw keyboard data:
;
; to be read once per program cycle, represents key switch states
;
	.align 8
_rawkeystates:
	.ds		9


; input state data
;
; define your keyboard button requirements thus:
;
; keyboard row number, row data mask, trigger impulse / key state
; e.g:
;
;_states:
; 	.byte	0,%01000000,0		; exit	(space)
; 	.byte	5,%01000000,0		; up	(Q)
; 	.byte	6,%01000000,0		; down	(A)

; actual input impulse addresses are relative like so:
;
; kexit = _states + 2
; kup   = _states + 5
; kdown = _states + 8


; input impulse is a bit train of pressed/not pressed states
;
; bit 0 represents the latest reading
; bit 7 is the oldest reading
;
; checking the 2 lowest order bits allows the following states to be detected:
;
; 00 - key is not pressed
; 01 - key was just pressed
; 11 - key is held
; 10 - key was just released
;
; depending on the frequency of updating you can also look futher up
; the chain to tell, within reason, how long the key was held.
;
; 00111111 - key has been held for last 6 reads
;
; you should always mask off uninteresting bits
;
; 10111111 - key has been held for last 6 reads, but chain is != 00111111
; because there was a release 7 reads ago. masking with 00111111 will
; solve this glitch.



; reset input states
;
; enter with hl -> key list
; b with the number of buttons
;
reset:
	ld		de,3 		; bytes per state
	dec		hl			; adjust hl to point to key state member

-:	add		hl,de
	ld		(hl),0		; reset to never held
	djnz	{-}

	ret


; update input states
;
; enter with hl -> key list
; b with the number of buttons
;
update:
	call	_updatestate		; key 1
	djnz	update
	ret

_updatestate:
	; hl points at first input state block,
	; return from here pointing to next in list
	;
	ld		de,_rawkeystates
	ld		a,(hl)					; get key row selector
	or		e						; -> raw column data for row
	ld		e,a
	ld		a,(de)					; read column data
	inc		hl						; -> column mask
	and		(hl)					; result will be non-zero if required key is down

	inc		hl						; -> key state
	sla		(hl)					; advance time, losing oldest state, clearing latest

	and		a						; if a key was already detected A will be !0
	jr		z,{+}					; else js bit was just tested

	set		0,(hl)					; mark impulse

+:	inc		hl						; ready for next update
	ret


; read raw keyboard data
;
; should be called periodically
;
; CAVEAT: using the PSG IO for the keyboard is economical, but you
; must understand that any music or sound player either maintains
; or resets the IO direction bits in register 7. most don't.
; luckily the ay registers are readable on the einstein but some
; systems may not be so lucky.
;
readrawbits:
	ld		bc,$08fe				; 8 rows, mask in c
	ld		d,0						; will hold all key bits OR'd together
	ld		hl,_rawkeystates		; table to fill. must be on 8 byte boundary

-:	ld		a,$0e					; psg register 14, io port a, output, row select
	out		(PSG_SEL),a
	ld		a,c						; write row mask, active low: 11111110 selects row 0
	rlc		c						; next row
	out		(PSG_WR),a
	ld		a,0fh		 			; rpsg register 14, io port b, input, column data
	out		(PSG_SEL),a
	in		a,(PSG_RD)				; grab column data
	cpl			   					; active low -> hi
	ld		(hl),a					; stash column data
	inc	 	hl
	or		d						; maintain rolling digest of column bits
	ld		d,a						; can be used to quickly check if any key is pressed
	djnz	{-}

	ld		a,d						; stash digest at end of list
	ld		(_rawkeystates+8),a
	ret


; reset psg IO direction registers
;
; as stated above, the IO direction registers need to be maintained
; for keyboard IO to work. if you can't ensure the validity of these
; bits then they need to be reset after music/sfx playing but before
; keyboard reading.
;
resetpsgio:
	ld		a,7						; select reg 7
	out		(PSG_SEL),a
	nop
	in		a,(PSG_SEL)				; get reg 7's content

	and		$7f
	or		$40

	out		(PSG_WR),a				; bit 7 clear, bit 6 set
	ret



; turn a keyboard row num / col mask pair into a unique key number
;
; algo is: row * 8 + mask bit position
;
; e.g. row 3, mask 00010000
; key num = (3 * 8) + 4
;         = 28
;
; hl points to pair in memory, key num returned in a
;
rowcoltokeynum:
	ld		a,(hl)					; collect row num
	inc		hl						; -> col mask
	rlca							; row * 8
	rlca
	rlca
	push	af						; stash result
	ld		a,(hl)					; collect col mask
	push	bc
	call	math.masktobitpos		; 01000000 -> 6, 00000001 -> 0 etc
	pop		af
	or		b						; -> (row * 8) + col idx
	pop		bc
	ret


; returns with Z set if zero keys pressed, else NZ
;
anykeypressed:
	push	af
	ld		a,(_rawkeystates+8)
	or		a
	pop		af
	ret


.endmodule
