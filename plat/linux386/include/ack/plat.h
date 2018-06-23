/* $Source$
 * $State$
 * $Revision$
 */

#ifndef _ACK_PLAT_H
#define _ACK_PLAT_H

/* We're providing a time() system call rather than wanting a wrapper around
 * gettimeofday() in the libc. */
 
/* #define ACKCONF_TIME_IS_A_SYSCALL */

/* Linux has its own O_ definitions. */

#define ACKCONF_WANT_STANDARD_O 0
#define ACKCONF_WANT_STANDARD_SIGNALS 0

#endif
