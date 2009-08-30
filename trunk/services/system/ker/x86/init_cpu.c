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

#include "externs.h"

extern unsigned init_send_ipi();

extern void __ker_entry();
extern void __ker_sysenter();

extern void __exc0(), __exc1(), __exc3(), __exc4(), __exc5(), __exc6();
extern void __exc7(), __exc8(), __exc9(), __exca();
extern void __excb(), __excc(), __excd(), __exce();
extern void __excf(), __exc10(), __exc11(), __exc12();
extern void __exc7emul();
extern void __intr_unexpected();

extern void __fault_error(), __fault_noerror();

void *__fpuemu_stub = &__exc7emul;

const struct trap_entry {
	unsigned 		trapnum;
	void			(*func)();
} trap_list[] ={

	// Ignore trppoints and msgpoints if no kernel debugger present
	{0x620,	__fault_noerror},
	{0x621,	__fault_noerror},

	// Kernel entry point
	{0x428,	&__ker_entry},

	// Faults
	{0x000,	&__exc0},
	{0x001,	&__exc1},
	{0x403,	&__exc3},
	{0x004,	&__exc4},
	{0x005,	&__exc5},
	{0x006,	&__exc6},
	{0x007,	&__exc7},
	{0x008,	&__exc8},
	{0x009,	&__exc9},
	{0x00a,	&__exca},
	{0x00b,	&__excb},
	{0x00c,	&__excc},
	{0x00d,	&__excd},
	{0x00e,	&__exce},
	{0x00f,	&__excf},
	{0x010,	&__exc10},
	{0x011,	&__exc11},
	{0x012,	&__exc12},

	{0,		NULL}
};

//
// If bit 0x100 is set in trapnum we make it a conforming code segment.
// If bit 0x200 is set, don't install if vector is already non-NULL.
// If bit 0x400 is set, allow software interrupt in user level program.
//
void
set_trap(unsigned trapnum, void (*func)()) {
	struct x86_gate_descriptor_entry *gdp;
	uintptr_t	addr, old;

	gdp = &_syspage_ptr->un.x86.idt[trapnum & 0xff];

	old = gdp->selector == ker_cs ? gdp->offset_lo | (gdp->offset_hi << 16) : 0;
	if((old == NULL || ((trapnum & 0x200)==0)) && (addr = (uintptr_t)func)) {
		gdp->offset_lo = addr & 0xffff;
		gdp->offset_hi = (addr >> 16) & 0xffff;
		gdp->selector = ker_cs;
		gdp->flags = X86_PRESENT | (X86_INT_GATE32 >> 1) | X86_DPL0;
		if(trapnum & 0x400) {
			gdp->flags |= X86_DPL3;
		}
		if(trapnum & 0x100) {
			gdp->selector = ker2_cs;
			gdp->flags = X86_PRESENT | (X86_TRAP_GATE32 >> 1) | X86_DPL0;
		}
		gdp->intel_reserved = 0;
	}
}


//RUSH3: Note that the code in kernel.S currently doesn't set SEGV_ACCERR
//RUSH3: when it should - it always generates a SEG_MAPERR without consulting
//RUSH3: the fault flags. Make sure that gets fixed when removing the shim.

