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
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


    Authors: Jerome Stadelmann (JSN), Pietro Descombes (PDB), Daniel Rossier (DRE)
    Emails: <firstname.lastname@heig-vd.ch>
    Copyright (c) 2009 Reconfigurable Embedded Digital Systems (REDS) Institute from HEIG-VD, Switzerland
 */

#include "externs.h"
#include "apm.h"
//#define VARIANT_instr	/* PDB: TO BE REMOVED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#if defined(VARIANT_instr)	/* PDB: condition is true */
#include "kertrace.h"
#include "mt_kertrace.h"


static int check_class(int class) {
	switch(class) {
		case _NTO_TRACE_EMPTY:
		case _NTO_TRACE_CONTROL:
		case _NTO_TRACE_KERCALL:
		case _NTO_TRACE_KERCALLENTER:
		case _NTO_TRACE_KERCALLEXIT:
		case _NTO_TRACE_KERCALLINT:
		case _NTO_TRACE_INT:
		case _NTO_TRACE_INTENTER:
		case _NTO_TRACE_INTEXIT:
		case _NTO_TRACE_INT_HANDLER_ENTER:
		case _NTO_TRACE_INT_HANDLER_EXIT:
		case _NTO_TRACE_PROCESS:
		case _NTO_TRACE_THREAD:
		case _NTO_TRACE_VTHREAD:
		case _NTO_TRACE_SYSTEM:
		case _NTO_TRACE_COMM:
		{
			return (EOK);
		}
		default:
		{
			return (ENOTSUP);
		}
	}
}

static int get_int_level(unsigned v)
{
	switch(v) {
		case _NTO_TRACE_INTFIRST:
			return (0);
		case _NTO_TRACE_INTLAST:
		{
			       unsigned        i;
			       int             l  =0;
			struct intrinfo_entry* iip=intrinfoptr;

			for(i=0;i<intrinfo_num;++i,++iip) {
				l += iip->num_vectors;
			}

			return (l);
		}
		default:
			return (get_interrupt_level(NULL, v));
	}
}

static int check_call_masks()
{
	unsigned i;
	unsigned j;

	for(i=0,j=0;i<ker_call_entry_num;++i) {
		if((_TRACE_EXIT_CALL|_TRACE_ENTER_CALL)&trace_masks.ker_call_masks[i]) {
			_trace_call_table[i] = _trace_ker_call_table[i];
			++j;
		} else {
			_trace_call_table[i] = ker_call_table[i];
		}
	}
	if(trace_masks.comm_mask[_NTO_TRACE_COMM_RMSG  ]&_TRACE_ENTER_COMM ||
	   trace_masks.comm_mask[_NTO_TRACE_COMM_RPULSE]&_TRACE_ENTER_COMM) {
		j++;
	}
	ker_exit_enable_mask = j?(0xffffffff):(0x00000000);

	return (EOK);
}

static int add_eh
(
	THREAD*           thp,
	ehandler_data_t** loc,
	void*             handler,
	event_data_t*     area
)
{
	InterruptLock(&trace_masks.eh_spin);
	if(thp->process&&trace_masks.eh_num<_TRACE_MAX_EV_HANDLER_NUM) {
		unsigned i;

		++trace_masks.eh_num;
		for(i=0;;++i) {
			if(trace_masks.eh_storage[i].handler==NULL) {
				trace_masks.eh_storage[i].area     = area;
				trace_masks.eh_storage[i].thp      = thp;
				trace_masks.eh_storage[i].process  = thp->process;
				trace_masks.eh_storage[i].location = loc;
				trace_masks.eh_storage[i].handler  = (int (*)(event_data_t*)) handler;
				_TRACE_SAVE_REGS(thp, &trace_masks.eh_storage[i].cpu);
				*loc = trace_masks.eh_storage + i;
				InterruptUnlock(&trace_masks.eh_spin);

				return (EOK);
			}
		}
	} else {
		InterruptUnlock(&trace_masks.eh_spin);

		return (ENOTSUP);
	}
}

static int delete_eh(ehandler_data_t** loc)
{
	InterruptLock(&trace_masks.eh_spin);
	if(*loc) {
		ehandler_data_t* eh_p=*loc;

		*loc           = NULL;
		eh_p->handler  = NULL;
		eh_p->area     = NULL;
		eh_p->thp      = NULL;
		eh_p->process  = NULL;
		eh_p->location = NULL;
		--trace_masks.eh_num;
	}
	InterruptUnlock(&trace_masks.eh_spin);

	return (EOK);
}

//Local macros used only within kernel trace call.
#define _TRACE_ADD_EH(loc) \
((a_1)=(loc),(a_2)=(*(++c_p)),(a_3)=(*(++c_p)),(add_eh(act,a_1,(void*)a_2,(event_data_t*)a_3)))

#define _TRACE_DOUBLE_ADD_EH(l_1, l_2)                     \
(a_0=l_1,a_1=l_2,a_2=*(++c_p),a_3=*(++c_p),                \
 ((add_eh(act,a_0,(void*)a_2,(event_data_t*)a_3)==(EOK) && \
   add_eh(act,a_1,(void*)a_2,(event_data_t*)a_3)==(EOK))   \
   ?(EOK):(ENOTSUP)))

