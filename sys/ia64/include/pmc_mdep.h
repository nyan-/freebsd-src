/*-
 * This file is in the public domain.
 *
 * $FreeBSD: src/sys/ia64/include/pmc_mdep.h,v 1.2.20.1 2009/04/15 03:14:26 kensmith Exp $
 */

#ifndef _MACHINE_PMC_MDEP_H_
#define	_MACHINE_PMC_MDEP_H_

union pmc_md_op_pmcallocate {
	uint64_t		__pad[4];
};

/* Logging */
#define	PMCLOG_READADDR		PMCLOG_READ64
#define	PMCLOG_EMITADDR		PMCLOG_EMIT64

#if	_KERNEL
union pmc_md_pmc {
};

#endif

#endif /* !_MACHINE_PMC_MDEP_H_ */
