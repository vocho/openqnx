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



/*
 *	kernel/cpu_ppc.h


 */
#include <ppc/cpu.h>
#include <ppc/inline.h>
#include <ppc/priv.h>
#include <ppc/smpxchg.h>
#include <ppc/bookecpu.h>

#define FAULT_ISWRITE(_n)	((_n) & SIGCODE_STORE)

typedef PPC_CPU_REGISTERS	CPU_REGISTERS;
typedef PPC_FPU_REGISTERS	FPU_REGISTERS;
typedef PPC_ALT_REGISTERS	ALT_REGISTERS;
typedef PPC_PERFREGS		PERF_REGISTERS;
#define CPU_STRINGNAME		PPC_STRINGNAME

#define STARTUP_STACK_NBYTES		4096
#define KER_STACK_NBYTES			8192
#define IDLE_STACK_NBYTES			512
#define DEF_MAX_THREAD_STACKSIZE	32768

#define DEF_VIRTUAL_THREAD_STACKSIZE		131072
#define DEF_PHYSICAL_THREAD_STACKSIZE		4096
#define DEF_VIRTUAL_FIRST_THREAD_STACKSIZE	524288
#define DEF_PHYSICAL_FIRST_THREAD_STACKSIZE	4096

#define STACK_ALIGNMENT						16
#define STACK_INITIAL_CALL_CONVENTION_USAGE	16

#define CPU_P2V(p)				((uintptr_t)(p))
#define CPU_P2V_NOCACHE(p)		CPU_P2V(p)
#define CPU_V2P(v)				((paddr_t)(uintptr_t)(v))
#define CPU_VADDR_IN_RANGE(v)	1

#define VM_USER_SPACE_BOUNDRY		0x40000000
#define VM_USER_SPACE_SIZE 			(0xbfff0000-0x00040000)
#define VM_KERN_SPACE_BOUNDRY		0x00000000
#define VM_KERN_SPACE_SIZE			0x40000000
#define VM_CPUPAGE_ADDR				0xfffc0000
#define VM_SYSPAGE_ADDR				0xffff2000

//RUSH3: I suspect that we actually don't need the #if anymore - all
//RUSH3: families can use the 600/900 values. Something to investigate later
#if defined(VARIANT_600) || defined(VARIANT_900)
	#define VM_MSG_XFER_START		(0x10000000)
	#define VM_MSG_XFER_END			(VM_MSG_XFER_START+(512UL*1024UL*1024UL))
#else
	#define VM_MSG_XFER_START		(VM_MSG_XFER_END-(128UL*1024UL*1024UL))
	#define VM_MSG_XFER_END			VM_KERN_SPACE_SIZE
#endif

#define VM_MSG_XFER_SIZE			(VM_MSG_XFER_END-VM_MSG_XFER_START)

#if defined(VARIANT_booke) || defined(VARIANT_600) || defined(VARIANT_900)
	#define VM_KERN_LOW_SIZE		(256*1024*1024)
#else
	#define VM_KERN_LOW_SIZE		VM_MSG_XFER_START
#endif

#define VM_FAULT_INKERNEL	0x80000000
#define VM_FAULT_WRITE		0x40000000
#define VM_FAULT_INSTR		0x20000000
#define VM_FAULT_WIMG_ERR	0x10000000
#define VM_FAULT_GLOBAL		0x08000000
#define VM_FAULT_KEREXIT	0x04000000

/* registers that need to be saved for interrupt routine invocation */
struct cpu_intrsave {
	ppcint		gpr2;
	ppcint		gpr13;
};

/* Breakpoint fields that this cpu may need for breakpoints */
#define CPU_DEBUG_BRKPT			\
	paddr_t		paddr;			\
	union {						\
		uint32_t	ins;		\
		uint8_t		value[4];	\
	}			data; \
	unsigned	hwreg;

/* Fields this cpu may need for debugging a process */
#define CPU_DEBUG		\
	BREAKPT		step; \
	unsigned	dbcr0; \
	unsigned	real_step_flags; \
	unsigned	max_hw_watchpoints; \
	unsigned	num_watchpoints;

/* CPU specific fields in the thread entry structure */
struct cpu_thread_entry {
	PPC_PERFREGS		*pcr;
	uintptr_t			low_mem_boundry;
	union ppc_alt_regs	*alt;
	unsigned	dbsr; /* note - only valid after debug exception */
};

/* cpu specific information for memmgr fault handling */
struct cpu_fault_info {
	unsigned	code;
	unsigned	asid;
};

/* Extra state (read only) made available by the kernel debugger */
struct cpu_extra_state_400 {
	struct {
		unsigned		num_entries;
		struct {
			uint32_t	lo;
			uint32_t	hi;
			uint32_t	tid;
		}			*entry;
	} tlb;
	struct {
		uint32_t	dear;
		uint32_t	esr;
		uint32_t	pid;
		uint32_t	zpr;
	} spr;
};
	
