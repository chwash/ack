.define	putchr
.sect .text
.sect .rom
.sect .data
.sect .bss
.sect .text
! output routine in monitor
CRT	= 0x013B
! output a charcter
! entry: ascii character in a
putchr:
	push	hl
	push	bc
	ld	hl,tab
	ld	b,5
1:	cp	(hl)
	jr	z,fetch
	inc	hl
	inc	hl
	djnz	1b
2:	call	CRT
	pop	bc
	pop	hl
	ret
fetch:	inc	hl
	ld	a,(hl)
	jr	2b
! conversion table for nascom characters
tab:	.data1	0x0D,0x00
	.data1	0x1B,0x1E
	.data1	0x08,0x1D
	.data1	0x0A,0x1F
	.data1	0x7F,0x00
