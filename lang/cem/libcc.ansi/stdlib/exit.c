/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Header$ */

#include	<stdio.h>
#include	<stdlib.h>

#define	NEXITS	32

void (*__functab[NEXITS])(void);
int __funccnt = 0;

extern void _exit(int);
/* only fflush when there is output */
int (*_fflush)(FILE *stream) = NULL;

static void
_calls(void)
{
	register int i = __funccnt;
	
	/* "Called in reversed order of their registration" */
	while (--i >= 0)
		(*__functab[i])();
}

void
exit(int status)
{
	_calls();
	if (_fflush) _fflush((FILE *)NULL) ;
	_exit(status) ;
}
