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

#ifndef __PROCFS_H_INCLUDED
#define __PROCFS_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef _DCMD_PROC_H_INCLUDED
#include _NTO_HDR_(sys/dcmd_proc.h)
#endif

#ifndef _SIGNAL_H_INCLUDED
#include _NTO_HDR_(signal.h)
#endif

#ifndef __SYSPAGE_H_INCLUDED
 #include _NTO_HDR_(sys/syspage.h)
#endif

#ifndef __DEBUG_H_INCLUDED
 #include _NTO_HDR_(sys/debug.h)
#endif


__BEGIN_DECLS

#include _NTO_HDR_(_pack64.h)

typedef struct syspage_entry	procfs_sysinfo;
typedef debug_process_t			procfs_info;
typedef debug_thread_t			procfs_status;
typedef debug_run_t				procfs_run;
typedef debug_break_t			procfs_break;
typedef debug_greg_t			procfs_greg;
typedef debug_fpreg_t			procfs_fpreg;
typedef debug_altreg_t			procfs_altreg;
typedef debug_irq_t				procfs_irq;
typedef debug_timer_t			procfs_timer;
typedef debug_channel_t			procfs_channel;

typedef struct _procfs_signal {
	pthread_t					tid;
	_Int32t						signo;
	_Int32t						code;
	_Intptrt					value;
}							procfs_signal;

typedef struct _procfs_map_info {
	_Uint64t					vaddr;
	_Uint64t					size;
	_Uint32t					flags;
	dev_t						dev;
#if _FILE_OFFSET_BITS - 0 == 64
	off_t						offset;
	ino_t						ino;
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
	off64_t						offset;
	ino64_t						ino;
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif
	_Int64t						reserved;
}							procfs_mapinfo;

typedef struct _procfs_debug_info {
	_Uint64t					vaddr;
	char						path[1];
}							procfs_debuginfo;

enum {
	REGSET_GPREGS = 0,
	REGSET_FPREGS,
	REGSET_ALTREGS,
	REGSET_PERFREGS,
};

typedef struct _procfs_regset {
	_Uint32t					id;
	char						buf[8192];
}							procfs_regset;

typedef struct _procfs_threadctl {
	_Uint32t					tid;
	_Int32t						cmd;
	char						data[1024];
}							procfs_threadctl;

/* This call is made to obtain information stored in the system page.
   To get the whole syspage, two calls would have to be made. The 
   first gets the "total_size" entry, the second should be for this size.
   Args: A procfs_sysinfo structure is passed as an argument, and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_SYSINFO		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 0, procfs_sysinfo)

/* This call is made to obtain information about a specific process
   associated with the given file descriptor.
   Args: A procfs_info structure is passed as an argument, and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_INFO			__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 1, procfs_info)

/* This call is made to obtain segment specific information about mapped
   memory segments in the specific process associated with 
   the given file descriptor. This call matches corresponding "mmap()" calls.
   Individual page data is not returned (i.e. the PG_* flags in mman.h are 
   not returned). If you need the page attributes use DCMD_PROC_PAGEDATA 
   instead.
   Args: A procfs_mapinfo structure is passed as an argument, and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_MAPINFO		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 2, procfs_mapinfo)

/* It returns the best guess to the elf object on 
   the host machine. This is used by debuggers to find the object that 
   contains the symbol information even though it may have been stripped 
   on the target machine. This call is only useful on MAP_ELF mappings. 
   If any relocation of the ELF object was done, this translation will 
   be undone. This allows you to pass in an address within a elf module, 
   and be returned the address that the original object was linked at 
   so a debugger can find the symbol. (This is an extension from
   the SYSV interface)
   Args: A procfs_debuginfo structure is passed as an argument, and 
   this is filled in with the required information upon return.  
   The procfs_debuginfo can specify the base address of the
   mapped segment that one is interested in.*/
#define DCMD_PROC_MAPDEBUG		__DIOTF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 3, procfs_debuginfo)

/* This call is made to obtain information pertaining to the path
   associated with the process associated with the given file descriptor.
   This is a convenience extension of the above. It is equivalent to using
   DCMD_PROC_INFO, then calling DCMD_PROC_MAPDEBUG with the "base_address"
   field. The base address is the address of the initial executable.
   Args: A procfs_debuginfo structure is passed as an argument, and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_MAPDEBUG_BASE	__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 3, procfs_debuginfo)

/* This call is used to drop a signal on the process that is being
   debugged, and that is specified by the associated file descriptor.
   This is a way for a debugger to artificially generate signals as if they
   came from the system.
   Args: A procfs_signal structure is passed as an argument, 
   and this specifies the signal that will be sent. */
