/*-
 * Copyright (C) 2005 Michael J. Silbersack <silby@freebsd.org> 
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

/*
 * $FreeBSD: src/tools/regression/pipe/pipe-overcommit2.c,v 1.1.20.1 2009/04/15 03:14:26 kensmith Exp $
 * This program tests how sys_pipe.c handles the case where there
 * is ample memory to allocate a pipe, but the file descriptor
 * limit for that user has been exceeded.
 */

int main (int argc, void *argv[])

{
	int i, returnval, lastfd;
	int pipes[10000];

	for (i = 0; i < 100000; i++) {
		returnval = open(argv[0], O_RDONLY);
		if (returnval < 1)
			break; /* All descriptors exhausted. */
		else
			lastfd = returnval;
	}

	/* First falloc failure case in sys_pipe.c:pipe() */
	for (i = 0; i < 1000; i++) {
		returnval = pipe(&pipes[i]);
	}

	/*
	 * Free just one FD so that the second falloc failure
	 * case will occur.
	 */
	close(lastfd);

	for (i = 0; i < 1000; i++) {
		returnval = pipe(&pipes[i]);
	}
	printf("PASS\n");
}
