/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */

#include <kernel/cpu_ppc.h>

#define xfer_setjmp(env) _setjmp(env)
#define xfer_longjmp(_env, _ret, _regs)		_longjmp(_env, _ret)

#define REGIP(reg)			((reg)->iar)
#define REGSP(reg)			((reg)->gpr[1])
#define REGTYPE(reg)		((reg)->gpr[0])
#define REGSTATUS(reg)		((reg)->gpr[0])

#define SETREGIP(reg,v)			(REGIP(reg) = (uint32_t)(v))
#define SETREGSP(reg,v)			(REGSP(reg) = (uint32_t)(v))
#define SETREGSTATUS(reg,v)		(REGSTATUS(reg) = (uint32_t)(v))
#define SETREGTYPE(reg,v)		(REGTYPE(reg) = (uint32_t)(v))

#define KER_ENTRY_SIZE		8	/* size of kernel entry opcode */
#define KERERR_SKIPAHEAD	8	/* increment IP by this much on error */

#if defined(VARIANT_smp)
extern volatile unsigned inkernel;
#define HAVE_INKERNEL_STORAGE	1
#define INKERNEL_BITS_SHIFT	8
#endif

//
//Override the definitions in ppc/neutrino.h with versions that can deal
//with the 400/booke series critical interrupt.
//
#if defined(VARIANT_400) || defined(VARIANT_booke) || defined(COMPILING_MODULE)
	extern  unsigned		ppc_ienable_bits;
	#define __inline_InterruptEnable()	(set_msr(get_msr() | ppc_ienable_bits))
	#define __inline_InterruptDisable()	(set_msr(get_msr() & ~ppc_ienable_bits))
#endif

#define rd_probe_1(ptr)	({ __attribute__((unused)) uint32_t dummy = *(const volatile uint32_t *)(ptr); })

static inline void
rd_probe_num(const void *loc, int num) {
	ulong_t tmp;

	asm volatile(
		"subi	%1,%2,4;"
		"mtctr	%3;"
		"1:;"
		"	lwzu %0,4(%1);" 
		"bdnz	1b;"
		: "=&r" (tmp), "=&b"(loc)
		: "1" (loc), "r" (num)
		: "ctr","1"
	);
}

static inline void
wr_probe_1(void *loc) {
	ulong_t tmp;

	asm volatile(
		"	lwz %0,0(%1);"
		"	stw %0,0(%1);"
		: "=&r" (tmp)
		: "b" (loc)
	);
}

static inline void
wr_probe_num(void *loc, int num) {
	ulong_t tmp;

	asm volatile(
		"mtctr	%3;"
		"1:;"
		"	lwz %0,0(%1);"
		"	stw %0,0(%1);"
		"	addi %1,%1,4;"
		"bdnz	1b;"
		: "=&r" (tmp), "=&b" (loc)
		: "1" (loc), "r" (num)
		: "ctr","%r0"
	);
}


#ifdef VARIANT_smp

#if INKERNEL_BITS_SHIFT!=8
#error Need to modify bitset_inkernel macro
#endif
#define bitset_inkernel(bits)		(*(((char *)&inkernel)+2) |= (bits>>INKERNEL_BITS_SHIFT))
#define bitclr_inkernel(bits)		(*(((char *)&inkernel)+2) &= ~(bits>>INKERNEL_BITS_SHIFT))

#define am_inkernel() ppc_am_inkernel()

static inline unsigned
ppc_am_inkernel(void) {
	unsigned tmp;

	asm volatile (
		"mfmsr %0;"
		"rlwinm %0,%0,30,31,31;"
		: "=&r" (tmp)
		:
	);
	return tmp;
}

extern void		smp_init_cpu(void);

#endif

struct exc_copy_block {
	uint32_t		size;
	uint32_t		code[0x10000]; // variable sized, big num to avoid SDA
};

struct trap_entry {
	unsigned					index;
	void						(*func)();
	const struct exc_copy_block	*exc_code;
};

// NOTE: if you update this enum, you must also update the tlbrtn_array[]
// array in ppc/booke/init_vm.c.
enum {
	PPCBKE_TLB_SELECT_E500,
	PPCBKE_TLB_SELECT_E500v2,
	PPCBKE_TLB_SELECT_IBM
};

// On Book E CPU's, we keep the L1 page table pointer in SPRG4
#define PPCBKE_SPR_L1PAGETABLE_WR	PPCBKE_SPR_SPRG4
#define PPCBKE_SPR_L1PAGETABLE_RD	PPCBKE_SPR_SPRG4RO


union ppc_alt_regs {
	PPC_VMX_REGISTERS	vmx;
	PPC_SPE_REGISTERS	spe;
};

struct thread_entry;
extern void alt_context_alloc();
extern void alt_context_init(struct thread_entry *);
extern void alt_context_free(struct thread_entry *thp);
extern void alt_force_save(struct thread_entry *thp);

extern uint32_t		*trap_install(unsigned, void (*)(), const struct exc_copy_block *copy);
extern uintptr_t	trap_chain_addr(unsigned);
extern void 		trap_install_set(const struct trap_entry *, unsigned);
extern void 		copy_code(uint32_t *dst, const struct exc_copy_block *src);
extern unsigned		determine_family(unsigned pvr);
extern void			config_cpu(unsigned pvr, const struct exc_copy_block **entry, const struct exc_copy_block **exit);

struct intrinfo_entry;

extern void			exc_vector_init(void);
extern void			exc_intr_install(struct intrinfo_entry *, void *, void **);
extern uint32_t		*exc_vector_address(unsigned, unsigned);


extern void __ker_entry();
extern void __exc_unexpected();

extern const struct exc_copy_block	__ker_entry_exc;
extern const struct exc_copy_block	__common_exc_entry;
extern const struct exc_copy_block	intr_entry;
extern const struct exc_copy_block	intr_entry_critical;

/*
 * Reserved areas in exc. table 
 *
 *  0x1e00:	used by 6xx tlb miss handlers
 *  0x2100:	low code area
 *  0x2100: TLB reload random sequence (book E)
 *	0x2c00:	kerentry sequence
 *	0x2e00:	kerexit sequence
 */  
#define PPC_LOW_CODE_START	0x2100
#define PPCBKE_RANDOM_BASE	0x2100
#define PPCBKE_RANDOM_SHIFT	6
#define PPCBKE_RANDOM_SIZE	(1 << PPCBKE_RANDOM_SHIFT)
#define PPC_KERENTRY_COMMON 0x2c00
#define PPC_KEREXIT_COMMON	0x2e00

/* __SRCVERSION("kercpu.h $Rev: 170836 $"); */
