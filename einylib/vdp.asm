;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; use BRASS assembler: http://www.benryves.com/bin/brass/
;
.module vdp

;	7		6		5		4		3		2		1		0	bit/registr
;-------+-------+-------+-------+-------+-------+-------+-------.
;	0	|	0	|	0	|	0	|	0	|	0	|	M3	| EXTVI | R0
;-------+-------+-------+-------+-------+-------+-------+-------+
; VRAM	| BLANK | INTEN |	M1	|	M2	|	0	| SIZE	|	MAG	| R1
;-------+-------+-------+-------+-------+-------+-------+-------+
;	0	|	0	|	0	|	0	|		screen address			| R2
;-------+-------+-------+-------+-------+-------+-------+-------+
;						color table address						| R3
;-------+-------+-------+-------+-------+-------+-------+-------+
;	0	|	0	|	0	|	0	|	0	|	char. base address	| R4
;-------+-------+-------+-------+-------+-------+-------+-------+
;	0	|				Sprite table address					| R5
;-------+-------+-------+-------+-------+-------+-------+-------+
;	0	|	0	|	0	|	0	|	0	| addr sprite templates | R6
;-------+-------+-------+-------+-------+-------+-------+-------+
;		foreground colour		|	background color or frame	| R7
;-------+-------+-------+-------+-------+-------+-------+-------'


VDP_DATA		.equ $08	; read/write data
VDP_REG			.equ $09	; write/address
VDP_STAT		.equ $09	; read


COL_TRANS		.equ $00
COL_BLACK		.equ $01
COL_DGREEN		.equ $0c
COL_MGREEN		.equ $02
COL_LGREEN		.equ $03
COL_DBLUE		.equ $04
COL_LBLUE		.equ $05
COL_DRED		.equ $06
COL_MRED		.equ $08
COL_LRED		.equ $09
COL_CYAN		.equ $07
COL_MAGENTA 	.equ $0D
COL_DYELLOW 	.equ $0A
COL_LYELLOW 	.equ $0B
COL_GREY		.equ $0E
COL_WHITE		.equ $0F



; await the vertical sync pulse
;
; poll VDP's status register for the vblank bit (7). reading it clears it.
;
awaitvsync:
	push	af

-:	in		a,(VDP_STAT)
	rla
	jr		nc,{-}

	pop		af
	ret


.endmodule
