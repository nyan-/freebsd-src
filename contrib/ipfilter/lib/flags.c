/*	$FreeBSD: src/contrib/ipfilter/lib/flags.c,v 1.4.10.1.6.1 2010/12/21 17:09:25 kensmith Exp $	*/

/*
 * Copyright (C) 2001-2002 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: flags.c,v 1.4.4.1 2006/06/16 17:20:58 darrenr Exp $
 */

#include "ipf.h"

/*
 * ECN is a new addition to TCP - RFC 2481
 */
#ifndef TH_ECN
# define	TH_ECN  0x40
#endif
#ifndef TH_CWR
# define	TH_CWR  0x80
#endif

char	flagset[] = "FSRPAUEC";
u_char	flags[] = { TH_FIN, TH_SYN, TH_RST, TH_PUSH, TH_ACK, TH_URG,
		    TH_ECN, TH_CWR };
