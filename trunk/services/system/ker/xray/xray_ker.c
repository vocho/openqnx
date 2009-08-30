/*
 * XRAY profiler/trace kernel support routines
 */

#include <cisco/vendor.h>
#include <externs.h>
#include <xray_defs.h>
#include <cisco/hwinfo.h>

void (*xray_cswitch_callout)(THREAD *);
xray_entry *xray_base = 0;
xray_record *xray_bufend = 0;

typedef struct profile_pidtid_ {
	ushort_t	pid_hi;
	ushort_t	pid_lo;
	ushort_t	tid;
} profile_pidtid;

/*
 * Special routine for saving PIDs and TIDs during context switch
 */
void _xray_pidtid(ulong_t pid, ulong_t tid)
{
    register profile_pidtid	*lptr;
    register xray_entry		*xp = xray_base;
    unsigned long aligned_size;

    /*
     * It's only called by xray_profile_cswitch()
     * when interrupts are disabled, so no atomic
     * instructions are necessary.
     */
    if (xp) {
	if (xp->header.start_xray) {
	    lptr = (profile_pidtid *) ((int)xp->header.xray_pointer + (int)xp);
	    if ((int)lptr + sizeof(profile_pidtid) < (int)xray_bufend) {
		lptr->pid_hi = pid >> 16;
		lptr->pid_lo = pid;
		lptr->tid = tid;
                aligned_size = XRAY_PREC_ALIGN(sizeof(profile_pidtid));
                ((unsigned long)xp->header.xray_pointer) += aligned_size; 
		return;
	    }
 
	    xp->header.xray_done = 1;
	    xp->header.start_xray = 0;
	}
    }
}

static void					(rdecl *orig_mark_running)(THREAD *act);

/*
 * Profiler context switch callout
 */
void rdecl
xray_profile_cswitch(THREAD *thp)
{
	static THREAD *prev_thp = 0;

#ifdef __MIPS__
	/* MIPS needs this to be able to read the CP0 registers */
	thp->reg.regs[MIPS_CREG(MIPS_REG_SREG)] |= MIPS_SREG_CU0;
#endif

	if (thp != prev_thp) {
		_xray_profile(CSWITCH_TAG);
		_xray_pidtid(thp->process->pid, thp->tid+1);
	}

	prev_thp = thp;

	orig_mark_running(thp);
}

/*
 * Trace context switch callout
 */
void rdecl
xray_trace_cswitch(THREAD *thp)
{
	static THREAD *prev_thp = 0;

#ifdef __MIPS__
	/* MIPS needs this to be able to read the CP0 registers */
	thp->reg.regs[MIPS_CREG(MIPS_REG_SREG)] |= MIPS_SREG_CU0;
#endif

	if (thp == prev_thp)
		return;

	if (prev_thp)
		_xray_trace4(CSWITCH_TAG, prev_thp->process->pid,
			     prev_thp->tid+1, thp->process->pid, thp->tid+1);
	prev_thp = thp;
	orig_mark_running(thp);
}

/*
 * XRAY initialization
 */
void
xray_initialize(unsigned version, unsigned pass)
{
	struct hwi_cisco_xray *	xray;
	unsigned int			xray_off;
	xray_entry *			base;

	if(version != LIBMOD_VERSION_CHECK) {
		kprintf("Version mismatch between procnto (%d) and libmod_cisco (%d)\n", version, LIBMOD_VERSION_CHECK);
		crash();
	}
	switch(pass) {
	case 5:
		break;
	default:
		return;
	}

	kprintf("Installing Xray Module\n");

	xray_off = hwi_find_tag(0, 0, HWI_TAG_NAME_cisco_xray);
	xray = hwi_off2tag(xray_off);
	if ( xray == NULL ) {
		kprintf("No xray tag found in syspage!\n");
		return;
	}

	/*
	 * Set xray_base
	 */
	if ((base = (xray_entry *)xray->xray_base)) {
		xray_base = xray->xray_flags & XRAY_FLAGS_CACHEABLE ?
			    (xray_entry *)PHYS_TO_CACHED_ADDR(base) :
			    (xray_entry *)PHYS_TO_UNCACHED_ADDR(base);
		xray_bufend = (xray_record *) ((ulong_t) xray_base +
					       (ulong_t) xray_base->header.xray_bufend);
	} else {
		kprintf("No xray base pointer found!\n");
		return;
	}
	
	/*
	 * Set xray_cswitch_callout
	 */
	if (xray->xray_flags & XRAY_FLAGS_CSWITCH) {
		kprintf("Installing %s Xray Context Switch callout\n",
			xray->xray_flags & XRAY_FLAGS_TRACE ? "Tracing":"Profiling");
		orig_mark_running = mark_running;
		mark_running = xray->xray_flags & XRAY_FLAGS_TRACE ?
				       xray_trace_cswitch :
				       xray_profile_cswitch;
	}
}

__SRCVERSION("xray_ker.c $Rev: 140019 $");
