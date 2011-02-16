/*-
 * Test 0001:	Catch illegal instruction.
 *
 * $FreeBSD: src/tools/regression/bpf/bpf_filter/tests/test0001.h,v 1.4.2.1.6.1 2010/12/21 17:09:25 kensmith Exp $
 */

/* BPF program */
struct bpf_insn pc[] = {
	BPF_STMT(0x55, 0),
	BPF_STMT(BPF_RET+BPF_A, 0),
};

/* Packet */
u_char	pkt[] = {
	0x00,
};

/* Packet length seen on wire */
u_int	wirelen =	sizeof(pkt);

/* Packet length passed on buffer */
u_int	buflen =	sizeof(pkt);

/* Invalid instruction */
int	invalid =	1;

/* Expected return value */
u_int	expect =	0;

/* Expected signal */
int	expect_signal =	SIGABRT;