//RUSH3: Shim code to be removed later
unsigned 
vmm_fault_shim(PROCESS *prp, void *vaddr, unsigned flags) {
	struct fault_info	info;
	unsigned			sc;

	info.cpu.code = flags;
	info.prp = prp;
	info.vaddr = (uintptr_t)vaddr;
	if(flags & X86_FAULT_PAGELP) {
		sc = MAKE_SIGCODE(SIGSEGV, SEGV_ACCERR, FLTPAGE);
	} else {
		sc = MAKE_SIGCODE(SIGSEGV, SEGV_MAPERR, FLTPAGE);
	}
	if(flags & X86_FAULT_WRITE) {
		sc |= SIGCODE_STORE;
	}
	//temp hack from kernel.S to let us know if we were in system, kerexit.
	if(flags & 0x80000000) {
		sc |= SIGCODE_KERNEL;
		if(flags & 0x40000000) {
			sc |= SIGCODE_KEREXIT;
		}
		if(GET_XFER_HANDLER() != NULL) {
			sc |= SIGCODE_INXFER;
		}
	}
	info.sigcode = sc;
	switch(memmgr.fault(&info)) {
	case -1:	// illegal access, drop a signal on the process
		// temp hack to turn off the SIGCODE_KERNEL/KEREXIT/INXFER bits
		// since those will confuse the kernel debugger.
		return info.sigcode & ~(SIGCODE_KERNEL|SIGCODE_KEREXIT|SIGCODE_INXFER);
	case 0:		// defer to process time
		//At this point in time, we need to make sure to
		//acquire the kernel, if we haven't already.
		if(PageWait(info.vaddr, info.sigcode, info.prp->pid, memmgr.fault_pulse_code) != EOK) {
			/*
			 * We're in serious trouble here. We either got called from an
			 * interrupt handler or we've run out of memory. Return a
			 * somewhat strange code so that people know what's going on.
			 */
			return MAKE_SIGCODE(SIGILL, ILL_BADSTK, FLTSTACK);
		}
		break;
	case 1:		// access OK and corrected, retry instruction
		break;
	default: break;	
	}
	return 0;
}

void
init_cpu() {
	static int						cr0 = 0;
	static paddr32_t				cr3;
	int 							cpu;
	struct x86_seg_descriptor_entry	*sdp;
	X86_TSS							*tssp;
	uintptr_t						 addr;

	//  EM MP NE ET
	//   1  0  1  0   No FPU
	//   0  1  1  1   FPU == 387
	//   0  1  1  x   FPU > 387

	// The BIOS will have correctly set up the various cache options for the
	// boot processor only. Therefore we use it as a model for all other
	// processors.
	if(cr0 == 0) {
		cr0 = rdcr0() & ~(X86_MSW_EM_BIT | X86_MSW_MP_BIT);
		cr0 |= X86_MSW_TS_BIT | X86_MSW_NE_BIT | X86_MSW_AM_BIT;

		if((__cpu_flags & CPU_FLAG_FPU) && !fpuemul) {
			cr0 |= X86_MSW_MP_BIT;
		} else {
			cr0 |= X86_MSW_EM_BIT;
			set_trap(0x100 | 0x07, __fpuemu_stub);
		}

		if(SYSPAGE_ENTRY(cpuinfo)->cpu == 386) {
			cr0 |= X86_MSW_ET_BIT;
		}
		cr3 = rdpgdir();

	}
	ldcr0(cr0);
	ldpgdir(cr3);

#if defined(VARIANT_smp)
	cpu = init_send_ipi();
#else
	cpu = 0;
#endif

	// Allocate enough space in the TSS to allow an io permission
	// bitmap for the first 256 ports. We'll allow threads being run
	// in V86 mode to access those ports without causing exceptions
	// (see the SETTSS macro in x86/kernel.S). Normal user threads
	// will be disallowed.
	#define V86_PORT_IOMAP_SIZE	(256/(sizeof(uint8_t)*8))

	// We can't let the first portion of a TSS cross a page boundry
	// since the chip assumes that the memory is physically contiguous.
	// What we do is allocate extra space and then figure out a pointer
	// value that won't cause the problem and free back the rest.
	#define TSS_ALLOC_SIZE (sizeof(*tssp)*2 + V86_PORT_IOMAP_SIZE)
	tssp = _scalloc(TSS_ALLOC_SIZE);
	if((((uintptr_t)tssp) & (__PAGESIZE-1)) > (__PAGESIZE - sizeof(*tssp))) {
		_sfree(tssp, sizeof(*tssp));
		tssp += 1;
	} else {
		_sfree((uint8_t *)tssp + (TSS_ALLOC_SIZE - sizeof(*tssp)), sizeof(*tssp));
	}
	tss[cpu] = tssp;
	tssp->ss0 = ker_ss;
	tssp->esp0 = 0; // Temp value till first thread runs
	tssp->pdbr = (unsigned)rdpgdir();
	tssp->iomap_base = offsetof(X86_TSS, iomap_data);
	memset(tssp->iomap_data, ~0, V86_PORT_IOMAP_SIZE+1);

	addr = (unsigned) tssp;
	sdp = &_syspage_ptr->un.x86.gdt[32+cpu];
	sdp->flags = X86_TYPE_TSS32_SEG | X86_DPL1;
	sdp->base_lo = addr & 0xffff;
	sdp->base_hi = (addr >> 16) & 0xff;
	sdp->base_xhi = (addr >> 24) & 0xff;
	sdp->limit = tssp->iomap_base + (1 + V86_PORT_IOMAP_SIZE);
	sdp->limflags = 0;

	ldds(usr_ds);
	ldes(usr_ds);
	ldfs(cpupage_segs[cpu] = 0x20 + cpu*sizeof(struct x86_seg_descriptor_entry));
	ldgs(0);

	wrtr((32+cpu)*8);

	// Init SYSENTER MSR's on CPU's that support it
	if(__cpu_flags & X86_CPU_SEP) {
		// SYSENTER selector is 208...
		wrmsr(0x174,0xe0);
		wrmsr(0x175,ker_stack[cpu]);
		wrmsr(0x176,(unsigned) &__ker_sysenter);
	}
}


