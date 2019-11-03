;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
; use BRASS assembler: http://www.benryves.com/bin/brass/
;
.module math


; convert a bit mask to a bit position
;
; on entry
;   A holds single bit
;
; on exit
;   B holds bit number
;   P set if mask bit wasn't found (moer than one bit set)
;
masktobitpos:
	push	hl
	ld		hl,_bit2bytetbl
	ld		bc,8
	cpir
	pop		hl
	ret

_bit2bytetbl:
	.byte	128,64,32,16,8,4,2,1


.endmodule
