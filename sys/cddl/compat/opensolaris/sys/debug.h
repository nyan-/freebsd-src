/*-
 * Copyright (c) 2007 Pawel Jakub Dawidek <pjd@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/cddl/compat/opensolaris/sys/debug.h,v 1.2.2.1.4.1 2009/04/15 03:14:26 kensmith Exp $
 */

#ifndef _OPENSOLARIS_SYS_DEBUG_H_
#define	_OPENSOLARIS_SYS_DEBUG_H_

#ifdef _KERNEL
#include <sys/types.h>
#include <sys/systm.h>

#include_next <sys/debug.h>

#define	assfail(a, f, l)						\
	(panic("solaris assert: %s, file: %s, line: %d", (a), (f), (l)), 0)

#define	assfail3(a, lv, op, rv, f, l)					\
	panic("solaris assert: %s (0x%jx %s 0x%jx), file: %s, line: %d", \
	    (a), (uintmax_t)(lv), (op), (uintmax_t)(rv), (f), (l))
#else	/* !_KERNEL */
#include_next <sys/debug.h>
#endif

#endif	/* _OPENSOLARIS_SYS_DEBUG_H_ */
