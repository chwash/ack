.define _creat
.extern _creat
.text
_creat:		move.w #0x8,d0
		move.l 4(sp),a0
		move.w 8(sp),d1
		ext.l  d1
		jmp call
