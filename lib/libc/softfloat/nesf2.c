/* $NetBSD: nesf2.c,v 1.1 2000/06/06 08:15:07 bjh21 Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/softfloat/nesf2.c,v 1.1.20.1 2009/04/15 03:14:26 kensmith Exp $");

flag __nesf2(float32, float32);

flag
__nesf2(float32 a, float32 b)
{

	/* libgcc1.c says a != b */
	return !float32_eq(a, b);
}
