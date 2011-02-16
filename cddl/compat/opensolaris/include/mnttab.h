/* $FreeBSD: src/cddl/compat/opensolaris/include/mnttab.h,v 1.4.2.2.2.1 2010/12/21 17:09:25 kensmith Exp $ */

#ifndef	_OPENSOLARIS_MNTTAB_H_
#define	_OPENSOLARIS_MNTTAB_H_

#include <sys/param.h>
#include <sys/mount.h>

#include <stdio.h>
#include <paths.h>

#define	MNTTAB		_PATH_DEVZERO
#define	MNT_LINE_MAX	1024

#define	umount2(p, f)	unmount(p, f)

struct mnttab {
	char	*mnt_special;
	char	*mnt_mountp;
	char	*mnt_fstype;
	char	*mnt_mntopts;
};
#define	extmnttab	mnttab

int getmntany(FILE *fd, struct mnttab *mgetp, struct mnttab *mrefp);
int getmntent(FILE *fp, struct mnttab *mp);
char *hasmntopt(struct mnttab *mnt, char *opt);

void statfs2mnttab(struct statfs *sfs, struct mnttab *mp);

#endif	/* !_OPENSOLARIS_MNTTAB_H_ */