struct cpu_extra_state_booke {
	struct {
		uint32_t		dbcr0;
		uint32_t		dbcr1;
		uint32_t		dbcr2;
		uint32_t		dbsr;
		uint32_t		dear;
		uint32_t		esr;
		uint32_t		ivpr;
		uint32_t		ivor[16];
		uint32_t		pid;
		uint32_t		pir;
		uint32_t		sprg4;
		uint32_t		sprg5;
		uint32_t		sprg6;
		uint32_t		sprg7;
		uint32_t		tcr;
		uint32_t		tsr;
	} spr;
	struct {
		unsigned		num_entries;
		ppcbke_tlb_t	*entry;
	} 					tlb[8];
};

struct cpu_extra_state_440 {
	struct cpu_extra_state_booke	ppcbke;
	struct {
		uint32_t			ccr0;
		uint32_t			mmucr;
		uint32_t			rstcfg;
	} spr;
};

struct cpu_extra_state_booke_m {
	struct cpu_extra_state_booke	ppcbke;
	struct {
		uint32_t			mas[7];
		uint32_t			pid1;
		uint32_t			pid2;
		uint32_t			pid3;
	} spr;
};

struct cpu_extra_state_e500 {
	struct cpu_extra_state_booke_m	ppcbkem;
	struct {
		uint32_t			ivor32;
		uint32_t			ivor33;
		uint32_t			ivor34;
		uint32_t			ivor35;
		uint32_t			hid0;
		uint32_t			hid1;
		uint32_t			bucsr;
	} spr;
};

struct cpu_extra_state_600 {
	struct ppc600_bat {
		uint32_t		lo;
		uint32_t		up;
	}						dbat[8];
	struct ppc600_bat		ibat[8];
	uint32_t				hid0;
	uint32_t				hid1;
	uint32_t                srr0;
	uint32_t                srr1;
	uint32_t                sdr1;
	uint32_t                dar;
	uint32_t                dsisr;
	uint32_t                sr[16];
	uint32_t                sprg0;
	uint32_t                sprg1;
	uint32_t                sprg2;
	uint32_t                sprg3;
	uint32_t                pir;
	uint32_t                pvr;
	uint32_t                ldstcr;
	uint32_t                ictrl;
	uint32_t                l2cr;
	uint32_t                l3cr;
	uint32_t                msscr0;
	uint32_t                msssr0;
};

struct cpu_extra_state_900 {
	uint64_t				hid[6];
	uint64_t				dsisr;
	uint64_t				dar;
	uint64_t				sdr1;
	uint64_t				accr;
	uint64_t				ctrl;
	uint64_t				asr;
	uint64_t				dabr;
	uint64_t				pir;
	struct {
		uint32_t			v;
		uint32_t			e;
	}						slb[16];
};

struct cpu_extra_state_970 {
	struct cpu_extra_state_900	ppc900;
	uint64_t					dabrx;
	uint64_t					scomc;
	uint64_t					scomd;
};

struct cpu_extra_state {
	union {
		struct cpu_extra_state_400		ppc400;
		struct cpu_extra_state_booke	ppcbke;
		struct cpu_extra_state_booke_m	ppcbkem;
		struct cpu_extra_state_440		ppc440;
		struct cpu_extra_state_e500		ppce500;
		struct cpu_extra_state_600		ppc600;
		struct cpu_extra_state_900		ppc900;
		struct cpu_extra_state_970		ppc970;
	} u;
};


/*
 * Layout of a Page Table Entry on a Book E CPU.
 */
struct ppcbke_pte {
	uint32_t			paddr;
	uint32_t			flags;
};

#define PPCBKE_PTE_ATTR_E		0x00000001	
#define PPCBKE_PTE_ATTR_G		0x00000002
#define PPCBKE_PTE_ATTR_M		0x00000004
#define PPCBKE_PTE_ATTR_I		0x00000008
#define PPCBKE_PTE_ATTR_W		0x00000010
#define PPCBKE_PTE_ACCESS_R		0x00000020
#define PPCBKE_PTE_ACCESS_W		0x00000040
#define PPCBKE_PTE_ACCESS_X		0x00000080
#define PPCBKE_PTE_PGSZ_MASK	0x0000ff00
#define PPCBKE_PTE_PGSZ_SHIFT	         8
	
extern struct tlbops {
	void	(*flush_all)(void);
	void	(*flush_vaddr)(uintptr_t vaddr, unsigned asid);
	void	(*flush_asid)(unsigned asid);
	void	(*write_entry)(int tlb, int idx, const ppcbke_tlb_t *entry);
	void	(*read_entry)(int tlb, int idx, ppcbke_tlb_t *entry);
} tlbop;


extern unsigned		tlbop_get_wired(void);
extern void			tlbop_set_wired(unsigned);
extern void			set_l1pagetable(void *, unsigned
#if defined(VARIANT_600)
		, unsigned
#endif		
		);
extern void			*get_l1pagetable(void);
extern void			copy_vm_code(void);
extern unsigned		get_spr_indirect(unsigned spr);
extern void			set_spr_indirect(unsigned spr, unsigned val);

/* __SRCVERSION("cpu_ppc.h $Rev: 204741 $"); */
