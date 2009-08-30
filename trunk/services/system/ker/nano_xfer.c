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

#include <unistd.h>
#include <setjmp.h>
#include "externs.h"

#define XFER_IOV_NUM		32		/* Number of iov's to copy at a time */
#define XFER_IOV_LIMIT		((4*1024*1024)/8)		/* Maximum number of iov allowed */

#define XFER_PREEMPT(act)	if(NEED_PREEMPT(act)) { xfer_preempt(act); }

jmp_buf					*xfer_env;

#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
/*
 * This routine is used by the preemption to restore thread state for xfer
 *	kernel is locked before this rutine is entered.
 *
 */
 void xfer_restorestate(THREAD *act) {
	 THREAD *thp; 
	 
	lock_kernel();
	if(act->internal_flags & _NTO_ITF_MSG_DELIVERY) {
		act->internal_flags &= ~_NTO_ITF_MSG_DELIVERY;
		
		/* AsyncMsg store a cop in act->restart */
		if (act->restart && ((CONNECT *)(act->restart))->type == TYPE_CONNECTION) {
			((CONNECT *)(act->restart))->cd = 
			  (struct _asyncmsg_connection_descriptor *)act->args.ms.rparts;
			return;
		}
		
		if((thp = act->blocked_on)) {
			act->blocked_on = 0;
#ifndef NDEBUG
		if((TYPE_MASK(thp->type) != TYPE_THREAD) && (TYPE_MASK(thp->type) != TYPE_VTHREAD)) {
			kprintf("\nWrong type: thp %p type %x act %p act->state %x act->type %x\n",thp, thp->type, act, act->state, act->type);
			return;
		}
#endif
			if(thp->internal_flags & _NTO_ITF_MSG_DELIVERY) {
			 	thp->internal_flags &= ~_NTO_ITF_MSG_DELIVERY;
				if(thp->internal_flags & _NTO_ITF_MSG_FORCE_RDY) {
					force_ready(thp,KSTATUS(thp));
					thp->internal_flags &= ~_NTO_ITF_MSG_FORCE_RDY;
				}
			} else {
#ifndef NDEBUG
//					kprintf("\nblocked thread has not set flag in restart act %x thp %x state %x type %x\n",act,thp,thp->state,thp->type);
#endif
			}
		} else {
#ifndef NDEBUG
			kprintf("\nrestart:  %p %p\n",act,thp);
#endif
		}
	}
 }
#endif

/*
 * This routine is used by the XFER_PREEMPT macro in xfermsg()
 *
 */
static void xfer_preempt(THREAD *thp) {
	lock_kernel();
	if(get_inkernel() & INKERNEL_SPECRET) {
		unspecret_kernel();
	} else {
		CRASHCHECK(TYPE_MASK(thp->type) != TYPE_THREAD);
		SETKIP(thp, KIP(thp) - KER_ENTRY_SIZE);
	}
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
	xfer_restorestate(thp);
#endif
	SET_XFER_HANDLER(NULL);
	__ker_exit();
}

/*
 * This routine is called when the kernel wants to preempt
 * a thread in the middle of a message pass. The current
 * position should be saved so when we restart, it the
 * message pass can continue where it left off. The restart
 * must verify that it is the origional threads to allow the
 * restart to occur.
 * On entry:
 *   thp
 *      thread to preempt (May not be active)
 *
 *   thp->restart
 *      thread that was the source of the message pass
 *
 *   thp->restart->args.ms.msglen
 *      Number of bytes that have been sucessfully transfered
 *
 * On Exit:
 *   thp->restart->args.ms.msglen
 *      Updated using regs to more accurate number (if possible)
 */
void xfer_restart(THREAD *thp, CPU_REGISTERS *regs) {
	if((thp = thp->restart)) {
		thp->args.ms.msglen += xferiov_pos(regs);
	}
	__ker_exit();
}

void xfer_async_restart(THREAD *thp, CPU_REGISTERS *regs) {
	thp->args.ms.msglen += xferiov_pos(regs);
	__ker_exit();
}

