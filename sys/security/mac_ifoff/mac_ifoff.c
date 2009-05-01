/*-
 * Copyright (c) 1999-2002, 2007 Robert N. M. Watson
 * Copyright (c) 2001-2002 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed by Robert Watson for the TrustedBSD Project.
 *
 * This software was developed for the FreeBSD Project in part by Network
 * Associates Laboratories, the Security Research Division of Network
 * Associates, Inc. under DARPA/SPAWAR contract N66001-01-C-8035 ("CBOSS"),
 * as part of the DARPA CHATS research program.
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
 * $FreeBSD: src/sys/security/mac_ifoff/mac_ifoff.c,v 1.13.2.1.4.1 2009/04/15 03:14:26 kensmith Exp $
 */

/*
 * Developed by the TrustedBSD Project.
 *
 * Limit access to interfaces until they are specifically administratively
 * enabled.  Prevents protocol stack-driven packet leakage in unsafe
 * environments.
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

#include <net/bpfdesc.h>
#include <net/if_types.h>

#include <security/mac/mac_policy.h>

SYSCTL_DECL(_security_mac);

SYSCTL_NODE(_security_mac, OID_AUTO, ifoff, CTLFLAG_RW, 0,
    "TrustedBSD mac_ifoff policy controls");

static int	ifoff_enabled = 1;
SYSCTL_INT(_security_mac_ifoff, OID_AUTO, enabled, CTLFLAG_RW,
    &ifoff_enabled, 0, "Enforce ifoff policy");
TUNABLE_INT("security.mac.ifoff.enabled", &ifoff_enabled);

static int	ifoff_lo_enabled = 1;
SYSCTL_INT(_security_mac_ifoff, OID_AUTO, lo_enabled, CTLFLAG_RW,
    &ifoff_lo_enabled, 0, "Enable loopback interfaces");
TUNABLE_INT("security.mac.ifoff.lo_enabled", &ifoff_lo_enabled);

static int	ifoff_other_enabled = 0;
SYSCTL_INT(_security_mac_ifoff, OID_AUTO, other_enabled, CTLFLAG_RW,
    &ifoff_other_enabled, 0, "Enable other interfaces");
TUNABLE_INT("security.mac.ifoff.other_enabled", &ifoff_other_enabled);

static int	ifoff_bpfrecv_enabled = 0;
SYSCTL_INT(_security_mac_ifoff, OID_AUTO, bpfrecv_enabled, CTLFLAG_RW,
    &ifoff_bpfrecv_enabled, 0, "Enable BPF reception even when interface "
    "is disabled");
TUNABLE_INT("security.mac.ifoff.bpfrecv.enabled", &ifoff_bpfrecv_enabled);

static int
check_ifnet_outgoing(struct ifnet *ifp)
{

	if (!ifoff_enabled)
		return (0);

	if (ifoff_lo_enabled && ifp->if_type == IFT_LOOP)
		return (0);

	if (ifoff_other_enabled && ifp->if_type != IFT_LOOP)
		return (0);

	return (EPERM);
}

static int
check_ifnet_incoming(struct ifnet *ifp, int viabpf)
{
	if (!ifoff_enabled)
		return (0);

	if (ifoff_lo_enabled && ifp->if_type == IFT_LOOP)
		return (0);

	if (ifoff_other_enabled && ifp->if_type != IFT_LOOP)
		return (0);

	if (viabpf && ifoff_bpfrecv_enabled)
		return (0);

	return (EPERM);
}

static int
ifoff_check_bpfdesc_receive(struct bpf_d *d, struct label *dlabel,
    struct ifnet *ifp, struct label *ifplabel)
{

	return (check_ifnet_incoming(ifp, 1));
}

static int
ifoff_check_ifnet_transmit(struct ifnet *ifp, struct label *ifplabel,
    struct mbuf *m, struct label *mlabel)
{

	return (check_ifnet_outgoing(ifp));
}

static int
ifoff_check_inpcb_deliver(struct inpcb *inp, struct label *inplabel,
    struct mbuf *m, struct label *mlabel)
{

	M_ASSERTPKTHDR(m);
	if (m->m_pkthdr.rcvif != NULL)
		return (check_ifnet_incoming(m->m_pkthdr.rcvif, 0));

	return (0);
}

static int
ifoff_check_socket_deliver(struct socket *so, struct label *solabel,
    struct mbuf *m, struct label *mlabel)
{

	M_ASSERTPKTHDR(m);
	if (m->m_pkthdr.rcvif != NULL)
		return (check_ifnet_incoming(m->m_pkthdr.rcvif, 0));

	return (0);
}

static struct mac_policy_ops ifoff_ops =
{
	.mpo_check_bpfdesc_receive = ifoff_check_bpfdesc_receive,
	.mpo_check_ifnet_transmit = ifoff_check_ifnet_transmit,
	.mpo_check_inpcb_deliver = ifoff_check_inpcb_deliver,
	.mpo_check_socket_deliver = ifoff_check_socket_deliver,
};

MAC_POLICY_SET(&ifoff_ops, mac_ifoff, "TrustedBSD MAC/ifoff",
    MPC_LOADTIME_FLAG_UNLOADOK, NULL);
