/*	$Id: catopen.c,v 1.1.6.1 1998/04/30 16:48:57 ache Exp $ */

/*
 * Written by J.T. Conklin, 10/05/94
 * Public domain.
 */

#include <sys/cdefs.h>

#ifdef __indr_reference
__indr_reference(_catopen,catopen);
#else

#include <nl_types.h>

extern nl_catd _catopen __P((__const char *, int));

nl_catd
catopen(name, oflag)
	__const char *name;
	int oflag;
{
	return _catopen(name, oflag);
}

#endif