/*
 * This is the main message passing function.
 *
 * This function will map other processes address spaces as
 * nessessary to allow the lower level xferiov() to transfer
 * the data. It also converts negative parts into a single
 * iov, so the xferiov() doesn't have to worry about this.
 *   
 * memmgr.aspace() may be called to change the current address
 * space for more efficent transfers. The caller must be aware
 * that when this function returns, the address space may have
 * been changed.
 *
 * memmgr.map_xfer() will be called to map addresses. The first
 * time it is called, lo and hi will be zero. Each time it is called
 * after that, it must be called with the same prp and aspace_prp,
 * and the previous lo and hi mappings to allow the memory manager
 * to perform cleanup if nessessary before mapping the new translation.
 * Before this function exits and if a non-zero translation from
 * a memmgr.map_xfer() was returned, a zero sized call memmgr.map_xfer()
 * will be called to let the memory manager have a chance to clean up.
 *
 * On entry:
 *   dthp
 *      Destination thread
 *
 *   dthp->args.ms.sparts
 *      If negative, if msg is a pointer to a buffer, else pointer to IOV.
 *
 *   dthp->args.ms.smsg
 *      IOV if sparts is positive, buffer if sparts is negative
 *
 *   doff
 *      Number of bytes to skip in destination before starting xfer
 *
 *   sthp
 *      Source thread
 *
 *   sthp->args.ms.sparts
 *      If negative, if msg is a pointer to a buffer, else pointer to IOV.
 *
 *   sthp->args.ms.smsg
 *      IOV if sparts is positive, buffer if sparts is negative
 *  
 *   soff
 *      Number of bytes to skip in source before starting xfer
 *
 * On Exit (return == 0) success:
 *   sthp->args.ms.msglen
 *      Number of bytes accually transfered
 *
 * On Exit (return != 0) Failed:
 *      XFER_SRC_FAULT   Fault in source side
 *      XFER_DST_FAULT   Fault in destination side
 *
 * On Exit (preempted) restarted:
 *   dthp->restart
 *      Destination thread, has pointer to source thread
 *      so it can verify the message pass can continue.
 *
 *   sthp->args.ms.msglen
 *      Number of bytes that have been sucessfully transfered
 */

#define	XFER_FAULTRET(fault)							\
		SET_XFER_HANDLER(0);							\
		actives[KERNCPU]->restart = 0;					\
		return fault									

#define BOUNDRY_CHECK(bound, iov, parts, fault) 		\
	if(xfer_memchk(bound, iov, parts) != 0) {			\
		XFER_FAULTRET(fault);							\
	}

#define OFFSET_SKIP(off, iov, iovparts)					\
	while(off >= GETIOVLEN(iov)) {						\
		off -= GETIOVLEN(iov);							\
		if(--iovparts == 0) { 							\
			SET_XFER_HANDLER(0);						\
			return 0;									\
		}												\
		++iov;											\
	}												

#define OFFSET_SKIP_BR(off, iov, iovparts)				\
	do{													\
		while(off >= GETIOVLEN(iov)) {					\
			off -= GETIOVLEN(iov);						\
			if(--iovparts == 0)break;					\
			++iov;										\
		}												\
		SET_XFER_HANDLER(0);							\
	}while(0)
	