int kdecl
ker_trace_event(THREAD *act,struct kerargs_trace_event *kap) {
	unsigned*         c_p;
	ehandler_data_t** a_0;
	ehandler_data_t** a_1;
	unsigned          a_2;
	unsigned          a_3;

	// Verify parameters are good before we lock.
	// FIXME: currently we only verify parameters for user-provided events.
	// Maybe we should also verify parameters in other event types?
	RD_PROBE_INT(act, kap->data, sizeof(*kap->data)/sizeof(int));
	switch (_TRACE_GET_FLAG(*(c_p=(unsigned *)kap->data))) {
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTCUSEREVENT):
		{
			if (*(c_p+3) > 0) {
				RD_VERIFY_PTR(actives[RUNCPU], (void*)*(c_p+2), (*(c_p+3))*sizeof(int));
				RD_PROBE_INT(actives[RUNCPU], (uint32_t*)*(c_p+2), *(c_p+3));
			}
			break;
		}

		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTUSRSTREVENT):
		{
			register char* 	s=(char*)*(c_p+2);
			register char* s2 = s;
			unsigned l;
			// Run the string to make sure we can access all of it.
			while(*s++) {
				// nothing to do
			}
			l = s - (char*)*(c_p+2);
			// Double check that the string is in the user space.
			RD_VERIFY_PTR(actives[RUNCPU], s2, l);
			break;
		}
		default:
			;
	}

	// FIXME: for many of the cases we're locking far earlier than we need to.  This
	// makes it clear in each case that we are locked, but at the cost of locking
	// earlier than we really need to.  We may want to refrain from locking here
	// and place a lock at the appropriate point in each case.  This is made more
	// complex by the fact that the "appropriate place" in some cases is well inside
	// the trace_event function, but that function is invoked here and also invoked
	// directly from interrupt code through inside_trace_event.
	lock_kernel();
	switch(_TRACE_GET_FLAG(*c_p)) {
		case _TRACE_GET_FLAG(_NTO_TRACE_ADDALLCLASSES): // Adding all classes
		{
			unsigned i;

			int_enter_enable_mask      = (0xffffffff);
			int_exit_enable_mask       = (0xffffffff);
			ker_exit_enable_mask       = (0xffffffff);
			trace_masks.control_mask   = (0xffffffff);
			trace_masks.pr_mask        = (0xffffffff);
			trace_masks.th_mask        |= ~_TRACE_THREAD_ARG_WIDE;
			trace_masks.vth_mask       = (0xffffffff);

			for(i=0;i<_TRACE_MAX_USER_NUM;++i) {
				trace_masks.user_mask[i] |= _TRACE_ENTER_USER;
			}

			for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
				trace_masks.system_mask[i] |= _TRACE_ENTER_SYSTEM;
			}

			for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
				trace_masks.comm_mask[i] |= _TRACE_ENTER_COMM;
			}

			for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
				trace_masks.int_masks[i] |= (_TRACE_ENTER_INT|_TRACE_EXIT_INT);
				trace_masks.int_handler_masks[i] |= (_TRACE_ENTER_INT|_TRACE_EXIT_INT);
			}

			for(i=0;i<ker_call_entry_num;++i) {
				_trace_call_table         [i]  = _trace_ker_call_table[i];
				trace_masks.ker_call_masks[i] |= (_TRACE_ENTER_CALL|_TRACE_EXIT_CALL);
			}

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_DELALLCLASSES): // Deleting all classes
		{
			unsigned i;

			int_enter_enable_mask      = (0x00000000);
			int_exit_enable_mask       = (0x00000000);
			ker_exit_enable_mask       = (0x00000000);
			trace_masks.control_mask   = (0x00000000);
			trace_masks.pr_mask        = (0x00000000);
			trace_masks.th_mask        &= _TRACE_THREAD_ARG_WIDE;
			trace_masks.vth_mask       = (0x00000000);

			for(i=0;i<_TRACE_MAX_USER_NUM;++i) {
				trace_masks.user_mask[i] &= ~(_TRACE_ENTER_USER);
			}

			for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
				trace_masks.system_mask[i] &= ~(_TRACE_ENTER_SYSTEM);
			}

			for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
				trace_masks.comm_mask[i] &= ~(_TRACE_ENTER_COMM);
			}

			for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
				trace_masks.int_masks[i] &= ~(_TRACE_ENTER_INT|_TRACE_EXIT_INT);
				trace_masks.int_handler_masks[i] &= ~(_TRACE_ENTER_INT|_TRACE_EXIT_INT);
			}

			for(i=0;i<ker_call_entry_num;++i) {
				trace_masks.ker_call_masks[i] &= ~(_TRACE_ENTER_CALL|_TRACE_EXIT_CALL);
				_trace_call_table         [i]  = ker_call_table[i];
			}

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETCLASSPID): // Selecting class process ID
		{
			unsigned class=*(++c_p);
			unsigned pid  =*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				case _NTO_TRACE_KERCALLENTER:
				case _NTO_TRACE_KERCALLEXIT:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_pid[i] = pid;
						trace_masks.ker_call_tid[i] = NULL;
					}

					return (EOK);
				}
				case _NTO_TRACE_THREAD:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_TH_STATE_NUM;++i) {
						trace_masks.thread_pid[i] = pid;
						trace_masks.thread_tid[i] = NULL;
					}

					return (EOK);
				}
				case _NTO_TRACE_VTHREAD:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_TH_STATE_NUM;++i) {
						trace_masks.vthread_pid[i] = pid;
						trace_masks.vthread_tid[i] = NULL;
					}

					return (EOK);
				}
				case _NTO_TRACE_SYSTEM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
						trace_masks.system_pid[i] = pid;
						trace_masks.system_tid[i] = NULL;
					}

					return (EOK);
				}
				case _NTO_TRACE_COMM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
						trace_masks.comm_pid[i] = pid;
						trace_masks.comm_tid[i] = NULL;
					}

					return (EOK);
				}
				default:
				{
					return (ENOTSUP);
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETCLASSTID): // Selecting class thread ID
		{
			unsigned class=*(++c_p);
			unsigned pid  =*(++c_p);
			unsigned tid  =*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				case _NTO_TRACE_KERCALLENTER:
				case _NTO_TRACE_KERCALLEXIT:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_pid[i] = pid;
						trace_masks.ker_call_tid[i] = tid;
					}

					return (EOK);
				}
				case _NTO_TRACE_THREAD:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_TH_STATE_NUM;++i) {
						trace_masks.thread_pid[i] = pid;
						trace_masks.thread_tid[i] = tid;
					}

					return (EOK);
				}
				case _NTO_TRACE_VTHREAD:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_TH_STATE_NUM;++i) {
						trace_masks.vthread_pid[i] = pid;
						trace_masks.vthread_tid[i] = tid;
					}

					return (EOK);
				}
				case _NTO_TRACE_SYSTEM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
						trace_masks.system_pid[i] = pid;
						trace_masks.system_tid[i] = tid;
					}

					return (EOK);
				}
				case _NTO_TRACE_COMM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
						trace_masks.comm_pid[i] = pid;
						trace_masks.comm_tid[i] = tid;
					}

					return (EOK);
				}
				default:
				{
					return (ENOTSUP);
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_CLRCLASSPID): // Clearing class process ID
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				case _NTO_TRACE_KERCALLENTER:
				case _NTO_TRACE_KERCALLEXIT:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						if(trace_masks.ker_call_tid[i]==NULL) {
							trace_masks.ker_call_pid[i] = NULL;
						}
					}

					return (EOK);
				}
				case _NTO_TRACE_THREAD:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_TH_STATE_NUM;++i) {
						if(trace_masks.thread_tid[i]==NULL) {
							trace_masks.thread_pid[i] = NULL;
						}
					}

					return (EOK);
				}
				case _NTO_TRACE_VTHREAD:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_TH_STATE_NUM;++i) {
						if(trace_masks.vthread_tid[i]==NULL) {
							trace_masks.vthread_pid[i] = NULL;
						}
					}

					return (EOK);
				}
				case _NTO_TRACE_SYSTEM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
						if(trace_masks.system_tid[i]==NULL) {
							trace_masks.system_pid[i] = NULL;
						}
					}

					return (EOK);
				}
				case _NTO_TRACE_COMM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
						if(trace_masks.comm_tid[i]==NULL) {
							trace_masks.comm_pid[i] = NULL;
						}
					}

					return (EOK);
				}
				default:
				{
					return (check_class(class));
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_CLRCLASSTID): // Clearing class thread ID
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				case _NTO_TRACE_KERCALLENTER:
				case _NTO_TRACE_KERCALLEXIT:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						if (trace_masks.ker_call_tid[i]) {
							trace_masks.ker_call_pid[i] = NULL;
							trace_masks.ker_call_tid[i] = NULL;
						}
					}

					return (EOK);
				}
				case _NTO_TRACE_THREAD:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_TH_STATE_NUM;++i) {
						if (trace_masks.thread_tid[i]) {
							trace_masks.thread_pid[i] = NULL;
							trace_masks.thread_tid[i] = NULL;
						}
					}

					return (EOK);
				}
				case _NTO_TRACE_VTHREAD:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_TH_STATE_NUM;++i) {
						if (trace_masks.vthread_tid[i]) {
							trace_masks.vthread_pid[i] = NULL;
							trace_masks.vthread_tid[i] = NULL;
						}
					}

					return (EOK);
				}
				case _NTO_TRACE_SYSTEM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
						if(trace_masks.system_tid[i]==NULL) {
							trace_masks.system_pid[i] = NULL;
							trace_masks.system_tid[i] = NULL;
						}
					}

					return (EOK);
				}
				case _NTO_TRACE_COMM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
						if(trace_masks.comm_tid[i]==NULL) {
							trace_masks.comm_pid[i] = NULL;
							trace_masks.comm_tid[i] = NULL;
						}
					}

					return (EOK);
				}
				default:
				{
					return (check_class(class));
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETEVENTPID): // Selecting one event process ID
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				case _NTO_TRACE_KERCALLENTER:
				case _NTO_TRACE_KERCALLEXIT:
				{
					unsigned e=*(++c_p);

					if(e<ker_call_entry_num) {
						trace_masks.ker_call_pid[e] = *(++c_p);
						trace_masks.ker_call_tid[e] = NULL;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_THREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM) {
						trace_masks.thread_pid[i_2] = *(++c_p);
						trace_masks.thread_tid[i_2] = NULL;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_VTHREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM) {
						trace_masks.vthread_pid[i_2] = *(++c_p);
						trace_masks.vthread_tid[i_2] = NULL;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_SYSTEM:
				{
					unsigned e=*(++c_p);

					if(e<_TRACE_MAX_SYSTEM_NUM) {
						trace_masks.system_pid[e] = *(++c_p);
						trace_masks.system_tid[e] = NULL;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_COMM:
				{
					unsigned e=*(++c_p);

					if(e<_TRACE_MAX_COMM_NUM) {
						trace_masks.comm_pid[e] = *(++c_p);
						trace_masks.comm_tid[e] = NULL;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				default:
				{
					return (ENOTSUP);
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETEVENTTID): // Selecting one event thread ID
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				case _NTO_TRACE_KERCALLENTER:
				case _NTO_TRACE_KERCALLEXIT:
				{
					unsigned e=*(++c_p);

					if(e<ker_call_entry_num) {
						trace_masks.ker_call_pid[e] = *(++c_p);
						trace_masks.ker_call_tid[e] = *(++c_p);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_THREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM) {
						trace_masks.thread_pid[i_2] = *(++c_p);
						trace_masks.thread_tid[i_2] = *(++c_p);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_VTHREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM) {
						trace_masks.vthread_pid[i_2] = *(++c_p);
						trace_masks.vthread_tid[i_2] = *(++c_p);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_SYSTEM:
				{
					unsigned e=*(++c_p);

					if(e<_TRACE_MAX_SYSTEM_NUM) {
						trace_masks.system_pid[e] = *(++c_p);
						trace_masks.system_tid[e] = *(++c_p);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_COMM:
				{
					unsigned e=*(++c_p);

					if(e<_TRACE_MAX_COMM_NUM) {
						trace_masks.comm_pid[e] = *(++c_p);
						trace_masks.comm_tid[e] = *(++c_p);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				default:
				{
					return (ENOTSUP);
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_CLREVENTPID): // Clearing one event process ID
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				case _NTO_TRACE_KERCALLENTER:
				case _NTO_TRACE_KERCALLEXIT:
				{
					if((*(++c_p))<ker_call_entry_num &&
					   trace_masks.ker_call_tid[*c_p]==NULL) {
						trace_masks.ker_call_pid[*c_p] = NULL;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_THREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM &&
					   trace_masks.thread_tid[i_2]==NULL) {
						trace_masks.thread_pid[i_2] = NULL;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_VTHREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						//nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM &&
					   trace_masks.vthread_tid[i_2]==NULL) {
						trace_masks.vthread_pid[i_2] = NULL;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_SYSTEM:
				{
					if((*(++c_p))<_TRACE_MAX_SYSTEM_NUM &&
					   trace_masks.system_tid[*c_p]==NULL) {
						trace_masks.system_pid[*c_p] = NULL;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_COMM:
				{
					if((*(++c_p))<_TRACE_MAX_COMM_NUM &&
					   trace_masks.comm_tid[*c_p]==NULL) {
						trace_masks.comm_pid[*c_p] = NULL;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				default:
				{
					return (check_class(class));
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_CLREVENTTID): // Clearing one event thread ID
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				case _NTO_TRACE_KERCALLENTER:
				case _NTO_TRACE_KERCALLEXIT:
				{
					if((*(++c_p))<ker_call_entry_num) {
						if (trace_masks.ker_call_tid[*c_p]) {
							trace_masks.ker_call_pid[*c_p] = NULL;
							trace_masks.ker_call_tid[*c_p] = NULL;
						}

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_THREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM) {
						if (trace_masks.thread_tid[i_2]) {
							trace_masks.thread_pid[i_2] = NULL;
							trace_masks.thread_tid[i_2] = NULL;
						}

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_VTHREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM) {
						if (trace_masks.vthread_tid[i_2]) {
							trace_masks.vthread_pid[i_2] = NULL;
							trace_masks.vthread_tid[i_2] = NULL;
						}

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_SYSTEM:
				{
					if((*(++c_p))<_TRACE_MAX_SYSTEM_NUM) {
						if (trace_masks.system_tid[*c_p]) {
							trace_masks.system_pid[*c_p] = NULL;
							trace_masks.system_tid[*c_p] = NULL;
						}

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
					case _NTO_TRACE_COMM:
				{
					if((*(++c_p))<_TRACE_MAX_COMM_NUM) {
						if (trace_masks.comm_tid[*c_p]) {
							trace_masks.comm_pid[*c_p] = NULL;
							trace_masks.comm_tid[*c_p] = NULL;
						}

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				default:
				{
					return (check_class(class));
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETALLCLASSESFAST): // Setting all classes fast
		{
			unsigned i;

			for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
				trace_masks.system_mask[i] &= ~_TRACE_SYSTEM_ARG_WIDE;
			}

			for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
				trace_masks.comm_mask[i] &= ~_TRACE_COMM_ARG_WIDE;
			}

			for(i=0;i<ker_call_entry_num;++i) {
				trace_masks.ker_call_masks[i] &= ~(_TRACE_CALL_ARG_WIDE|_TRACE_CALL_RET_WIDE);
			}

			trace_masks.th_mask &= ~_TRACE_THREAD_ARG_WIDE;

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETALLCLASSESWIDE): // Setting all classes wide
		{
			unsigned i;

			for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
				trace_masks.system_mask[i] |= _TRACE_SYSTEM_ARG_WIDE;
			}

			for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
				trace_masks.comm_mask[i] |= _TRACE_COMM_ARG_WIDE;
			}

			for(i=0;i<ker_call_entry_num;++i) {
				trace_masks.ker_call_masks[i] |= (_TRACE_CALL_ARG_WIDE|_TRACE_CALL_RET_WIDE);
			}

			trace_masks.th_mask |= _TRACE_THREAD_ARG_WIDE;

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_ADDCLASS): // Adding one class
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_USER:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_USER_NUM;++i) {
						trace_masks.user_mask[i] |= (_TRACE_ENTER_USER);
					}

					return (EOK);
				}
				case _NTO_TRACE_CONTROL:
				{
					trace_masks.control_mask = (0xffffffff);

					return (EOK);
				}
				case _NTO_TRACE_KERCALL:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] |= (_TRACE_ENTER_CALL|_TRACE_EXIT_CALL);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] |= _TRACE_ENTER_CALL;
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] |= _TRACE_EXIT_CALL;
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_INT:
				{
					unsigned i;

					int_enter_enable_mask = (0xffffffff);
					int_exit_enable_mask  = (0xffffffff);
					for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
						trace_masks.int_masks[i] |= (_TRACE_ENTER_INT|_TRACE_EXIT_INT);
						trace_masks.int_handler_masks[i] |= (_TRACE_ENTER_INT|_TRACE_EXIT_INT);
					}

					return (EOK);
				}
				case _NTO_TRACE_INTENTER:
				{
					unsigned i;

					int_enter_enable_mask = (0xffffffff);
					for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
						trace_masks.int_masks[i] |= _TRACE_ENTER_INT;
					}

					return (EOK);
				}
				case _NTO_TRACE_INTEXIT:
				{
					unsigned i;

					int_exit_enable_mask = (0xffffffff);
					for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
						trace_masks.int_masks[i] |= _TRACE_EXIT_INT;
					}

					return (EOK);
				}
				case _NTO_TRACE_INT_HANDLER_ENTER:
				{
					unsigned i;

					int_enter_enable_mask = (0xffffffff);
					for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
						trace_masks.int_handler_masks[i] |= _TRACE_ENTER_INT;
					}

					return (EOK);
				}
				case _NTO_TRACE_INT_HANDLER_EXIT:
				{
					unsigned i;

					int_exit_enable_mask = (0xffffffff);
					for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
						trace_masks.int_handler_masks[i] |= _TRACE_EXIT_INT;
					}

					return (EOK);
				}
				case _NTO_TRACE_PROCESS:
				{
					trace_masks.pr_mask = (0xffffffff);

					return (EOK);
				}
				case _NTO_TRACE_THREAD:
				{
					trace_masks.th_mask = (0xffffffff);

					return (EOK);
				}
				case _NTO_TRACE_VTHREAD:
				{
					trace_masks.vth_mask = (0xffffffff);

					return (EOK);
				}
				case _NTO_TRACE_SYSTEM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
						trace_masks.system_mask[i] |= _TRACE_ENTER_SYSTEM;
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_COMM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
						trace_masks.comm_mask[i] |= _TRACE_ENTER_COMM;
					}

					return (check_call_masks());
				}
				default:
				{
					return (check_class(class));
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_DELCLASS): // Deleting one class
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_USER:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_USER_NUM;++i) {
						trace_masks.user_mask[i] &= ~(_TRACE_ENTER_USER);
					}

					return (EOK);
				}
				case _NTO_TRACE_CONTROL:
				{
					trace_masks.control_mask = (0x00000000);

					return (EOK);
				}
				case _NTO_TRACE_KERCALL:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] &= ~(_TRACE_ENTER_CALL|_TRACE_EXIT_CALL);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] &= ~(_TRACE_ENTER_CALL);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] &= ~(_TRACE_EXIT_CALL);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_INT:
				{
					unsigned i;

					int_enter_enable_mask = (0x00000000);
					int_exit_enable_mask  = (0x00000000);
					for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
						trace_masks.int_masks[i] &= ~(_TRACE_ENTER_INT|_TRACE_EXIT_INT);
						trace_masks.int_handler_masks[i] &= ~(_TRACE_ENTER_INT|_TRACE_EXIT_INT);
					}

					return (EOK);
				}
				case _NTO_TRACE_INTENTER:
				{
					unsigned i;

					int_enter_enable_mask = (0x00000000);
					for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
						trace_masks.int_masks[i] &= ~(_TRACE_ENTER_INT);
					}

					return (EOK);
				}
				case _NTO_TRACE_INTEXIT:
				{
					unsigned i;

					int_exit_enable_mask = (0x00000000);
					for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
						trace_masks.int_masks[i] &= ~(_TRACE_EXIT_INT);
					}

					return (EOK);
				}
				case _NTO_TRACE_INT_HANDLER_ENTER:
				{
					unsigned i;

					int_enter_enable_mask = (0x00000000);
					for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
						trace_masks.int_handler_masks[i] &= ~(_TRACE_ENTER_INT);
					}

					return (EOK);
				}
				case _NTO_TRACE_INT_HANDLER_EXIT:
				{
					unsigned i;

					int_exit_enable_mask = (0x00000000);
					for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
						trace_masks.int_handler_masks[i] &= ~(_TRACE_EXIT_INT);
					}

					return (EOK);
				}
				case _NTO_TRACE_PROCESS:
				{
					trace_masks.pr_mask = (0x00000000);

					return (EOK);
				}
				case _NTO_TRACE_THREAD:
				{
					trace_masks.th_mask = (0x00000000);

					return (EOK);
				}
				case _NTO_TRACE_VTHREAD:
				{
					trace_masks.vth_mask = (0x00000000);

					return (EOK);
				}
				case _NTO_TRACE_SYSTEM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
						trace_masks.system_mask[i] &= ~(_TRACE_ENTER_SYSTEM);
					}

					return (check_call_masks());
					}
				case _NTO_TRACE_COMM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
						trace_masks.comm_mask[i] &= ~(_TRACE_ENTER_COMM);
					}

					return (check_call_masks());
				}
				default:
				{
					return (check_class(class));
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETCLASSFAST): // Setting fast one class
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] &= ~(_TRACE_CALL_ARG_WIDE|_TRACE_CALL_RET_WIDE);
					}

					return (EOK);
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] &= ~(_TRACE_CALL_ARG_WIDE);
					}

					return (EOK);
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] &= ~(_TRACE_CALL_RET_WIDE);
					}

					return (EOK);
				}
				case _NTO_TRACE_SYSTEM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
						trace_masks.system_mask[i] &= ~_TRACE_SYSTEM_ARG_WIDE;
					}

					return (EOK);
				}
				case _NTO_TRACE_COMM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
						trace_masks.comm_mask[i] &= ~_TRACE_COMM_ARG_WIDE;
					}

					return (EOK);
				}
				case _NTO_TRACE_THREAD:
				{
					trace_masks.th_mask &= ~_TRACE_THREAD_ARG_WIDE;
					return (EOK);
				}
				default:
				{
					return (check_class(class));
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETCLASSWIDE): // Setting wide one class
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] |= (_TRACE_CALL_ARG_WIDE|_TRACE_CALL_RET_WIDE);
					}

					return (EOK);
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] |= _TRACE_CALL_ARG_WIDE;
					}

					return (EOK);
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					unsigned i;

					for(i=0;i<ker_call_entry_num;++i) {
						trace_masks.ker_call_masks[i] |= _TRACE_CALL_RET_WIDE;
					}

					return (EOK);
				}
				case _NTO_TRACE_SYSTEM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_SYSTEM_NUM;++i) {
						trace_masks.system_mask[i] |= _TRACE_SYSTEM_ARG_WIDE;
					}

					return (EOK);
				}
				case _NTO_TRACE_COMM:
				{
					unsigned i;

					for(i=0;i<_TRACE_MAX_COMM_NUM;++i) {
						trace_masks.comm_mask[i] |= _TRACE_COMM_ARG_WIDE;
					}

					return (EOK);
				}
				case _NTO_TRACE_THREAD:
				{
					trace_masks.th_mask |= _TRACE_THREAD_ARG_WIDE;
					return (EOK);
				}
				default:
				{
					return (check_class(class));
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_ADDEVENT): // Add one event
		{
			switch(*(++c_p)) {
				case _NTO_TRACE_EMPTY:
				{
					return (EOK);
				}
				case _NTO_TRACE_USER:
				{
					uint32_t code = *(++c_p);
					if ( code > _NTO_TRACE_USERLAST ) {
						return EINVAL;
					}
					trace_masks.user_mask[code] |= (_TRACE_ENTER_USER);

					return (EOK);
				}
				case _NTO_TRACE_CONTROL:
				{
					trace_masks.control_mask |= *(++c_p);

					return (EOK);
				}
				case _NTO_TRACE_KERCALL:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] |= (_TRACE_ENTER_CALL|_TRACE_EXIT_CALL);
					} else {
						return (ENOTSUP);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] |= _TRACE_ENTER_CALL;
					} else {
						return (ENOTSUP);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] |= _TRACE_EXIT_CALL;
					} else {
						return (ENOTSUP);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_INT:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						int_enter_enable_mask = (0xffffffff);
						int_exit_enable_mask  = (0xffffffff);
						trace_masks.int_masks[l] |= (_TRACE_ENTER_INT|_TRACE_EXIT_INT);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INTENTER:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						int_enter_enable_mask = (0xffffffff);
						trace_masks.int_masks[l] |= _TRACE_ENTER_INT;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INTEXIT:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						int_exit_enable_mask = (0xffffffff);
						trace_masks.int_masks[l] |= _TRACE_EXIT_INT;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INT_HANDLER_ENTER:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						int_enter_enable_mask = (0xffffffff);
						trace_masks.int_handler_masks[l] |= _TRACE_ENTER_INT;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INT_HANDLER_EXIT:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						int_exit_enable_mask = (0xffffffff);
						trace_masks.int_handler_masks[l] |= _TRACE_EXIT_INT;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_PROCESS:
				{
					trace_masks.pr_mask |= *(++c_p);

					return (EOK);
				}
				case _NTO_TRACE_THREAD:
				{
					trace_masks.th_mask |= *(++c_p);

					return (EOK);
				}
				case _NTO_TRACE_VTHREAD:
				{
					trace_masks.vth_mask |= *(++c_p);

					return (EOK);
				}
				case _NTO_TRACE_SYSTEM:
				{
					if((*(++c_p))<_TRACE_MAX_SYSTEM_NUM) {
						trace_masks.system_mask[*c_p] |= _TRACE_ENTER_SYSTEM;
					} else {
						return (ENOTSUP);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_COMM:
				{
					if((*(++c_p))<_TRACE_MAX_COMM_NUM) {
						trace_masks.comm_mask[*c_p] |= _TRACE_ENTER_COMM;
					} else {
						return (ENOTSUP);
					}

					return (check_call_masks());
				}
				default:
				{
					return (ENOTSUP);
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_DELEVENT): // Deleting one event
		{
			switch(*(++c_p)) {
				case _NTO_TRACE_EMPTY:
				{
					return (EOK);
				}
				case _NTO_TRACE_USER:
				{
					uint32_t code = *(++c_p);
					if ( code > _NTO_TRACE_USERLAST ) {
						return EINVAL;
					}
					trace_masks.user_mask[code] &= ~(_TRACE_ENTER_USER);

					return (EOK);
				}
				case _NTO_TRACE_CONTROL:
				{
					trace_masks.control_mask &= ~(*(++c_p));

					return (EOK);
				}
				case _NTO_TRACE_KERCALL:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] &= ~(_TRACE_ENTER_CALL|_TRACE_EXIT_CALL);
					} else {
						return (ENOTSUP);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] &= ~(_TRACE_ENTER_CALL);
					} else {
						return (ENOTSUP);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] &= ~(_TRACE_EXIT_CALL);
					} else {
						return (ENOTSUP);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_INT:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						unsigned i,j;

						trace_masks.int_masks[l] &= ~(_TRACE_ENTER_INT|_TRACE_EXIT_INT);
						for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
							if(_TRACE_ENTER_INT&trace_masks.int_masks[i]) {
								break;
							}
						}
						for(j=0;j<_TRACE_MAX_INT_NUM;++j) {
							if(_TRACE_ENTER_INT&trace_masks.int_handler_masks[j]) {
								break;
							}
						}
						if(i==_TRACE_MAX_INT_NUM && j==_TRACE_MAX_INT_NUM) {
							int_enter_enable_mask = (0x00000000);
						}
						for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
							if(_TRACE_EXIT_INT&trace_masks.int_masks[i]) {
								break;
							}
						}
						for(j=0;j<_TRACE_MAX_INT_NUM;++j) {
							if(_TRACE_EXIT_INT&trace_masks.int_handler_masks[j]) {
								break;
							}
						}
						if(i==_TRACE_MAX_INT_NUM && j==_TRACE_MAX_INT_NUM) {
							int_exit_enable_mask = (0x00000000);
						}

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INTENTER:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						unsigned i;

						trace_masks.int_masks[l] &= ~(_TRACE_ENTER_INT);
						for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
							if(_TRACE_ENTER_INT&trace_masks.int_masks[i]) {
								return (EOK);
							}
						}
						for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
							if(_TRACE_ENTER_INT&trace_masks.int_handler_masks[i]) {
								return (EOK);
							}
						}
						int_enter_enable_mask = (0x00000000);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INTEXIT:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						unsigned i;

						trace_masks.int_masks[l] &= ~(_TRACE_EXIT_INT);
						for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
							if(_TRACE_EXIT_INT&trace_masks.int_masks[i]) {
								return (EOK);
							}
						}
						for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
							if(_TRACE_EXIT_INT&trace_masks.int_handler_masks[i]) {
								return (EOK);
							}
						}
						int_exit_enable_mask = (0x00000000);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INT_HANDLER_ENTER:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						unsigned i;

						trace_masks.int_handler_masks[l] &= ~(_TRACE_ENTER_INT);
						for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
							if(_TRACE_ENTER_INT&trace_masks.int_handler_masks[i]) {
								return (EOK);
							}
						}
						for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
							if(_TRACE_ENTER_INT&trace_masks.int_masks[i]) {
								return (EOK);
							}
						}
						int_enter_enable_mask = (0x00000000);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INT_HANDLER_EXIT:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						unsigned i;

						trace_masks.int_handler_masks[l] &= ~(_TRACE_EXIT_INT);
						for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
							if(_TRACE_EXIT_INT&trace_masks.int_handler_masks[i]) {
								return (EOK);
							}
						}
						for(i=0;i<_TRACE_MAX_INT_NUM;++i) {
							if(_TRACE_EXIT_INT&trace_masks.int_masks[i]) {
								return (EOK);
							}
						}
						int_exit_enable_mask = (0x00000000);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_PROCESS:
				{
					trace_masks.pr_mask &= ~(*(++c_p));

					return (EOK);
				}
				case _NTO_TRACE_THREAD:
				{
					trace_masks.th_mask &= ~(*(++c_p));

					return (EOK);
				}
				case _NTO_TRACE_VTHREAD:
				{
					trace_masks.vth_mask &= ~(*(++c_p));

					return (EOK);
				}
				case _NTO_TRACE_SYSTEM:
				{
					if((*(++c_p))<_TRACE_MAX_SYSTEM_NUM) {
						trace_masks.system_mask[*c_p] &= ~(_TRACE_ENTER_SYSTEM);
					} else {
						return (ENOTSUP);
					}

					return (check_call_masks());
				}
				case _NTO_TRACE_COMM:
				{
					if((*(++c_p))<_TRACE_MAX_COMM_NUM) {
						trace_masks.comm_mask[*c_p] &= ~(_TRACE_ENTER_COMM);
					} else {
						return (ENOTSUP);
					}

					return (check_call_masks());
				}
				default:
				{
					return (ENOTSUP);
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETEVENTFAST): // Setting one event fast
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] &= ~(_TRACE_CALL_ARG_WIDE|_TRACE_CALL_RET_WIDE);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] &= ~(_TRACE_CALL_ARG_WIDE);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] &= ~(_TRACE_CALL_RET_WIDE);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_COMM:
				{
					if((*(++c_p))<_TRACE_MAX_COMM_NUM) {
						trace_masks.comm_mask[*c_p] &= ~_TRACE_COMM_ARG_WIDE;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_SYSTEM:
				{
					if((*(++c_p))<_TRACE_MAX_SYSTEM_NUM) {
						trace_masks.system_mask[*c_p] &= ~_TRACE_SYSTEM_ARG_WIDE;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				default:
				{
					return (check_class(class));
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETEVENTWIDE): // Setting one event wide
		{
			unsigned class=*(++c_p);

			switch(class) {
				case _NTO_TRACE_KERCALL:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] |= (_TRACE_CALL_ARG_WIDE|_TRACE_CALL_RET_WIDE);

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] |= _TRACE_CALL_ARG_WIDE;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					if((*(++c_p))<ker_call_entry_num) {
						trace_masks.ker_call_masks[*c_p] |= _TRACE_CALL_RET_WIDE;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_COMM:
				{
					if((*(++c_p))<_TRACE_MAX_COMM_NUM) {
						trace_masks.comm_mask[*c_p] |= _TRACE_COMM_ARG_WIDE;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_SYSTEM:
				{
					if((*(++c_p))<_TRACE_MAX_SYSTEM_NUM) {
						trace_masks.system_mask[*c_p] |= _TRACE_SYSTEM_ARG_WIDE;

						return (EOK);
					} else {
						return (ENOTSUP);
					}
				}
				default:
				{
					return (check_class(class));
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_ADDEVENTHANDLER): // Adding one event handler
		{
			if(!kerisroot(act)||!(act->flags&_NTO_TF_IOPRIV)) return (EPERM);
			if(trace_masks.eh_num>=_TRACE_MAX_EV_HANDLER_NUM) {
				return (ENOTSUP);
			}
			switch(*(++c_p)) {
				case _NTO_TRACE_EMPTY:
				{
					return (EOK);
				}
				case _NTO_TRACE_CONTROL:
				{
					switch(*(++c_p)) {
						case _NTO_TRACE_CONTROLTIME:
						{
							return (_TRACE_ADD_EH(&trace_masks.control_time_ehd_p));
						}
						default:
						{
							return (ENOTSUP);
						}
					}
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					if((*(++c_p))<ker_call_entry_num) {
						return (_TRACE_ADD_EH(trace_masks.ker_call_enter_ehd_p + *c_p));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					if((*(++c_p))<ker_call_entry_num) {
						return (_TRACE_ADD_EH(trace_masks.ker_call_exit_ehd_p + *c_p));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INTENTER:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						return (_TRACE_ADD_EH(trace_masks.int_enter_ehd_p + l));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INTEXIT:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						return (_TRACE_ADD_EH(trace_masks.int_exit_ehd_p + l));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_PROCESS:
				{
					switch(*(++c_p)) {
						case _NTO_TRACE_PROCCREATE:
						{
							return (_TRACE_ADD_EH(&trace_masks.pr_create_ehd_p));
						}
						case _NTO_TRACE_PROCDESTROY:
						{
							return (_TRACE_ADD_EH(&trace_masks.pr_destroy_ehd_p));
						}
						case _NTO_TRACE_PROCCREATE_NAME:
						{
							return (_TRACE_ADD_EH(&trace_masks.pr_create_name_ehd_p));
						}
						case _NTO_TRACE_PROCDESTROY_NAME:
						{
							return (_TRACE_ADD_EH(&trace_masks.pr_destroy_name_ehd_p));
						}
						case _NTO_TRACE_PROCTHREAD_NAME:
						{
							return (_TRACE_ADD_EH(&trace_masks.th_name_ehd_p));
						}
						default:
						{
							return (ENOTSUP);
						}
					}
				}
				case _NTO_TRACE_THREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM) {
						return (_TRACE_ADD_EH(trace_masks.thread_ehd_p + i_2));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_VTHREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM) {
						return (_TRACE_ADD_EH(trace_masks.vthread_ehd_p + i_2));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_SYSTEM:
				{
					if((*(++c_p))<_TRACE_MAX_SYSTEM_NUM) {
						return (_TRACE_ADD_EH(trace_masks.system_ehd_p + *c_p));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_COMM:
				{
					if((*(++c_p))<_TRACE_MAX_COMM_NUM) {
						return (_TRACE_ADD_EH(trace_masks.comm_ehd_p + *c_p));
					} else {
						return (ENOTSUP);
					}
				}
				default:
				{
					return (ENOTSUP);
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_DELEVENTHANDLER):  // Deleting one event handler
		{
			switch(*(++c_p)) {
				case _NTO_TRACE_EMPTY:
				{
					return (EOK);
				}
				case _NTO_TRACE_CONTROL:
				{
					switch(*(++c_p)) {
						case _NTO_TRACE_CONTROLTIME:
						{
							return (delete_eh(&trace_masks.control_time_ehd_p));
						}
						default:
						{
							return (ENOTSUP);
						}
					}
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					if((*(++c_p))<ker_call_entry_num) {
						return (delete_eh(trace_masks.ker_call_enter_ehd_p + *c_p));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					if((*(++c_p))<ker_call_entry_num) {
						return (delete_eh(trace_masks.ker_call_exit_ehd_p + *c_p));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INTENTER:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						return (delete_eh(trace_masks.int_enter_ehd_p + l));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_INTEXIT:
				{
					int l=get_int_level(*(++c_p));

					if(l>(-1)&&l<_TRACE_MAX_INT_NUM) {
						return (delete_eh(trace_masks.int_exit_ehd_p + l));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_PROCESS:
				{
					switch(*(++c_p)) {
						case _NTO_TRACE_PROCCREATE:
						{
							return (delete_eh(&trace_masks.pr_create_ehd_p));
						}
						case _NTO_TRACE_PROCDESTROY:
						{
							return (delete_eh(&trace_masks.pr_destroy_ehd_p));
						}
						case _NTO_TRACE_PROCCREATE_NAME:
						{
							return (delete_eh(&trace_masks.pr_create_name_ehd_p));
						}
						case _NTO_TRACE_PROCDESTROY_NAME:
						{
							return (delete_eh(&trace_masks.pr_destroy_name_ehd_p));
						}
						case _NTO_TRACE_PROCTHREAD_NAME:
						{
							return (delete_eh(&trace_masks.th_name_ehd_p));
						}
						default:
						{
							return (ENOTSUP);
						}
					}
				}
				case _NTO_TRACE_THREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM) {
						return (delete_eh(trace_masks.thread_ehd_p + i_2));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_VTHREAD:
				{
					unsigned i_1 = *(++c_p);
					unsigned i_2 = 0U;

					for(i_1=i_1>>1;i_1;i_1=i_1>>1,++i_2) {
						// nothing to do
					}
					if(i_2<_TRACE_MAX_TH_STATE_NUM) {
						return (delete_eh(trace_masks.vthread_ehd_p + i_2));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_SYSTEM:
				{
					if((*(++c_p))<_TRACE_MAX_SYSTEM_NUM) {
						return (delete_eh(trace_masks.system_ehd_p + *c_p));
					} else {
						return (ENOTSUP);
					}
				}
				case _NTO_TRACE_COMM:
				{
					if((*(++c_p))<_TRACE_MAX_COMM_NUM) {
						return (delete_eh(trace_masks.comm_ehd_p + *c_p));
					} else {
						return (ENOTSUP);
					}
				}
				default:
				{
					return (ENOTSUP);
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_ADDCLASSEVHANDLER): // Adding one event handler
		{
			if(!kerisroot(act)||!(act->flags&_NTO_TF_IOPRIV)) return (EPERM);
			if(trace_masks.eh_num>=_TRACE_MAX_EV_HANDLER_NUM) {
				return (ENOTSUP);
			}
			switch(*(++c_p)) {
				case _NTO_TRACE_EMPTY:
				{
					return (EOK);
				}
				case _NTO_TRACE_CONTROL:
				{
					return (_TRACE_ADD_EH(&trace_masks.class_control_ehd_p));
				}
				case _NTO_TRACE_KERCALL:
				{
					return (_TRACE_DOUBLE_ADD_EH
					        (
					         &trace_masks.class_ker_call_enter_ehd_p,
					         &trace_masks.class_ker_call_exit_ehd_p
					        ));
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					return (_TRACE_ADD_EH(&trace_masks.class_ker_call_enter_ehd_p));
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					return (_TRACE_ADD_EH(&trace_masks.class_ker_call_exit_ehd_p));
				}
				case _NTO_TRACE_INT:
				{
					return (_TRACE_DOUBLE_ADD_EH
					        (
					         &trace_masks.class_int_enter_ehd_p,
					         &trace_masks.class_int_exit_ehd_p
					        ));
				}
				case _NTO_TRACE_INTENTER:
				{
					return (_TRACE_ADD_EH(&trace_masks.class_int_enter_ehd_p));
				}
				case _NTO_TRACE_INTEXIT:
				{
					return (_TRACE_ADD_EH(&trace_masks.class_int_exit_ehd_p));
				}
				case _NTO_TRACE_PROCESS:
				{
					return (_TRACE_ADD_EH(&trace_masks.class_process_ehd_p));
				}
				case _NTO_TRACE_THREAD:
				{
					return (_TRACE_ADD_EH(&trace_masks.class_thread_ehd_p));
				}
				case _NTO_TRACE_VTHREAD:
				{
					return (_TRACE_ADD_EH(&trace_masks.class_vthread_ehd_p));
				}
				case _NTO_TRACE_SYSTEM:
				{
					return (_TRACE_ADD_EH(&trace_masks.class_system_ehd_p));
				}
				case _NTO_TRACE_COMM:
				{
					return (_TRACE_ADD_EH(&trace_masks.class_comm_ehd_p));
				}
				default:
				{
					return (ENOTSUP);
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_DELCLASSEVHANDLER): // Deleting one event handler
		{
			switch(*(++c_p)) {
				case _NTO_TRACE_EMPTY:
				{
					return (EOK);
				}
				case _NTO_TRACE_CONTROL:
				{
					return (delete_eh(&trace_masks.class_control_ehd_p));
				}
				case _NTO_TRACE_KERCALL:
				{
					(void) delete_eh(&trace_masks.class_ker_call_enter_ehd_p);
					(void) delete_eh(&trace_masks.class_ker_call_exit_ehd_p);

					return (EOK);
				}
				case _NTO_TRACE_KERCALLENTER:
				{
					return (delete_eh(&trace_masks.class_ker_call_enter_ehd_p));
				}
				case _NTO_TRACE_KERCALLEXIT:
				{
					return (delete_eh(&trace_masks.class_ker_call_exit_ehd_p));
				}
				case _NTO_TRACE_INT:
				{
					(void) delete_eh(&trace_masks.class_int_enter_ehd_p);
					(void) delete_eh(&trace_masks.class_int_exit_ehd_p);

					return (EOK);
				}
				case _NTO_TRACE_INTENTER:
				{
					return (delete_eh(&trace_masks.class_int_enter_ehd_p));
				}
				case _NTO_TRACE_INTEXIT:
				{
					return (delete_eh(&trace_masks.class_int_exit_ehd_p));
				}
				case _NTO_TRACE_PROCESS:
				{
					return (delete_eh(&trace_masks.class_process_ehd_p));
				}
				case _NTO_TRACE_THREAD:
				{
					return (delete_eh(&trace_masks.class_thread_ehd_p));
				}
				case _NTO_TRACE_VTHREAD:
				{
					return (delete_eh(&trace_masks.class_vthread_ehd_p));
				}
				case _NTO_TRACE_SYSTEM:
				{
					return (delete_eh(&trace_masks.class_system_ehd_p));
				}
				case _NTO_TRACE_COMM:
				{
					return (delete_eh(&trace_masks.class_comm_ehd_p));
				}
				default:
				{
					return (ENOTSUP);
				}
			}
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETRINGMODE): // Setting ring mode of the kernel
		{
			tracebuf_t* l_b_p  =privateptr->tracebuf;
			tracebuf_t* b_p    =l_b_p;

			if(trace_masks.main_flags||l_b_p==NULL) return (ECANCELED);
			trace_masks.ring_mode = 1;
			do {
				InterruptLock(&b_p->h.spin);
				b_p->h.tail_ptr = b_p->h.begin_ptr;
				b_p->h.num_events = 0;
				b_p->h.flags      = _TRACE_FLAGS_RING;
				InterruptUnlock(&b_p->h.spin);
				b_p               = b_p->h.next;
			} while(l_b_p!=b_p);

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SETLINEARMODE): // Setting linear mode of the kernel
		{
			tracebuf_t* l_b_p=privateptr->tracebuf;
			tracebuf_t* b_p  =l_b_p;

			if(trace_masks.main_flags||l_b_p==NULL) return (ECANCELED);
			trace_masks.ring_mode = 0;
			do {
				InterruptLock(&b_p->h.spin);
				b_p->h.tail_ptr = b_p->h.begin_ptr;
				b_p->h.num_events = 0;
				b_p->h.flags      = 0;
				InterruptUnlock(&b_p->h.spin);
				b_p               = b_p->h.next;
			} while(l_b_p!=b_p);

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTSCLASSEVENT): // Insert user simple class event
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTCCLASSEVENT): // Insert user combine class event
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTSUSEREVENT): // Insert user simple class event
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTCUSEREVENT): // Insert user combine class event
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTUSRSTREVENT):// Insert user string class event
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTEVENT):      // Insert "row" user trace event
		case _TRACE_GET_FLAG(_NTO_TRACE_STARTNOSTATE):     // Begin kernel tracing, no initial state
		case _TRACE_GET_FLAG(_NTO_TRACE_START):            // Begin kernel tracing
		case _TRACE_GET_FLAG(_NTO_TRACE_STOP):             // Stop kernel tracing
		{
			return (trace_event(c_p));
		}
case _TRACE_GET_FLAG(_NTO_TRACE_ALLOCBUFFER): // Allocating kernel trace buffer
		{
			unsigned    b_n = *++c_p;
			unsigned    i;
			int			status;
			tracebuf_t* ptbuf;
			part_id_t mpid;

			if(!kerisroot(act)) return (EPERM);
			**((void ***) ++c_p) = NULL;
			InterruptLock(&trace_masks.state_spin);
			if(trace_masks.buff_0_ptr) {
				InterruptUnlock(&trace_masks.state_spin);
				return (EEXIST);                  // buffer exist
			}
			InterruptUnlock(&trace_masks.state_spin);
			/* We specifically use MAP_PRIVATE here, since it's not
			 * really a shared object (we pass the physical address back to
			 * the tracelogger, which then maps the memory directly, and
			 * on ARM, it creates a kernel vaddr mapping which we use directly
			 * in tracelogger.  In that case, passing MAP_SHARED would force the
			 * mapping to be PROT_NOCACHE, which just kills the performance in
			 * copying the trace events out of the kernel buffer.
			 */
			status = memmgr.mmap(
				NULL, /* prp */
				0, /* address */
				b_n*(_TRACEBUFSIZE),
				PROT_WRITE|PROT_READ,
				MAP_PHYS|MAP_PRIVATE|MAP_ANON,
				NULL, /* object */
				0, /* offset */
				0, /* align */
				0, /* prealloc */
				NOFD,
				(void *)&ptbuf,
				&i,
				mpid = mempart_getid(NULL, sys_memclass_id)		// FIX ME - this ok?
			);
			InterruptLock(&trace_masks.state_spin);
			if(trace_masks.buff_0_ptr) {
				InterruptUnlock(&trace_masks.state_spin);
				(void) memmgr.munmap(NULL, (uintptr_t)ptbuf, (trace_masks.buff_num*(_TRACEBUFSIZE)), 0, mpid);
				return (EEXIST);                  // buffer exist
			}
			if (status != EOK || MAP_FAILED == (void *)ptbuf) {
				InterruptUnlock(&trace_masks.state_spin);

				return (ENOMEM);
			}
			trace_masks.buff_0_ptr = (void*)ptbuf;
			trace_masks.buff_num   = b_n;
			trace_masks.max_events = (_TRACELEMENTS - _TRACEBUFEMPTY);
			InterruptUnlock(&trace_masks.state_spin);
			privateptr->tracebuf   = ptbuf;
			for (i=0;i<b_n;++i,++ptbuf) {            // setup header information
				ptbuf->h.flags       = (uint32_t)0;
				ptbuf->h.num_events  = (uint32_t)0;
				ptbuf->h.ls13_rb_num = i<<13;
				ptbuf->h.begin_ptr   = (traceevent_t*) &ptbuf->data[0];
				ptbuf->h.tail_ptr    = ptbuf->h.begin_ptr;
				ptbuf->h.baseaddr    = privateptr->tracebuf;
				ptbuf->h.next        = ptbuf+1;
				memset(&ptbuf->h.spin, 0, sizeof(ptbuf->h.spin));
			}
			(--ptbuf)->h.next = privateptr->tracebuf;

#if defined(__ARM__)
			**((_Paddr32t**) c_p) = (_Paddr32t) privateptr->tracebuf;
#else
			{
				paddr_t	paddr;

				(void) memmgr.vaddrinfo(aspaces_prp[KERNCPU], (uintptr_t)privateptr->tracebuf, &paddr, NULL, VI_NORMAL);
				**((_Paddr32t**) c_p) = paddr;
			}
#endif

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_DEALLOCBUFFER): // Deallocating trace buffer
		{
			if(privateptr->tracebuf) {
				void*     b_0_p;

				if(!kerisroot(act)) return (EPERM);
				trace_masks.main_flags = (uint32_t)0;
				InterruptLock(&trace_masks.state_spin);
				if((b_0_p=trace_masks.buff_0_ptr)==NULL) {
					trace_masks.buff_num = (uint32_t)0;
					InterruptUnlock(&trace_masks.state_spin);
					return (EOK);
				}
				trace_masks.buff_0_ptr = NULL;
				privateptr->tracebuf   = NULL;
				InterruptUnlock(&trace_masks.state_spin);

				/////////////////////////////////////////////////////
				////////////////////  FIX_ME!!  /////////////////////
				// There is a danger that deallocated buffers might
				// be in use.
				/////////////////////////////////////////////////////

				(void) memmgr.munmap(NULL, (uintptr_t)b_0_p, (trace_masks.buff_num*(_TRACEBUFSIZE)),
										0, mempart_getid(NULL, sys_memclass_id));
				trace_masks.buff_num = (uint32_t)0;
			}

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_FLUSHBUFFER): // Flushing the buffer
		{
			int ret = trace_flushbuffer();
			(void) check_call_masks();
			return ret;
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_QUERYEVENTS): // Getting event number
		{
			tracebuf_t* tracebuf=privateptr->tracebuf;

			if(tracebuf) {
				SETKSTATUS(act, tracebuf->h.num_events);

				return (ENOERROR);
			} else {
				return (EFAULT);
			}

		}
		case _TRACE_GET_FLAG(_NTO_TRACE_SKIPBUFFER):  // Skip first buffers when
		{                                             // ring overflow
			trace_masks.skip_buff = 1U;

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_OVERWRITEBUFFER):// Overwrite first buffer
		{                                                // when ring overflows
			trace_masks.skip_buff = 0U;

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_QUERYVERSION):// Query version
		{
			SETKSTATUS(act, _NTO_VERSION);

			return (ENOERROR);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_QUERYSUPPORT):// Query support
		{
			SETKSTATUS(act, _NTO_TRACE_SUPPINSTR);

			return (ENOERROR);
		}
		default:
		{
			return (ENOTSUP);
		}
	}

	/* NOTREACHED */
	return (ENOTSUP);
}

#else

int kdecl
ker_trace_event(THREAD *act,struct kerargs_trace_event *kap) {

	RD_PROBE_INT(act, kap->data, sizeof(*kap->data)/sizeof(int));

	lock_kernel();
	switch(_TRACE_GET_FLAG(*(unsigned*)kap->data)) {
		case _TRACE_GET_FLAG(_NTO_TRACE_QUERYVERSION):
		{
			SETKSTATUS(act, _NTO_VERSION);

			return (ENOERROR);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_QUERYSUPPORT):
		{
			SETKSTATUS(act, _NTO_TRACE_NOINSTRSUPP);

			return (ENOERROR);
		}
		default:
		{
			return trace_event(kap->data);
		}
	}
}

#endif

__SRCVERSION("ker_trace.c $Rev: 206799 $");
