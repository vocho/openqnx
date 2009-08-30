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

#include <gulliver.h>
#include "externs.h"

extern const char		timestamp[];
extern const char		os_version_string[];

static void
dump_mem32(const char *name, const uint32_t *p, unsigned size) {
	unsigned	i;

	kprintf("\n%s[%p]:", name, p);
	for(i = 0; i < size; i += sizeof(uint32_t)) {
		if((i % (8*sizeof(uint32_t))) == 0) {
			kprintf("\n%04x:", i);
		}
		kprintf(" %08x", *p++);
	}
}

#define VTHP_CHAR	' '
#define INVTHP_CHAR	'?'

#if defined(VARIANT_be) || defined(VARIANT_le)
	#define	ENDIAN	ENDIAN_STRINGNAME
#else
	#define	ENDIAN	""
#endif

#if defined(VARIANT_instr) && defined(VARIANT_g)
#define ENABLE_DUMP_TRACERING
#endif

#ifdef ENABLE_DUMP_TRACERING
void dump_tracering(void)
{
	struct qtime_entry *qtp = SYSPAGE_ENTRY(qtime);
	uint64_t		cyc;
	int				i;
	tracebuf_t		*t_b_p, *start;
	traceevent_t	*e_p;
#ifdef __BIGENDIAN__
		kprintf("TRACE_BIG_ENDIAN: TRUE\n");
#else
		kprintf("TRACE_LITTLE_ENDIAN: TRUE\n");
#endif
		cyc = qtp->cycles_per_sec >> CLOCKCYCLES_INCR_BIT;
		kprintf("TRACE_CYCLES_PER_SEC: 0x%x%x\n", (unsigned)(cyc >> 32), (unsigned)(cyc & 0xffffffff) );
		if ( trace_masks.main_flags != 0 ) {
			trace_masks.main_flags = 0;

			if((t_b_p=privateptr->tracebuf)!=NULL) {
				t_b_p = start = t_b_p->h.next;
				do {
					if ( t_b_p->h.num_events > 0 &&
							!(t_b_p->h.flags & _TRACE_FLAGS_WRITING) ) {
						for( i = 0, e_p = (traceevent_t *)t_b_p->h.begin_ptr; e_p < t_b_p->h.tail_ptr; e_p++, i++ ) {
							kprintf( "%8x %8x %8x %8x%s", 
								e_p->header,
								e_p->data[0],
								e_p->data[1],
								e_p->data[2],
								(i%2) ? "\n":" "
							);
						}
						kprintf("\n");
					}
					t_b_p = t_b_p->h.next;
				} while( t_b_p != start );
			}

		}
}
#endif

void
shutdown(unsigned sigcode, const CPU_REGISTERS *reg) {
	unsigned		ker;
	THREAD			*thp;
	PROCESS			*prp;
	PROCESS			*prp2;
	unsigned		i;
	uint32_t		*stack;
	uint8_t			*ip;
	char			valid_thp;
	static int		shutting_down = 0;	//In case of fault during shutdown()
	extern int		main();

	if(!shutting_down) {
		shutting_down = 1;
		SPINUNLOCK(&debug_slock);
	
#ifdef ENABLE_DUMP_TRACERING
		dump_tracering();
#endif

		kprintf("\nShutdown[%d,%d] S/C/F=%d/%d/%d C/D=%p/%p",
				RUNCPU, KERNCPU,
				SIGCODE_SIGNO(sigcode), SIGCODE_CODE(sigcode), SIGCODE_FAULT(sigcode),
				(void *)&main, (void *)&actives);
	
   		valid_thp = VTHP_CHAR;
		ker = get_inkernel();
		if(ker != 0) {
			kprintf(" state(%x)=", ker);
			if(ker & INKERNEL_NOW) {
				kprintf(" now");
			}
			if(ker & INKERNEL_LOCK) {
				kprintf(" lock");
				valid_thp = INVTHP_CHAR;
			}
			if(ker & INKERNEL_EXIT) {
				kprintf(" exit");
			}
			if(ker & INKERNEL_SPECRET) {
				kprintf(" specret");
				valid_thp = VTHP_CHAR;	// SPECRET overrides LOCK state for valid thp
			}
			if(ker & INKERNEL_INTRMASK) {
				kprintf(" %d", ker & INKERNEL_INTRMASK);
				valid_thp = INVTHP_CHAR;
			}
		}
		kprintf("\nQNX Version %s Release %s", os_version_string, timestamp );
   		for(i = 0; i < NUM_PROCESSORS; ++i) {
			thp = actives[i];
			prp = thp->process;
	
			kprintf("\n[%d]PID-TID=%d-%d%c P/T FL=%08x/%08x",
				i, prp->pid, thp->tid + 1, valid_thp, prp->flags, thp->flags);
			if(prp->debug_name) {
				kprintf(" \"%s\"", prp->debug_name);
			}
			prp2 = actives_prp[i];
			if(prp != prp2) {
				kprintf("\n[%d]ACTIVE PID=%d PF=%08x", i, prp2->pid, prp2->flags);
				if(prp2->debug_name) {
					kprintf(" \"%s\"", prp2->debug_name);
				}
			}
			prp2 = aspaces_prp[i];
			if(prp2 == NULL) {
				kprintf("\n[%d]ASPACE=>NULL", i);
			} else if(prp != prp2) {
				kprintf("\n[%d]ASPACE PID=%d PF=%08x", i, prp2->pid, prp2->flags);
				if(prp2->debug_name) {
					kprintf(" \"%s\"", prp2->debug_name);
				}
			}
		}
	
   		dump_mem32(CPU_STRINGNAME ENDIAN " context", (void *)reg, sizeof(CPU_REGISTERS));
	
		ip = (uint8_t *)REGIP(reg);
		kprintf("\ninstruction[%p]:\n", ip);
		for(i = 0; i < 26; ++i) {
			kprintf("%02x ", *ip++);
		}
	
		stack = (uint32_t *)REGSP(reg);
#ifdef __X86__
		//
		// On the X86, the REGSP(reg) value is not valid if we didn't
		// perform a privity switch. Check to see if we're coming from ring 0
		// and, if so, use the exx value in the context for the stack pointer
		// (that's the ESP from before the PUSHA). The "+ 3*4" is to skip
		// over the EIP/CS/EFLAGS pushed by the exception. Ugh.
		//
		if((reg->cs & 0x3) == 0) {
			stack = (uint32_t *)(reg->exx + 3*4);
		}
#endif
		dump_mem32("stack", stack, 32*4);
		kprintf("\n");
	}
		
	RebootSystem(1);
}

__SRCVERSION("shutdown_nto.c $Rev: 202328 $");