#define DCMD_PROC_SIGNAL		__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 4, procfs_signal)

/* This call stops the process that is being debugged
   and specified by the associated file descriptor. 
   The FD must have been open for write.
   Args: A procfs_status structure is passed as an argument, and 
   this is filled with status information upon return */
#define DCMD_PROC_STOP			__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 5, procfs_status)

/* This call holds off the calling process until the process that is 
   being debugged and specified by the associated file descriptor reaches
   a "point of interest". The "point of interest" is set up by using
   DCMD_PROC_RUN. The FD must have been open for write.
   Args: A procfs_status structure is passed as an argument, and 
   this is filled with status information upon return */
#define DCMD_PROC_WAITSTOP		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 6, procfs_status)

/* This call is made to obtain current status about a for thread specific 
   information in the process associated with the given file descriptor.
   Args: A procfs_status structure is passed as an argument, and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_STATUS		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 7, procfs_status)

/* This call is made to obtain current status about a thread in 
   a specific process associated with the given file descriptor.
   This is a short form of DMCD_PROC_CURTHREAD to set the current thread then
   calling DCMD_PROC_STATUS to get information about that thread, then
   restoring the current thread.
   Args: A procfs_status structure is passed as an argument, with
   the required thread id filled in the 'tid' field and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_TIDSTATUS		__DIOTF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 7, procfs_status)

/* This call is used to specify the current thread in a process
   that is being debugged and which is specified by the associated 
   file descriptor. This is used to specify a current thread for other 
   DCMD_PROC's that target the current thread (like DCMD_PROC_RUN in 
   singlestep, or DCMD_PROC_STATUS)
   Args: A pthread_t value is passed as an argument and this specifies
   the thread that will be made the current thread. */
#define DCMD_PROC_CURTHREAD		__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 8, pthread_t)

/* This call resumes the process that is being debugged and 
   that is associated with the given file descriptor, if it 
   has previously been stopped. The FD must have been open for write.
   It also lets you set the "points of interest" (i.e. signal's or faults you
   want to stop on) and other run flags (like instruction pointer or single
   step)
   Args: A procfs_run structure is passed as an argument, and
   this structure is passed on as control information to the 
   process before it is resumes. */
#define DCMD_PROC_RUN			__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 9, procfs_run)

/* This call returns the information stored in the CPU registers
   based on the current thread for the process associated with the given file descriptor.
   Args: A procfs_greg structure is passed as an argument, and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_GETGREG		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 10, procfs_greg)

/* This call sets the CPU registers with the values provided
   for the process associated with the given file descriptor.
   The FD must have been open for write.
   Args: A procfs_greg structure is passed as an argument, and 
   this is used to the set the values of the CPU registers.  */
#define DCMD_PROC_SETGREG		__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 11, procfs_greg)

/* This call returns the information stored in the Floating Point Data 
   registers for the process associated with the given file descriptor.
   Args: A procfs_fpreg structure is passed as an argument, and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_GETFPREG		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 12, procfs_fpreg)

/* This call sets the Floating Point Data registers with the values 
   provided for the process associated with the specific file descriptor.
   The FD must have been open for write.
   Args: A procfs_fpreg structure is passed as an argument, and 
   this is used to the set the values of the Floating Point Data registers.  */
#define DCMD_PROC_SETFPREG		__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 13, procfs_fpreg)

/* This call sets a break point the process that is being debugged
   and specified by the associated file descriptor. 
   The FD must have been open for write.
   Args: A procfs_break structure is passed as an argument, and 
   this is used to the set the break point */
#define DCMD_PROC_BREAK			__DIOTF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 14, procfs_break)

/* This call freezes a thread in the process that is being debugged
   and specified by the associated file descriptor. 
   The FD must have been open for write.
   Args: A pthread_t value is passed as an argument, and 
   this specifies the thread to be frozen. */
#define DCMD_PROC_FREEZETHREAD	__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 15, pthread_t)

/* This call un-freezes a thread in the process that is being debugged
   and specified by the associated file descriptor. 
   The FD must have been open for write.
   Args: A pthread_t value is passed as an argument, and 
   this specifies the thread to be thawed. */