int xfermsg(THREAD *dthp, THREAD *sthp, int doff, int soff) {
	IOV						*dst, *src;
	int						dparts, sparts;
	PROCESS					*prp, *sprp,*dprp;
	unsigned				sflags, dflags;
	int						status;
	IOV						diov[1], siov[1];

	XFER_PREEMPT(actives[KERNCPU]);

	/* If restarted, and threads match, continue where we left off */
	if(dthp->restart != sthp || sthp->restart != sthp || sthp->args.ms.dthp != dthp) {
		sthp->args.ms.msglen = 0;
		sthp->args.ms.dthp = dthp;
		// CAREFUL!  The above operations have to happen before dthp->restart gets updated
		// here.  The compiler can rearrange code, and if it rearranges the assignment to
		// dthp->restart so that it comes before the above, a preemption at a bad time can lead
		// to failure in the xfer.  We need to keep the compiler from rearranging things
		// here, hence we use atomic_order().
		atomic_order(sthp->restart = dthp->restart = sthp);
	} else {
		soff += sthp->args.ms.msglen;
		doff += sthp->args.ms.msglen;
	}

	/* Special case for 1 part source messages. */
	if((sparts = sthp->args.ms.sparts) == 0) {
		return 0;
	}

	if(sparts < 0) {
		SETIOV(siov, sthp->args.ms.smsg, -sparts);
		src = siov;
		sparts = 1;
		sflags = MAPADDR_FLAGS_IOVKERNEL;
	} else {
		if(sparts > XFER_IOV_LIMIT) {
			return XFER_SRC_FAULT;
		}
		src = sthp->args.ms.smsg;
		sflags = 0;
	}

	if(sthp->process->pid == SYSMGR_PID) {
		sflags |= MAPADDR_FLAGS_SYSPRP;
	}
	sprp = sthp->aspace_prp;

	/* Same special case for 1 part dst messages */
	if((dparts = dthp->args.ms.rparts) == 0) {
		return 0;
	}

	if(dparts < 0) {
		SETIOV(diov, dthp->args.ms.rmsg, -dparts);
		dst = diov;
		dparts = 1;
		dflags = MAPADDR_FLAGS_WRITE | MAPADDR_FLAGS_IOVKERNEL;
	} else {
		if(dparts > XFER_IOV_LIMIT) {
			return XFER_DST_FAULT;
		}
		dst = dthp->args.ms.rmsg;
		dflags = MAPADDR_FLAGS_WRITE;
	}

	if(dthp->process->pid == SYSMGR_PID) {
		dflags |= MAPADDR_FLAGS_SYSPRP;
	}
	dprp = dthp->aspace_prp;

	prp = aspaces_prp[KERNCPU];

	/* Find the translation or change address spaces as nessessary */
	/*
	 * dst_aspace  src_aspace  aspace  <=> aspace
	 *    1            2       !(1|2)      1 could not happen (for pipe case, prepare the aspace in read_writev)
	 *    1            NULL    !1          1
	 *    1            2       2           2(NC)*	++
	 *    1            1       1           1(NC)
	 *    1            2       1           1(NC)*
	 *    1            NULL    1           1(NC)
	 *    NULL         NULL    any         same(NC)
	 *    NULL         2       2           2(NC)	
	 *    NULL         2       !2          2
	 */
	if(dprp) {
		if(sprp == prp) {
			if(dprp != prp) {
				IOV						iovlist[XFER_IOV_NUM];
				int						parts;
				int						nbytes;

				/* Check for a src offset and skip over it. */
				if(soff) {
					OFFSET_SKIP(soff, src, sparts);
				}

				do {
					XFER_PREEMPT(actives[KERNCPU]);
					SET_XFER_HANDLER(&xfer_dst_handlers);
					parts = sizeof iovlist / sizeof *iovlist;
					nbytes = memmgr.map_xfer(prp, dprp, (IOV **)&dst, &dparts, &doff, iovlist, &parts, dflags);
					if(nbytes <= 0) {
						if(nbytes == 0) {
							break;
						} else {
							XFER_FAULTRET(XFER_DST_FAULT);
						}
					}
					dflags |= MAPADDR_FLAGS_NOTFIRST;

					SET_XFER_HANDLER(&xfer_src_handlers);
					if((status = xferiov(sthp, iovlist, src, parts, sparts, 0, soff)) != 0) {
						XFER_FAULTRET(status);
					}
					if(dparts == 0 ) {
						break;
					}
					soff += nbytes;
					OFFSET_SKIP_BR(soff,src,sparts);
				} while(sparts);
				SET_XFER_HANDLER(0);
				return 0;
			} else { /* end for (dprp != prp) */
				/* sprp == dprp == prp */
			}
		} else if(sprp) { /* end for (sprp == prp) */
			IOV							iovlist[XFER_IOV_NUM];
			int							parts;
			int							nbytes;


			/* Check for a dst offset and skip over it. */
			if(doff) {
				OFFSET_SKIP(doff, dst, dparts);
			}


			do {
				XFER_PREEMPT(actives[KERNCPU]);
				SET_XFER_HANDLER(&xfer_src_handlers);

				parts = sizeof iovlist / sizeof *iovlist;
				nbytes = memmgr.map_xfer(prp, sprp, (IOV**)&src, &sparts, &soff, iovlist, &parts, sflags);

				if(nbytes <= 0) {
					if(nbytes == 0) {
						break;
					} else {
						XFER_FAULTRET(XFER_SRC_FAULT);
					}
				}
				sflags |= MAPADDR_FLAGS_NOTFIRST;

				if((status = xferiov(sthp, dst, iovlist, dparts, parts, doff, 0)) != 0) {
					XFER_FAULTRET(status);
				}
				if(sparts == 0) {
					break;
				}
				doff += nbytes;
				OFFSET_SKIP_BR(doff,dst,dparts);

			} while(dparts);
			SET_XFER_HANDLER(0);
			return 0;
		} else { /* end for (sprp) */
			if(dprp != prp) {
				lock_kernel();
				memmgr.aspace(dprp, &aspaces_prp[KERNCPU]);
				unlock_kernel();
				XFER_PREEMPT(actives[KERNCPU]);
			}
		}
	} else {
		if(sprp != prp) {
			if(sprp) {
				lock_kernel();
				memmgr.aspace(sprp, &aspaces_prp[KERNCPU]);
				unlock_kernel();
				XFER_PREEMPT(actives[KERNCPU]);
		 	}
		}
	}


	/* Check for a dst offset and skip over it. */
	if(doff) {
		SET_XFER_HANDLER(&xfer_dst_handlers);
		OFFSET_SKIP(doff, dst, dparts);
	}
	
	SET_XFER_HANDLER(&xfer_src_handlers);

	/* Check for a src offset and skip over it. */
	if(soff) {
		OFFSET_SKIP(soff, src, sparts);
	}

	status = xferiov(sthp, dst, src, dparts, sparts, doff, soff);


	SET_XFER_HANDLER(0);
	return status;
}

