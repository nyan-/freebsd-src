/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "namespace.h"
#include <rpc/key_prot.h>
#include "un-namespace.h"
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
/* Copyright (c)  1990, 1991 Sun Microsystems, Inc. */

/* #pragma ident	"@(#)key_prot.x	1.7	94/04/29 SMI" */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/rpc/key_prot_xdr.c,v 1.6.2.1.6.1 2010/12/21 17:09:25 kensmith Exp $");

/* 
 * Compiled from key_prot.x using rpcgen.
 * DO NOT EDIT THIS FILE!
 * This is NOT source code!
 */

bool_t
xdr_keystatus(register XDR *xdrs, keystatus *objp)
{

	if (!xdr_enum(xdrs, (enum_t *)objp))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_keybuf(register XDR *xdrs, keybuf objp)
{

	if (!xdr_opaque(xdrs, objp, HEXKEYBYTES))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_netnamestr(register XDR *xdrs, netnamestr *objp)
{

	if (!xdr_string(xdrs, objp, MAXNETNAMELEN))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_cryptkeyarg(register XDR *xdrs, cryptkeyarg *objp)
{

	if (!xdr_netnamestr(xdrs, &objp->remotename))
		return (FALSE);
	if (!xdr_des_block(xdrs, &objp->deskey))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_cryptkeyarg2(register XDR *xdrs, cryptkeyarg2 *objp)
{

	if (!xdr_netnamestr(xdrs, &objp->remotename))
		return (FALSE);
	if (!xdr_netobj(xdrs, &objp->remotekey))
		return (FALSE);
	if (!xdr_des_block(xdrs, &objp->deskey))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_cryptkeyres(register XDR *xdrs, cryptkeyres *objp)
{

	if (!xdr_keystatus(xdrs, &objp->status))
		return (FALSE);
	switch (objp->status) {
	case KEY_SUCCESS:
		if (!xdr_des_block(xdrs, &objp->cryptkeyres_u.deskey))
			return (FALSE);
		break;
	default:
		break;
	}
	return (TRUE);
}

bool_t
xdr_unixcred(register XDR *xdrs, unixcred *objp)
{
	u_int **pgids_val;

	if (!xdr_u_int(xdrs, &objp->uid))
		return (FALSE);
	if (!xdr_u_int(xdrs, &objp->gid))
		return (FALSE);
	pgids_val = &objp->gids.gids_val;
	if (!xdr_array(xdrs, (char **) pgids_val, (u_int *) &objp->gids.gids_len, MAXGIDS,
		sizeof (u_int), (xdrproc_t) xdr_u_int))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_getcredres(register XDR *xdrs, getcredres *objp)
{

	if (!xdr_keystatus(xdrs, &objp->status))
		return (FALSE);
	switch (objp->status) {
	case KEY_SUCCESS:
		if (!xdr_unixcred(xdrs, &objp->getcredres_u.cred))
			return (FALSE);
		break;
	default:
		break;
	}
	return (TRUE);
}

bool_t
xdr_key_netstarg(register XDR *xdrs, key_netstarg *objp)
{

	if (!xdr_keybuf(xdrs, objp->st_priv_key))
		return (FALSE);
	if (!xdr_keybuf(xdrs, objp->st_pub_key))
		return (FALSE);
	if (!xdr_netnamestr(xdrs, &objp->st_netname))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_key_netstres(register XDR *xdrs, key_netstres *objp)
{

	if (!xdr_keystatus(xdrs, &objp->status))
		return (FALSE);
	switch (objp->status) {
	case KEY_SUCCESS:
		if (!xdr_key_netstarg(xdrs, &objp->key_netstres_u.knet))
			return (FALSE);
		break;
	default:
		break;
	}
	return (TRUE);
}
