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
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/cddl/compat/opensolaris/misc/fsshare.c,v 1.3.2.1.4.1 2009/04/15 03:14:26 kensmith Exp $");

#include <sys/param.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <libutil.h>
#include <assert.h>
#include <pathnames.h>	/* _PATH_MOUNTDPID */
#include <fsshare.h>

#define	FILE_HEADER	"# !!! DO NOT EDIT THIS FILE MANUALLY !!!\n\n"
#define	OPTSSIZE	1024
#define	MAXLINESIZE	(PATH_MAX + OPTSSIZE)

static void
restart_mountd(void)
{
	struct pidfh *pfh;
	pid_t mountdpid;

	pfh = pidfile_open(_PATH_MOUNTDPID, 0600, &mountdpid);
	if (pfh != NULL) {
		/* Mountd is not running. */
		pidfile_remove(pfh);
		return;
	}
	if (errno != EEXIST) {
		/* Cannot open pidfile for some reason. */
		return;
	}
	/* We have mountd(8) PID in mountdpid varible. */
	kill(mountdpid, SIGHUP);
}

/*
 * Read one line from a file. Skip comments, empty lines and a line with a
 * mountpoint specified in the 'skip' argument.
 */
static char *
getline(FILE *fd, const char *skip)
{
	static char line[MAXLINESIZE];
	size_t len, skiplen;
	char *s, last;

	if (skip != NULL)
		skiplen = strlen(skip);
	for (;;) {
		s = fgets(line, sizeof(line), fd);
		if (s == NULL)
			return (NULL);
		/* Skip empty lines and comments. */
		if (line[0] == '\n' || line[0] == '#')
			continue;
		len = strlen(line);
		if (line[len - 1] == '\n')
			line[len - 1] = '\0';
		last = line[skiplen];
		/* Skip the given mountpoint. */
		if (skip != NULL && strncmp(skip, line, skiplen) == 0 &&
		    (last == '\t' || last == ' ' || last == '\0')) {
			continue;
		}
		break;
	}
	return (line);
}

/*
 * Function translate options to a format acceptable by exports(5), eg.
 *
 *	-ro -network=192.168.0.0 -mask=255.255.255.0 -maproot=0 freefall.freebsd.org 69.147.83.54
 *
 * Accepted input formats:
 *
 *	ro,network=192.168.0.0,mask=255.255.255.0,maproot=0,freefall.freebsd.org
 *	ro network=192.168.0.0 mask=255.255.255.0 maproot=0 freefall.freebsd.org
 *	-ro,-network=192.168.0.0,-mask=255.255.255.0,-maproot=0,freefall.freebsd.org
 *	-ro -network=192.168.0.0 -mask=255.255.255.0 -maproot=0 freefall.freebsd.org
 *
 * Recognized keywords:
 *
 *	ro, maproot, mapall, mask, network, alldirs, public, webnfs, index, quiet
 *
 */
static const char *known_opts[] = { "ro", "maproot", "mapall", "mask",
    "network", "alldirs", "public", "webnfs", "index", "quiet", NULL };
static char *
translate_opts(const char *shareopts)
{
	static char newopts[OPTSSIZE];
	char oldopts[OPTSSIZE];
	char *o, *s = NULL;
	unsigned int i;
	size_t len;

	strlcpy(oldopts, shareopts, sizeof(oldopts));
	newopts[0] = '\0';
	s = oldopts;
	while ((o = strsep(&s, "-, ")) != NULL) {
		if (o[0] == '\0')
			continue;
		for (i = 0; known_opts[i] != NULL; i++) {
			len = strlen(known_opts[i]);
			if (strncmp(known_opts[i], o, len) == 0 &&
			    (o[len] == '\0' || o[len] == '=')) {
				strlcat(newopts, "-", sizeof(newopts));
				break;
			}
		}
		strlcat(newopts, o, sizeof(newopts));
		strlcat(newopts, " ", sizeof(newopts));
	}
	return (newopts);
}

static int
fsshare_main(const char *file, const char *mountpoint, const char *shareopts,
    int share)
{
	char tmpfile[PATH_MAX];
	char *line;
	FILE *newfd, *oldfd;
	int fd, error;

	newfd = oldfd = NULL;
	error = 0;

	/*
	 * Create temporary file in the same directory, so we can atomically
	 * rename it.
	 */
	if (strlcpy(tmpfile, file, sizeof(tmpfile)) >= sizeof(tmpfile))
		return (ENAMETOOLONG);
	if (strlcat(tmpfile, ".XXXXXXXX", sizeof(tmpfile)) >= sizeof(tmpfile))
		return (ENAMETOOLONG);
	fd = mkstemp(tmpfile);
	if (fd == -1)
		return (errno);
	/*
	 * File name is random, so we don't really need file lock now, but it
	 * will be needed after rename(2).
	 */
	error = flock(fd, LOCK_EX);
	assert(error == 0 || (error == -1 && errno == EOPNOTSUPP));
	newfd = fdopen(fd, "r+");
	assert(newfd != NULL);
	/* Open old exports file. */
	oldfd = fopen(file, "r");
	if (oldfd == NULL) {
		if (share) {
			if (errno != ENOENT) {
				error = errno;
				goto out;
			}
		} else {
			/* If there is no exports file, ignore the error. */
			if (errno == ENOENT)
				errno = 0;
			error = errno;
			goto out;
		}
	} else {
		error = flock(fileno(oldfd), LOCK_EX);
		assert(error == 0 || (error == -1 && errno == EOPNOTSUPP));
		error = 0;
	}

	/* Place big, fat warning at the begining of the file. */
	fprintf(newfd, "%s", FILE_HEADER);
	while (oldfd != NULL && (line = getline(oldfd, mountpoint)) != NULL)
		fprintf(newfd, "%s\n", line);
	if (oldfd != NULL && ferror(oldfd) != 0) {
		error = ferror(oldfd);
		goto out;
	}
	if (ferror(newfd) != 0) {
		error = ferror(newfd);
		goto out;
	}
	if (share) {
		fprintf(newfd, "%s\t%s\n", mountpoint,
		    translate_opts(shareopts));
	}

out:
	if (error != 0)
		unlink(tmpfile);
	else {
		if (rename(tmpfile, file) == -1) {
			error = errno;
			unlink(tmpfile);
		} else {
			/*
			 * Send SIGHUP to mountd, but unlock exports file later.
			 */
			restart_mountd();
		}
	}
	if (oldfd != NULL) {
		flock(fileno(oldfd), LOCK_UN);
		fclose(oldfd);
	}
	if (newfd != NULL) {
		flock(fileno(newfd), LOCK_UN);
		fclose(newfd);
	}
	return (error);
}

/*
 * Add the given mountpoint to the given exports file.
 */
int
fsshare(const char *file, const char *mountpoint, const char *shareopts)
{

	return (fsshare_main(file, mountpoint, shareopts, 1));
}

/*
 * Remove the given mountpoint from the given exports file.
 */
int
fsunshare(const char *file, const char *mountpoint)
{

	return (fsshare_main(file, mountpoint, NULL, 0));
}
