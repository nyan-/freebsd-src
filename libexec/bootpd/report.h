/* report.h */
/* $FreeBSD: src/libexec/bootpd/report.h,v 1.5.36.1.6.1 2010/12/21 17:09:25 kensmith Exp $ */

extern void report_init(int nolog);
extern void report(int, const char *, ...) __printflike(2, 3);
extern const char *get_errmsg(void);
