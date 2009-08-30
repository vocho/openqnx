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

/*
 * This routine does enables debugging on a thread. The thread
 * flag _NTO_TF_SSTEP is also set on this thread, so this could
 * be used by the fault handler code if needed. If single stepping
 * is done throught temporary breakpoints, the temp information could
 * be stored in the cpu area off the DEBUG structure so the 
 * cpu_debug_attach_brkpts() function can install them when the
 * thread is being executed.
 * On entry:
 *   dep
 *      DEBUG structure attached to the process being debugged
 *   thp
 *      thread to debug (_NTO_TF_SSTEP) will be set if function succeeds
 * On Exit:
 *   a errno is returned, single step will only occur if EOK is returned.
 */
int rdecl cpu_debug_sstep(DEBUG *dep, THREAD *thp) {
	thp->reg.efl |= X86_PSW_TF;

	//
	// We turn off this bit just in case the thread was running on
	// another cpu.  If it were, the TF bit (set above) would cause
	// it to get an exception in kernel mode when we do the sysexit
	// return to user space.
	//
	thp->reg.efl &= ~(SYSENTER_EFLAGS_BIT);

	return EOK;
}

/*
 * This routine is called when adding or removing breakpoints from the
 * process debug list. It allows selecting of the type of breakpoints
 * and watchpoints supported by the cpu. The breakpoints are not actually
 * written to the address space during this function, but when the process
 * is switched to via cpu_debug_attach_brkpts(). It will only be called
 * after cpu_debug_detach_brkpts() has been called, so it does not need to
 * work about this case.
 * On entry:
 *   dep
 *      DEBUG structure of process to for breakpoints.
 *   bpp
 *      BREAKPT structure with breakpoint to install.
 *      
 * On exit:
 *   a errno is returned, the breakpoint will not be added unless EOK is returned.
 * 
 */
int rdecl cpu_debug_brkpt(DEBUG *dep, BREAKPT *bpp) {
	size_t							size;

	if(bpp->brk.size == -1) {
		return EOK;
	}

	if(bpp->brk.type & _DEBUG_BREAK_EXEC) {
		// Execute must be by itself and it must be size zero
		if(bpp->brk.size != 0 || bpp->brk.type != _DEBUG_BREAK_EXEC) {
			return EINVAL;
		}
	} else if(bpp->brk.type & _DEBUG_BREAK_MODIFY) {
		if(		bpp->brk.size > sizeof bpp->cpu.old ||
				bpp->brk.size == 0 ||
				(size = memmgr.mapinfo(dep->process, bpp->brk.addr, NULL, NULL, 0, NULL, NULL, NULL)) == 0 ||
				bpp->brk.size > size) {
			return EINVAL;
		}
		memcpy(bpp->cpu.old, (void *)bpp->brk.addr, bpp->brk.size);
	} else if(!(bpp->brk.type & _DEBUG_BREAK_RD)) {
		return EINVAL;
	}
	return EOK;
}

/*
 * This routine is called at process switch time from the kernel.
 * It is responsible for modifying the processes address space to
 * plant breakpoints. It needs to take care if the region is not
 * mapped writable, probably calling memmgr.vaddrinfo() to get
 * physical memory that it can write to. If there is no physical
 * memory there (i.e. the page hasn't been faulted in yet) it should
 * skip this breakpoint quietly. When the page is faulted in, it
 * will get another chance to plant the breakpoint and it can't
 * be referenced untill it is faulted in anyhow.
 * On entry:
 *   dep
 *      DEBUG structure of process with breakpoint list.
 */
