/*-
 * Copyright (c) 2004 Robert N. M. Watson
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/tools/regression/sockets/accept_fd_leak/accept_fd_leak.c,v 1.6.10.1.6.1 2010/12/21 17:09:25 kensmith Exp $
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	LOOPS	500

volatile int quit;

static void
child_died(int sig)
{
	quit = 1;
}

/*
 * This test is intended to detect a leak of a file descriptor in the process
 * following a failed non-blocking accept.  It measures an available fd
 * baseline, then performs 1000 failing accepts, then checks to see what the
 * next fd is.  It relies on sequential fd allocation, and will test for it
 * briefly before beginning (not 100% reliable, but a good start).
 */
int
main(int argc, char *argv[])
{
	struct sockaddr_in sin;
	socklen_t size;
	pid_t child;
	int fd1, fd2, fd3, i, s;
	int status;

	printf("1..2\n");

	/*
	 * Check for sequential fd allocation, and give up early if not.
	 */
	fd1 = dup(STDIN_FILENO);
	fd2 = dup(STDIN_FILENO);
	if (fd2 != fd1 + 1)
		errx(-1, "Non-sequential fd allocation\n");

	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s == -1)
		errx(-1, "socket: %s", strerror(errno));

	bzero(&sin, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sin.sin_port = htons(8080);

	if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) != 0)
		errx(-1, "bind: %s", strerror(errno));

	if (listen(s, -1) != 0)
		errx(-1, "listen: %s", strerror(errno));

	i = fcntl(s, F_GETFL);
	if (i == -1)
		errx(-1, "ioctl(F_GETFL): %s", strerror(errno));
	i |= O_NONBLOCK;
	if (fcntl(s, F_SETFL, i) != 0)
		errx(-1, "ioctl(F_SETFL): %s", strerror(errno));
	i = fcntl(s, F_GETFL);
	if (i == -1)
		errx(-1, "ioctl(F_GETFL): %s", strerror(errno));
	if ((i & O_NONBLOCK) != O_NONBLOCK)
		errx(-1, "Failed to set O_NONBLOCK (i=0x%x)\n", i);

	for (i = 0; i < LOOPS; i++) {
		size = sizeof(sin);
		if (accept(s, (struct sockaddr *)&sin, &size) != -1)
			errx(-1, "accept succeeded\n");
		if (errno != EAGAIN)
			errx(-1, "accept: %s", strerror(errno));
	}

	/*
	 * Allocate a file descriptor and make sure it's fd2+2.  2 because
	 * we allocate an fd for the socket.
	 */
	fd3 = dup(STDIN_FILENO);
	if (fd3 != fd2 + 2)
		printf("not ok 1 - (%d, %d, %d)\n", fd1, fd2, fd3);
	else
		printf("ok 1\n");

	/*
	 * Try failing accept's w/o non-blocking where the destination
	 * address pointer is invalid.
	 */
	close(fd3);
	signal(SIGCHLD, child_died);
	child = fork();
	if (child < 0)
		errx(-1, "fork: %s", strerror(errno));

	/*
	 * Child process does 1000 connect's.
	 */
	if (child == 0) {
		bzero(&sin, sizeof(sin));
		sin.sin_len = sizeof(sin);
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		sin.sin_port = htons(8080);

		for (i = 0; i < 1000; i++) {
			s = socket(PF_INET, SOCK_STREAM, 0);
			if (s == -1)
				errx(-1, "socket: %s", strerror(errno));
			if (connect(s, (struct sockaddr *)&sin,
			    sizeof(sin)) < 0)
				errx(-1, "connect: %s", strerror(errno));
			close(s);
		}
		exit(0);
	}

	/* Reset back to a blocking socket. */
	i = fcntl(s, F_GETFL);
	if (i == -1)
		errx(-1, "ioctl(F_GETFL): %s", strerror(errno));
	i &= ~O_NONBLOCK;
	if (fcntl(s, F_SETFL, i) != 0)
		errx(-1, "ioctl(F_SETFL): %s", strerror(errno));
	i = fcntl(s, F_GETFL);
	if (i == -1)
		errx(-1, "ioctl(F_GETFL): %s", strerror(errno));
	if (i & O_NONBLOCK)
		errx(-1, "Failed to clear O_NONBLOCK (i=0x%x)\n", i);
	
	/* Do 1000 accept's with an invalid pointer. */
	for (i = 0; !quit && i < 1000; i++) {
		size = sizeof(sin);
		if (accept(s, (struct sockaddr *)(uintptr_t)(0x100),
		    &size) != -1)
			errx(-1, "accept succeeded\n");
		if (errno != EFAULT)
			errx(-1, "accept: %s", strerror(errno));
	}

	if (waitpid(child, &status, 0) < 0)
		errx(-1, "waitpid: %s", strerror(errno));
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		warnx("child process died");
	
	/*
	 * Allocate a file descriptor and make sure it's fd2+2.  2 because
	 * we allocate an fd for the socket.
	 */
	fd3 = dup(STDIN_FILENO);
	if (fd3 != fd2 + 2)
		printf("not ok 2 - (%d, %d, %d)\n", fd1, fd2, fd3);
	else
		printf("ok 2\n");

	return (0);
}