void
v86_mark_running(unsigned on) {
	X86_TSS		**tsspp;

	// If we have a V86 thread in the system, set the bottom bit
	// of the tss array so that the SETTSS macro in kernel.S knows that
	// it needs to update the iomap_data fields.
	tsspp = &tss[RUNCPU];
	*tsspp = (void *)(((uintptr_t)*tsspp & ~1) | on);
}


void
init_traps(void) {
	const struct trap_entry *tep;
	unsigned				i;
	unsigned				nintrs;
	struct segment_info		idt;

#if defined(VARIANT_smp)
	if(SYSPAGE_ENTRY(cpuinfo)->cpu < 486) {
		kprintf("SMP kernel requires a 486 or better CPU\n");
		crash();
	}
#endif

	// Setup trap vectors
	for(tep = &trap_list[0]; tep->func != NULL; ++tep) {
		set_trap(tep->trapnum, tep->func);
	}
	// Init SYSENTER on CPU's that support it
	if(__cpu_flags & X86_CPU_SEP) {
		// Make the default user mode selectors be those set by sysexit
		usr_cs = 0xf3;
		usr_ss = 0xfb;
	}
	if(__cpu_flags & X86_CPU_FXSR) {
		// Adjust size and alignment of fpu save area
		fpu_souls.size = 512;
		if(fpu_souls.align < 16) fpu_souls.align = 16;
	}

	sidt(&idt);
	nintrs = (idt.limit+1) / sizeof(struct x86_gate_descriptor_entry);
	//
	// Make sure all the entries in the IDT are pointing somewhere
	//
	for(i = 0; i < nintrs; ++i) {
		set_trap(0x200|i, __intr_unexpected);
	}
}

struct level_label {
	struct level_label	*combine;
	struct level_label	*cascade;
	int					primary_level;
	unsigned			off;
	unsigned			id_off;
	unsigned			done_off;
	int					secondary;
};

static uint8_t		*intrentry_base;
static uintptr_t	intrentry_off;
static unsigned		intrentry_size;