void rdecl cpu_debug_attach_brkpts(DEBUG *dep) {
	BREAKPT					*d;
	unsigned				dr7;
	volatile uint8_t		*p;
	unsigned				type;
	size_t					size;
	int						hwreg;
	int						step = 0;
	paddr_t					paddr;

	if(!(d = dep->brk)) {
		return;
	}
	dr7 = rddr7();
	hwreg = 0;
	for(; d; d = d->next) {
		if(d != dep->skip_brk) {
//if(__cpu_flags & CPU_FLAG_MMU) {
			if((memmgr.vaddrinfo(dep->process, d->brk.addr, &paddr, &size, VI_PGTBL) == PROT_NONE) || (size < d->brk.size)) {
				d->cpu.hwreg = -1;
				continue;
			}
			d->cpu.hwreg = 0;
//}
			if((type = (d->brk.size - 1) << 2) == ((3 - 1) << 2)) {
				type = (4 - 1) << 2;
			}
			if(d->brk.type & _DEBUG_BREAK_EXEC) {
				unsigned			cr0 = rdcr0();
	
				if(!(d->brk.type & _DEBUG_BREAK_HW)) {
					if(d->planted++)
						continue;
					ldcr0(cr0 & ~X86_MSW_WP_BIT);
					p = (volatile uint8_t *)d->brk.addr;
					d->cpu.old[0] = *p;
					*p = 0xcc;
					ldcr0(cr0);
					
					if(*p == 0xcc || d->cpu.old[0] == 0xcc) {
						continue;
					}
					// Couldn't plant breakpoint (in ROM).
					// Try to use debug regs.
					d->planted = 0;
				}
				type = 0x0;
			} else if(d->brk.type & (_DEBUG_BREAK_WR | _DEBUG_BREAK_MODIFY)) {
				type |= 0x1;
			} else if(d->brk.type & _DEBUG_BREAK_RD) {
				type |= 0x3;
			} else {
				continue;
			}
			if(hwreg >= 4) {
				if(!(d->brk.type & _DEBUG_BREAK_HW)) {
					step = 1;
				}
			} else {
				while(hwreg < 4) {
					if(!(dr7 & (3 << (hwreg * 2)))) {
						dr7 |= 1 << (hwreg * 2);		// set local bit
						dr7 &= ~((0xf << 16) << (hwreg * 4));
						dr7 |= (type << 16) << (hwreg * 4);
						switch(hwreg) {
						case 0:
							wrdr0(d->brk.addr);
							break;
						case 1:
							wrdr1(d->brk.addr);
							break;
						case 2:
							wrdr2(d->brk.addr);
							break;
						case 3:
							wrdr3(d->brk.addr);
							break;
						default: break;
						}			
						d->cpu.hwreg = ++hwreg;
						break;
					}
					hwreg++;
				}
			}
		}
	}
	if(step) { 		// do something to force single step
		// @@@ need to set singlestep on all threads in process...
		actives[KERNCPU]->reg.efl |= X86_PSW_TF;
	}
	wrdr7(dr7);
}

/*
 * This routine is called at process switch time from the kernel.
 * It is responsible for modifying the processes address space to
 * remove breakpoints. It needs to take care if the region is not
 * mapped writable, probably calling memmgr.vaddrinfo() to get
 * physical memory that it can write to. It should only remove
 * things that where modified during cpu_debug_attach_brkpts().
 * On entry:
 *   dep
 *      DEBUG structure of process with breakpoint list.
 */
void rdecl cpu_debug_detach_brkpts(DEBUG *dep) {
	BREAKPT					*d;
	unsigned				dr7;

	if(!(d = dep->brk)) {
		return;
	}
	dr7 = rddr7();
	for(; d; d = d->next) {
		if(d != dep->skip_brk) {
			if(d->cpu.hwreg > 0) {
				dr7 &= ~(3 << ((d->cpu.hwreg - 1)*2));
				d->cpu.hwreg = 0;
			} else if(d->cpu.hwreg == 0 && d->brk.type == _DEBUG_BREAK_EXEC) {
				if(d->planted) {
					if(--d->planted == 0) {
						char					*p = (char *)d->brk.addr;
						unsigned				cr0 = rdcr0();
	
						ldcr0(cr0 & ~X86_MSW_WP_BIT);
						*p = d->cpu.old[0];
						ldcr0(cr0);
					}
				}
			}
		}
	}
	wrdr7(dr7);
}

