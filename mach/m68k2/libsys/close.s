.define _close
.extern _close
.text
_close:		move.w #0x6,d0
		move.w 4(sp),a0
		jmp callc