static void
calculate_combines(struct level_label *labels) {
	unsigned			i;
	int					level;
	int					primary;
	struct level_label	*next;

	/*
	 * Figure out the combined interrupt links
	 * (things that share the same CPU interrupt or cascade vector).
	 */
	for(i = 0; i < intrinfo_num; ++i) {
		struct intrinfo_entry	*iip = &intrinfoptr[i];

		next = NULL;
		level = get_interrupt_level(NULL, iip->vector_base);
		primary = get_interrupt_level(NULL, iip->cascade_vector);
		labels[level].primary_level = primary;
		if(primary != -1) labels[primary].cascade = &labels[level];
		if(!labels[level].secondary) {
			unsigned	j;

			for(j = intrinfo_num - 1; j > i; --j) {
				struct intrinfo_entry	*iip2 = &intrinfoptr[j];
				int		combine = 0;

				if(iip2->cascade_vector == iip->cascade_vector) {
					if(iip->cascade_vector != _NTO_INTR_SPARE) {
						combine = 1;
					} else if(iip2->cpu_intr_base == iip->cpu_intr_base) {
						combine = 1;
					}
				}

				if(combine) {
					unsigned	slevel;

					slevel = get_interrupt_level(NULL, iip2->vector_base);

					labels[slevel].secondary = 1;
					labels[slevel].combine = next;
					next = &labels[slevel];
				}
			} 
			labels[level].combine = next;
		}
	}
}


static void
add_code(const void *start, unsigned size) {
	unsigned	new;

	new = intrentry_off + size;
	if(intrentry_base != NULL) {
		if(new <= intrentry_size) {
			memcpy(&intrentry_base[intrentry_off], start, size);
		} else {
			_sfree(intrentry_base, intrentry_size);
			intrentry_base = NULL;
		}
	}
	intrentry_off = new;
}

#define REGNO_NONE	(-1)
#define REGNO_EAX	0
#define REGNO_ECX	1
#define REGNO_EDX	2
#define REGNO_EBX	3
#define REGNO_ESP	4
#define REGNO_EBP	5
#define REGNO_ESI	6
#define REGNO_EDI	7
	
static void
gen_load(unsigned reg, uintptr_t value) {
	uint8_t	buff[5];

	buff[0] = 0xb8+reg;
	*(uintptr_t *)&buff[1] = value;
	add_code(buff, sizeof(buff));
}

static void
gen_modrm(unsigned opcode, int dest_reg, int base_reg, int index_reg, int scale, uintptr_t off) {
	uint8_t		buff[10];
	uint8_t		*p;
	unsigned	mod;

	p = buff;
	if(opcode > 0xff) {
		*p++ = opcode >> 8;
	}
	*p++ = opcode;
	if(off == 0) {
		mod = 0x00;
	} else if( (int)off >= -127 && (int)off <= 127) {
		mod = 0x40;
	} else {
		mod = 0x80;
	}
	if(index_reg != REGNO_NONE) {
		if(base_reg == REGNO_NONE) {
			*p++ = 0 + (dest_reg << 3) + 0x4;
			mod = 0x80; //trigger 4 byte offset later
			base_reg = REGNO_EBP;
		} else {
			*p++ = mod + (dest_reg << 3) + 0x04;
		}
		*p++ = (scale << 6) + (index_reg << 3) + base_reg; //SIB byte
	} else {
		*p++ = mod + (dest_reg << 3) + base_reg;
	}
	switch(mod) {
	case 0x00:
		break;
	case 0x40:
		*p++ = off;
		break;
	case 0x80:
		*(uintptr_t *)p = off;
		p += sizeof(uintptr_t);
		break;
	default: break;
	}
	add_code(buff, p - buff);
}

static void
gen_burst_loads(unsigned genflags, unsigned level, int cascaded_eoi) {
	if(genflags & INTR_GENFLAG_LOAD_LEVEL) {
		gen_load(REGNO_EAX, level);
	}
	if(genflags & INTR_GENFLAG_LOAD_SYSPAGE) {
		gen_load(REGNO_EBX, (uintptr_t)_syspage_ptr);
	}
	if(genflags & INTR_GENFLAG_LOAD_INTRINFO) {
		gen_load(REGNO_ECX, (uintptr_t)interrupt_level[level].info);
	}
	if(genflags & INTR_GENFLAG_LOAD_INTRMASK) {
		if(cascaded_eoi) {
			gen_load(REGNO_EDX, 0);
		} else {
			/* movzwl mask_count(%esi),%edx */
			gen_modrm(0x0fb7, REGNO_EDX, REGNO_ESI, REGNO_NONE, 0, offsetof(INTRLEVEL, mask_count));
		}
	}
	if(genflags & INTR_GENFLAG_LOAD_CPUNUM) {
		if(NUM_PROCESSORS == 1) {
			gen_load(REGNO_ESI, 0);
		} else {
			static uint8_t cpunum_burst[] = {
				0x31, 0xf6,			// xor %esi,%esi
				0x0f, 0x00, 0xce,	// str %si
				0xc1, 0xee, 0x03,	// shr $3,%esi
				0x83, 0xee, 0x20,	// sub $0x20,%esi
			};

			add_code(cpunum_burst, sizeof(cpunum_burst));
		}
	}
}

