/*	$FreeBSD: src/contrib/ipfilter/lib/count4bits.c,v 1.4.10.1.6.1 2010/12/21 17:09:25 kensmith Exp $	*/

/*
 * Copyright (C) 2002 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: count4bits.c,v 1.1.4.1 2006/06/16 17:20:57 darrenr Exp $
 */

#include "ipf.h"


/*
 * count consecutive 1's in bit mask.  If the mask generated by counting
 * consecutive 1's is different to that passed, return -1, else return #
 * of bits.
 */
int	count4bits(ip)
u_int	ip;
{
	int cnt = 0, i, j;
	u_int ipn;

	ip = ipn = ntohl(ip);
	for (i = 32; i; i--, ipn *= 2)
		if (ipn & 0x80000000)
			cnt++;
		else
			break;
	ipn = 0;
	for (i = 32, j = cnt; i; i--, j--) {
		ipn *= 2;
		if (j > 0)
			ipn++;
	}
	if (ipn == ip)
		return cnt;
	return -1;
}
