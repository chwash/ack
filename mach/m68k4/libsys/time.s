.define _time
.extern _time
.text
_time:		move.w #0xD,d0
		trap #0
		tst.l 4(sp)
		beq 1f
		move.l 4(sp),a0
		move.l d0,(a0)
1:
		rts