static void
gen_queue_load(unsigned level, struct intrinfo_entry *iip) {
	if(iip->id.size == 0 || iip->num_vectors == 1 || iip->cpu_intr_stride != 0) {
		gen_load(REGNO_ESI, (uintptr_t)&interrupt_level[level]);
	} else {
		/* lea 0(,%eax,8),%esi */
		gen_modrm(0x8d, REGNO_ESI, REGNO_NONE, REGNO_EAX, 3, 0);
		/* lea levelbase(,%esi,2),%esi */
		gen_modrm(0x8d, REGNO_ESI, REGNO_NONE, REGNO_ESI, 1,
					(uintptr_t)&interrupt_level[level]);
	}
}

static void
gen_cond_branch(int value, unsigned off) {
	uint8_t		buff[6];
	intptr_t	disp;

	/* cmpl %eax, immediate8 */
	buff[0] = 0x83;
	buff[1] = 0xf8 + REGNO_EAX;
	buff[2] = value;
	add_code(buff, 3);
	/* je off */
	disp = ((off - intrentry_off) - 2);
	if(disp >= -128 && disp <= 127) {
		buff[0] = 0x74;
		buff[1] = disp;
		add_code(buff, 2);
	} else {
		buff[0] = 0x0f;
		buff[1] = 0x84;
		*(uintptr_t *)&buff[2] = disp - 4;
		add_code(buff, 6);
	}
}

static void
gen_transfer(int call, uintptr_t dest) {
	uint8_t		buff[5];
	intptr_t	disp;

#if 0
	if(call) {
		//fake call - put return address into EDX
		gen_load(REGNO_EDX, (uintptr_t)&intrentry_base[intrentry_off+10]);
	}
#endif
	disp = (dest - (uintptr_t)&intrentry_base[intrentry_off]) - 2;
	if(!call && disp >= -128 && disp <= 127) {
		buff[0] = 0xeb;
		buff[1] = disp;
		add_code(buff, 2);
	} else {
		buff[0] = call ? 0xe8 : 0xe9;
		*(uintptr_t *)&buff[1] = disp - 3;
		add_code(buff, 5);
	}
}

#define POST_GENFLAGS	INTR_GENFLAG_LOAD_LEVEL

static void
gen_burst(struct __intrgen_data *burst, unsigned level, int cascaded_eoi) {
	gen_burst_loads(burst->genflags & ~POST_GENFLAGS, level, cascaded_eoi);
	add_code(burst->rtn, burst->size);
	gen_burst_loads(burst->genflags & POST_GENFLAGS, level, cascaded_eoi);
}