/*
 * This routine is called to allow cpu specific modifying and identifying
 * during a fault. If it is reporting a fault, it must turn off any
 * single step that was enabled by cpu_debug_sstep. If needed, this routine
 * could scan "MODIFY" breakpoints (if they aren't done with hardware) to
 * see if they changed.
 * On entry:
 *   dep
 *      DEBUG structure attached to the process (for cpu specific information)
 *   thp
 *      thread that had the fault (KIP(thp) is the ip that had the faulting instruction)
 *   psigno
 *      Pointer to signal number (This can be modified if nessessary)
 *   psigcode
 *      Pointer to signal code (This can be modified if nessessary)
 *   pfault
 *      Pointer to fault code (This can be modified if nessessary)
 *   pflags
 *      Pointer to debug flags (_DEBUG_FLAG_*). If a data watchpoint was
 *      triggered, the corispondinf _DEBUG_FLAG_TRACE_* flag should be set.
 *      If it was an execution breakpoint, _DEBUG_FLAG_ISTOP should be set.
 *      If it was a single step fault, _DEBUG_FLAGS_SSTEP should be set.
 * On Exit (non-zero):
 *   A non-zero return value, causes the fault to be ignored, and the thread
 *   will try to restart at the faulting instruction. This could be used to
 *   make watchpoints by having all the threads in a process single step, then
 *   check after each instruction for the modified memory. If nothing is modified,
 *   continue the thread so the single step occurs after the next instruction.
 * On Exit (zero):
 *   psigno
 *   psigcode
 *   psigfault
 *       These should be modified if nessessary to reflect the real values.
 *   pflags
 *       This should have all _DEBUG_FLAGS set that this fault matches
 */
int rdecl cpu_debug_fault(DEBUG *dep, THREAD *thp, siginfo_t *info, unsigned *pflags) {
	unsigned					dr6;

	dr6 = rddr6();
//	dr7 = rddr7();
	wrdr6(dr6 & ~0x700F);

	// Must change a hardware watchpoint for executing to FLTBPT. Also
	// check for any non-hardware watchpoints that may have changed. If
	// the thread is not being single-stepped, return a non-zero value so
	// fault is ignored.
	if(info->si_fltno == FLTTRACE) {
		BREAKPT					*d;

		// If watchtype == exec, change fault to FLTBPT
		// if watchtype == singlestep and no singlestep flag set and no watchpoints changed return 1
		for(d = dep->brk; d; d = d->next) {
			if(d->cpu.hwreg >= 0) {
				if((d->brk.type & _DEBUG_BREAK_MODIFY) && memcmp((void *)d->brk.addr, d->cpu.old, d->brk.size)) {
					*pflags |= _DEBUG_FLAG_TRACE_MODIFY;
				} else if(d->cpu.hwreg && (dr6 & (1 << (d->cpu.hwreg - 1)))) {
					if(d->brk.type == _DEBUG_BREAK_EXEC) {
						info->si_fltno = FLTBPT;
						break;
					}
					if(d->brk.type & _DEBUG_BREAK_RD) {
						*pflags |= _DEBUG_FLAG_TRACE_RD;
						break;
					}
				}
			}
		}
		if((thp->flags & _NTO_TF_SSTEP) && (dr6 & (1 << 14))) {
			*pflags |= _DEBUG_FLAG_SSTEP;
			thp->flags &= ~_NTO_TF_SSTEP;
		}
	}

	// Flag a breakpoint as a TRACE_EXEC
	if(info->si_fltno == FLTBPT) {
		*pflags |= _DEBUG_FLAG_TRACE_EXEC;
	}

	if(!*pflags && (dr6 & (1 << 14))) {
		return 1;
	}

	thp->reg.efl &= ~X86_PSW_TF;

	return 0;
}

int rdecl
cpu_debug_get_altregs(THREAD *thp, debug_altreg_t *reg) {

	return EINVAL;
}

int rdecl
cpu_debug_set_altregs(THREAD *thp, debug_altreg_t *reg) {

	return EINVAL;
}

__SRCVERSION("cpu_debug.c $Rev: 173896 $");
