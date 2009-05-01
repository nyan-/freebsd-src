/* $FreeBSD: src/usr.bin/kdump/kdump_subr.h,v 1.3.8.1 2009/04/15 03:14:26 kensmith Exp $ */

void signame (int);
void semctlname (int);
void shmctlname (int);
void semgetname (int);
void fcntlcmdname (int, int, int);
void rtprioname (int);
void modename (int);
void flagsname (int);
void flagsandmodename (int, int, int);
void accessmodename (int);
void mmapprotname (int);
void mmapflagsname (int);
void wait4optname (int);
void sendrecvflagsname (int);
void getfsstatflagsname (int);
void mountflagsname (int);
void rebootoptname (int);
void flockname (int);
void sockoptname (int);
void sockoptlevelname (int, int);
void sockdomainname (int);
void sockipprotoname (int);
void socktypename (int);
void thrcreateflagsname (int);
void mlockallname (int);
void shmatname (int);
void rforkname (int);
void nfssvcname (int);
void whencename (int);
void rlimitname (int);
void shutdownhowname (int);
void prioname (int);
void madvisebehavname (int);
void msyncflagsname (int);
void schedpolicyname (int);
void kldunloadfflagsname (int);
void ksethrcmdname (int);
void extattrctlname (int);
void kldsymcmdname (int);
void sendfileflagsname (int);
void acltypename (int);
void sigprocmaskhowname (int);
void lio_listioname (int);
void minheritname (int);
void quotactlname (int);
void ptraceopname (int);