static void
gen_intr(struct level_label *labels, unsigned level, struct intrinfo_entry *iip) {
	struct level_label	*combine;
	struct level_label	*llp;
	unsigned			cpu_intr;
	unsigned			num_vectors;
	unsigned			level_base;
	int					loop_check;
	int					primary;
	unsigned			id_off;
	extern uint8_t		intr_entry_start[];
	extern uint8_t		intr_entry_end[];
	extern uint8_t		intr_entry_nmi_start[];
	extern uint8_t		intr_entry_nmi_end[];
	extern uint8_t		intr_process_queue[];
	extern uint8_t		intr_done_chk_fault[];
	extern uint8_t		intr_done[];

	level_base = interrupt_level[level].level_base;
	num_vectors = iip->num_vectors;
	cpu_intr = iip->cpu_intr_base;
	primary = labels[level].primary_level;
	for( ;; ) {
		llp = &labels[level];
		if(!llp->secondary) {
			if(primary < 0) {
				/*
				 * Make interrupts go to our generated code...
				 */
				set_trap(cpu_intr, (void (*)())&intrentry_base[intrentry_off]);
				if(iip->flags & INTR_FLAG_NMI) {
					add_code(intr_entry_nmi_start, intr_entry_nmi_end - intr_entry_nmi_start);
				} else {
					add_code(intr_entry_start, intr_entry_end - intr_entry_start);
				}
			} else {
				/* cascaded */
	 			struct intrinfo_entry *iip2 = interrupt_level[primary].info;
	
				if(!(iip2->flags & INTR_FLAG_CASCADE_IMPLICIT_EOI)) {
					gen_burst(&iip2->eoi, primary, 1);
				}
			}
		}
		llp->id_off = intrentry_off;
		gen_burst(&iip->id, level, 0);
		if(iip->cpu_intr_stride == 0) {
			unsigned			i;

			for(i = 0; i < iip->num_vectors; ++i) {
				struct level_label	*cascade = llp[i].cascade;
		
				if(cascade != NULL) {
					gen_cond_branch(i, cascade->off);
				}
			}
		} else {
			struct level_label	*cascade = llp->cascade;
		
			if(cascade != NULL) {
				gen_transfer(0, (uintptr_t)intrentry_base + cascade->off);
			}
		}
		combine = llp->combine;
		if(combine != NULL) {
			gen_cond_branch(-1, combine->off);
		} else if(!(iip->id.genflags & (INTR_GENFLAG_LOAD_LEVEL|INTR_GENFLAG_NOGLITCH))) {
			gen_cond_branch(-1, llp->done_off);
		}
		gen_queue_load(level_base, iip);
		gen_transfer(1, (uintptr_t)intr_process_queue);
		gen_burst(&iip->eoi, level, 0);
	 	id_off = ~0U;
	 	loop_check = level;
	 	do {
			if(interrupt_level[loop_check].info->eoi.genflags & INTR_GENFLAG_ID_LOOP) {
				id_off = labels[loop_check].id_off;
			}
			loop_check = labels[interrupt_level[loop_check].level_base].primary_level;
		} while(loop_check >= 0);
		if(id_off !=  ~0U) {
			gen_transfer(0, (uintptr_t)intrentry_base + id_off);
		}
		llp->done_off = intrentry_off;
		if(iip->flags & INTR_FLAG_CPU_FAULT) {
			gen_transfer(0, (uintptr_t)intr_done_chk_fault);
		} else {
			gen_transfer(0, (uintptr_t)intr_done);
		}
		if(llp->secondary) break;
		if(--num_vectors == 0) break;
		if(iip->cpu_intr_stride == 0) break;
		cpu_intr += iip->cpu_intr_stride;
		++level;
		++level_base;
	}
}

void
cpu_interrupt_init(unsigned num_levels) {
	struct intrinfo_entry	*iip;
	unsigned				i;
	struct level_label		*labels;
	unsigned				lsize;

	lsize = num_levels * sizeof(*labels);
	labels = _scalloc(lsize);

	calculate_combines(labels);

	for(;;) {
		for(i = 0, iip = intrinfoptr; i < intrinfo_num; ++i, ++iip) {
			unsigned level = get_interrupt_level(NULL, iip->vector_base);

			if(labels[level].off != intrentry_off) {
				/* phase error ... generate everything again */
				labels[level].off = intrentry_off;
				if(intrentry_base != NULL) {
					_sfree(intrentry_base, intrentry_size);
					intrentry_base = NULL;
				}
			}
			gen_intr(labels, level, iip);
		}
		if(intrentry_base != NULL) break;
		intrentry_size = intrentry_off;
		intrentry_base = _smalloc(intrentry_size);
		intrentry_off = 0;
	}
	_sfree(labels, lsize);
}

__SRCVERSION("init_cpu.c $Rev: 212856 $");