#define DCMD_PROC_THAWTHREAD	__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 16, pthread_t)

/* This call defines an event that will be delivered when the process
   that is being debugged, and is associated with the given file 
   descriptor reaches a "point of interest". It would be used instead 
   of blocking with DCMD_PROC_WAITSTOP to let you know when it will not 
   block.
   Args: A sigevent structure is passed as an argument, and 
   this event will be delivered at the appropriate time. */
#define DCMD_PROC_EVENT			__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 17, struct sigevent)

/* This call sets specific debug flags with the values 
   provided for the process associated with the specific file descriptor.
   The flags that can be set are described in sys/debug.h
   Args: An unsigned integer is passed as an argument, and 
   this is used to the set the values of the debug flags */
#define DCMD_PROC_SET_FLAG		__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 18, _Uint32t)

/* This call clears specific debug flags with the values 
   provided for the process associated with the specific file descriptor.
   The flags that can be cleared are described in sys/debug.h
   Args: An unsigned integer is passed as an argument, and 
   this is used to the clear the values of the appropriate debug flags */
#define DCMD_PROC_CLEAR_FLAG	__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 19, _Uint32t)

/* See DCMD_PROC_MAPINFO */
#define DCMD_PROC_PAGEDATA		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 20, procfs_mapinfo)

/* This call returns the information stored in the alternate register set
   for the process associated with the specific file descriptor.
   Args: A procfs_fpreg structure is passed as an argument, and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_GETALTREG		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 21, procfs_fpreg)

/* This call sets the alternate register set with the values provided
   for the process associated with the specific file descriptor.
   The FD must be open for write.
   Args: A procfs_fpreg structure is passed as an argument, and 
   this is used to the set the values of the alternate register set.  */
#define DCMD_PROC_SETALTREG		__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 22, procfs_fpreg)

/* This call returns the timers owned by the process associated with 
   the given file descriptor.
   Args: A procfs_timer structure is passed as an argument, and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_TIMERS		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 23, procfs_timer)

/* This call returns the interrupt handlers owned by the process associated with 
   the given file descriptor.
   Args: A procfs_ir structure is passed as an argument, and 
   this is filled in with the required information upon return.  */
#define DCMD_PROC_IRQS		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 24, procfs_irq)

/* Read the given regset */
#define DCMD_PROC_GETREGSET		__DIOTF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 25, procfs_regset)

/* Set the given regset */
#define DCMD_PROC_SETREGSET		__DIOTF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 26, procfs_regset)

/* Perform a ThreadCtl on another process/thread */
#define DCMD_PROC_THREADCTL		__DIOTF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 27, procfs_threadctl)

/* This call returns a list of the active breakpoints for the process specified by the 
   associated file descriptor. The total number of breakpoints returned is provided as
   the extra field.
   Args: A pointer to a buffer for an array of  procfs_break data to be stored to
*/
#define DCMD_PROC_GET_BREAKLIST	__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 28, procfs_break)

/* Return information about the channels owned by the specified process. */
#define DCMD_PROC_CHANNELS		__DIOF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 29, procfs_channel)

/* This call will return the memory partition identifier which the process (specified
 * by the associated file descriptor) is associated with. If the memory partition
 * module is not installed, NULL will be returned for the identifier
 * Args: Pointer to an 'part_id_t'
*/ 
#define DCMD_PROC_GET_MEMPART_LIST	__DIOTF(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 30, part_list_t)

/* These calls will add/delete the memory partition identifier from the list
 * of partitions which the process (specified by the associated file descriptor)
 * is associated with. If the memory partition module is not installed, this
 * command will return ENOSYS.
 * If the partition could not be associated with/disassociated from, an error
 * will be returned
 * Args: Pointer to an 'part_id_t'
*/ 
#define DCMD_PROC_ADD_MEMPARTID	__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 31, part_id_t)
#define DCMD_PROC_DEL_MEMPARTID	__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 32, part_id_t)
#define DCMD_PROC_CHG_MEMPARTID	__DIOT(_DCMD_PROC, __PROC_SUBCMD_PROCFS + 33, part_id_t)

#include _NTO_HDR_(_packpop.h)

__END_DECLS

#endif

/* __SRCVERSION("procfs.h $Rev: 168445 $"); */
