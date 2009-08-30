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

/*************************************************************************
 *************************************************************************
 **
 ** This is a template file for the cpu specific code. If compiled in
 ** it will disable all process level debugging. Check the CPU specific
 ** directory for the real code.
 **
 *************************************************************************
 *************************************************************************/

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
	return ENOTSUP;
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
	return 0;
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
	return ENOTSUP;
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
}

/*
 * Get/Set the alternate register set.
 */
int rdecl
cpu_debug_get_altregs(THREAD *thp, debug_altreg_t *reg) {

	return EINVAL;
}

int rdecl
cpu_debug_set_altregs(THREAD *thp, debug_altreg_t *reg) {

	return EINVAL;
}

__SRCVERSION("cpu_debug.c $Rev: 153052 $");