/* for aysnc msg receive */
int rcvmsg(THREAD *dthp, PROCESS *sprp, void *destp, int destparts, void *srcp, int srcparts) {
	IOV						*dst, *src;
	int						sparts, dparts;
	PROCESS					*dprp;
	unsigned				sflags;
	int						status;
	IOV						diov[1], siov[1];
	int 					soff, doff;

	/* restart check */
	soff = doff = dthp->args.ms.msglen;

	if(srcparts == 0 || destparts == 0) {
		return 0;
	}
	
	/* Special case for 1 part source messages. */
	if(srcparts < 0) {
		SETIOV(siov, srcp, -srcparts);
		src = siov;
		sparts = 1;
		sflags = MAPADDR_FLAGS_IOVKERNEL;
	} else {
		if(srcparts > XFER_IOV_LIMIT) {
			return XFER_SRC_FAULT;
		}
		src = srcp;
   		sparts = srcparts;
		sflags = 0;
	}

	/* Same special case for 1 part dst messages */
	if(destparts < 0) {
		SETIOV(diov, destp, -destparts);
		dst = diov;
		dparts = 1;
	} else {
		if(destparts > XFER_IOV_LIMIT) {
			return XFER_DST_FAULT;
		}
		dst = destp;
		dparts = destparts;
	}

	dprp = dthp->aspace_prp;

	/* act is receiver */
	if (dprp != sprp) {
		IOV							iovlist[XFER_IOV_NUM];
		int							parts = 0;
		int							nbytes;
		
		/* check dst iov addresses */
		SET_XFER_HANDLER(&xfer_dst_handlers);
		if(dprp->pid != SYSMGR_PID) {
			BOUNDRY_CHECK(dprp->boundry_addr, dst, dparts, XFER_DST_FAULT);
		}
		
		/* Check for a dst offset and skip over it. */
		if(doff) {
			OFFSET_SKIP(doff, dst, dparts);
		}

		do {
			XFER_PREEMPT(actives[KERNCPU]);
			SET_XFER_HANDLER(&xfer_async_handlers);
			
			if(sprp->pid != SYSMGR_PID) {
				BOUNDRY_CHECK(sprp->boundry_addr, src, sparts, XFER_DST_FAULT);
			}

			parts = sizeof iovlist / sizeof *iovlist;
			nbytes = memmgr.map_xfer(dprp, sprp, (IOV**)&src, &sparts, &soff, iovlist, &parts, sflags);
			if(nbytes <= 0) {
				if(nbytes == 0) {
					break;
				} else {
					XFER_FAULTRET(XFER_SRC_FAULT);
				}
			}
			sflags |= MAPADDR_FLAGS_NOTFIRST;
			
			if((status = xferiov(dthp, dst, iovlist, dparts, parts, doff, soff)) != 0) {
				XFER_FAULTRET(status);
			}
			if(sparts == 0) {
				break;
			}
			doff += nbytes;
			OFFSET_SKIP_BR(doff,dst,dparts);

		} while(dparts);
		SET_XFER_HANDLER(0);
	} else {
		/* dprp == sprp */
		
		/* check dst iov addresses */
		if(dprp->pid != SYSMGR_PID) {
			SET_XFER_HANDLER(&xfer_dst_handlers);
			BOUNDRY_CHECK(dprp->boundry_addr, dst, dparts, XFER_DST_FAULT);
		}
		
		/* Check for a dst offset and skip over it. */
		if(doff) {
			SET_XFER_HANDLER(&xfer_dst_handlers);
			OFFSET_SKIP(doff, dst, dparts);
		}
		
		if(sprp->pid != SYSMGR_PID) {
			SET_XFER_HANDLER(&xfer_src_handlers);
			BOUNDRY_CHECK(sprp->boundry_addr, src, sparts, XFER_DST_FAULT);
		}
		
		/* Check for a dst offset and skip over it. */
		if(soff) {
			SET_XFER_HANDLER(&xfer_src_handlers);
			OFFSET_SKIP(soff, src, sparts);
		}

		/* xfer */
		SET_XFER_HANDLER(&xfer_async_handlers);
		status = xferiov(dthp, dst, src, dparts, sparts, doff, soff);
		SET_XFER_HANDLER(0);
	}
	
	return EOK;
}

__SRCVERSION("nano_xfer.c $Rev: 199078 $");
