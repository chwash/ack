.define _ioctl
.extern _ioctl
.text
_ioctl:		move.w #0x36,d0
		move.w 4(sp),a0
		move.w 6(sp),d1
		ext.l  d1
		move.l 8(sp),a1
		jmp callc
