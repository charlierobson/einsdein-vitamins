;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; use BRASS assembler: http://www.benryves.com/bin/brass/
;
.module mem



; find first non-space character in zero-terminated string
;
; hl -> string
;
; returns:
;  hl updated
;
-:	inc		hl
szfindfirstnonspace:
	ld		a,(hl)
	cp		' '
	jr		z,{-}
	and		a
	ret


; copy a word (string containing only alphanumeric characters) from hl -> de
;
; returns:
;  count of characters in C
;  hl, de updated
;
copyword:
	ld		c,0

-:	ld		a,(hl)
	call	isalpha				; returns with z set if character is alphanumeric
	ret		nz
	ld		(de),a
	inc		hl
	inc		de
	inc		c
	jr		{-}


; returns with Z set if char is alphanumeric 0..9, A-Z, a-z
;
isalpha:
	cp		123					; 'z'+1
	jr		nc,_nope
	cp		97					; 'a'
	jr		nc,_yep
	cp		91					; 'Z'+1
	jr		nc,_nope
	cp		65					; 'A'
	jr		nc,_yep
	cp		58					; '9'+1
	jr		nc,_nope
	cp		48					; '0'
	jr		nc,_yep
_nope:
	cp		'A'					; can't be this, so NZ
	ret
_yep:
	cp		a					; will always be this, so Z
	ret

isalphatest:
	ld		a,47
	call	isalpha
	ld		a,58
	call	isalpha
	ld		a,64
	call	isalpha
	ld		a,91
	call	isalpha
	ld		a,96
	call	isalpha
	ld		a,123
	call	isalpha
	ld		a,48
	call	isalpha
	ld		a,57
	call	isalpha
	ld		a,65
	call	isalpha
	ld		a,90
	call	isalpha
	ld		a,97
	call	isalpha
	ld		a,122
	call	isalpha
	ret


.endmodule
