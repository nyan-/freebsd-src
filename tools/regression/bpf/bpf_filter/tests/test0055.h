/*-
 * Test 0055:	Check boundary conditions (BPF_LD+BPF_H+BPF_IND)
 *
 * $FreeBSD: src/tools/regression/bpf/bpf_filter/tests/test0055.h,v 1.2.2.1.6.1 2010/12/21 17:09:25 kensmith Exp $
 */

/* BPF program */
struct bpf_insn pc[] = {
	BPF_STMT(BPF_LD+BPF_IMM, 0xdeadc0de),
	BPF_STMT(BPF_LDX+BPF_IMM, 4),
	BPF_STMT(BPF_LD+BPF_H+BPF_IND, 1),
	BPF_STMT(BPF_RET+BPF_A, 0),
};

/* Packet */
u_char	pkt[] = {
	0x01, 0x23, 0x45, 0x67,
};

/* Packet length seen on wire */
u_int	wirelen =	sizeof(pkt);

/* Packet length passed on buffer */
u_int	buflen =	sizeof(pkt);

/* Invalid instruction */
int	invalid =	0;

/* Expected return value */
u_int	expect =	0;

/* Expected signal */
int	expect_signal =	0;
