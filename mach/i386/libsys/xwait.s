.sect .text; .sect .rom; .sect .data; .sect .bss
.define __xwait
.sect .text
__xwait:
	mov	ax,7
	callf	0x7:0x0
	jb	1f
	shl	dx,0x10
	or	eax,edx
	ret
1:
	jmp	cerror
