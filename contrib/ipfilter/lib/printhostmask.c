/*	$FreeBSD: src/contrib/ipfilter/lib/printhostmask.c,v 1.4.10.1.6.1 2010/12/21 17:09:25 kensmith Exp $	*/

/*
 * Copyright (C) 2000-2005 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: printhostmask.c,v 1.8.4.1 2006/06/16 17:21:12 darrenr Exp $
 */

#include "ipf.h"


void	printhostmask(v, addr, mask)
int	v;
u_32_t	*addr, *mask;
{
#ifdef  USE_INET6
	char ipbuf[64];
#else
	struct in_addr ipa;
#endif

	if (!*addr && !*mask)
		printf("any");
	else {
#ifdef  USE_INET6
		void *ptr = addr;
		int af;

		if (v == 4) {
			ptr = addr;
			af = AF_INET;
		} else if (v == 6) {
			ptr = addr;
			af = AF_INET6;
		} else
			af = 0;
		printf("%s", inet_ntop(af, ptr, ipbuf, sizeof(ipbuf)));
#else
		ipa.s_addr = *addr;
		printf("%s", inet_ntoa(ipa));
#endif
		printmask(mask);
	}
}
