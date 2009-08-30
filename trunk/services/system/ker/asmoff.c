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
#include <stddef.h>
#include <sys/syspage.h>
#include <sys/fault.h>
#include <sys/perfregs.h>
#include <sys/image.h>
#include <mkasmoff.h>


COMMENT("object struct _sighandler_info");
VALUE( SIGSTACK_SIGINFO , offsetof(struct _sighandler_info, siginfo));
VALUE( SIGSTACK_SIGNO   , offsetof(struct _sighandler_info, siginfo.si_signo)); 
VALUE( SIGSTACK_SIGCODE , offsetof(struct _sighandler_info, siginfo.si_code)); 
VALUE( SIGSTACK_SIGVALUE, offsetof(struct _sighandler_info, siginfo.si_value)); 
VALUE( SIGSTACK_HANDLER , offsetof(struct _sighandler_info, handler));
VALUE( SIGSTACK_CONTEXT , offsetof(struct _sighandler_info, context));

COMMENT("object siginfo_t");
VALUE( SIGINFO_SIGNO    , offsetof(siginfo_t, si_signo));
VALUE( SIGINFO_SIGCODE  , offsetof(siginfo_t, si_code));
VALUE( SIGINFO_SIGVALUE , offsetof(siginfo_t, si_value));

COMMENT("object PROCESS");
VALUE( DEBUGGER       , offsetof(PROCESS, debugger) );
VALUE( PLS            , offsetof(PROCESS, pls) );
VALUE( CANSTUB        , offsetof(PROCESS, canstub) );
VALUE( BOUNDRY_ADDR   , offsetof(PROCESS, boundry_addr) );
VALUE( KDEBUG         , offsetof(PROCESS, kdebug) );
VALUE( MEMORY         , offsetof(PROCESS, memory) );
VALUE( PID         	  , offsetof(PROCESS, pid) );
VALUE( PFLAGS      	  , offsetof(PROCESS, flags) );

COMMENT("object THREAD");
VALUE( TFLAGS         , offsetof(THREAD, flags) );
  VALUE( _NTO_TF_SIG_ACTIVE   , _NTO_TF_SIG_ACTIVE );
  VALUE( _NTO_TF_SIGWAITINFO  , _NTO_TF_SIGWAITINFO );
  VALUE( _NTO_TF_NANOSLEEP    , _NTO_TF_NANOSLEEP );
  VALUE( _NTO_TF_SIGSUSPEND   , _NTO_TF_SIGSUSPEND );
  VALUE( _NTO_TF_RCVINFO      , _NTO_TF_RCVINFO );
  VALUE( _NTO_TF_WAAA         , _NTO_TF_WAAA );
  VALUE( _NTO_TF_TO_BE_STOPPED, _NTO_TF_TO_BE_STOPPED );
  VALUE( _NTO_TF_JOIN         , _NTO_TF_JOIN );
  VALUE( _NTO_TF_ACQUIRE_MUTEX, _NTO_TF_ACQUIRE_MUTEX );
  VALUE( _NTO_TF_V86          , _NTO_TF_V86 );
  VALUE( _NTO_TF_SPECRET_MASK , _NTO_TF_SPECRET_MASK );
  VALUE( _NTO_TF_KCALL_ACTIVE , _NTO_TF_KCALL_ACTIVE );
  VALUE( _NTO_TF_KERERR_SET   , _NTO_TF_KERERR_SET );
  VALUE( _NTO_TF_ALIGN_FAULT  , _NTO_TF_ALIGN_FAULT );
  VALUE( _NTO_TF_BUFF_MSG     , _NTO_TF_BUFF_MSG );
  VALUE( _NTO_TF_SHORT_MSG    , _NTO_TF_SHORT_MSG );
  VALUE( _NTO_TF_PULSE        , _NTO_TF_PULSE );
  VALUE( _NTO_TF_IOPRIV       , _NTO_TF_IOPRIV );
VALUE( ITFLAGS        , offsetof(THREAD, internal_flags) );
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
  VALUE( _NTO_ITF_MSG_DELIVERY , _NTO_ITF_MSG_DELIVERY );
#endif
VALUE( ATFLAGS        , offsetof(THREAD, async_flags) );
  VALUE( _NTO_ATF_TIMESLICE   , _NTO_ATF_TIMESLICE );
  VALUE( _NTO_ATF_FPUSAVE_ALLOC, _NTO_ATF_FPUSAVE_ALLOC );
  VALUE( _NTO_ATF_SMP_RESCHED , _NTO_ATF_SMP_RESCHED );
  VALUE( _NTO_ATF_SMP_EXCEPTION, _NTO_ATF_SMP_EXCEPTION );
  VALUE( _NTO_ATF_WAIT_FOR_KER, _NTO_ATF_WAIT_FOR_KER );
  VALUE( _NTO_ATF_REGCTX_ALLOC, _NTO_ATF_REGCTX_ALLOC );
  VALUE( _NTO_ATF_FORCED_KERNEL, _NTO_ATF_FORCED_KERNEL );
  VALUE( _NTO_ATF_WATCHPOINT, _NTO_ATF_WATCHPOINT );
VALUE( PRIORITY       , offsetof(THREAD, priority) );
VALUE( TLS            , offsetof(THREAD, un.lcl.tls) );
VALUE( PROCESS        , offsetof(THREAD, process) );
VALUE( STATE          , offsetof(THREAD, state) );
VALUE( SYSCALL        , offsetof(THREAD, syscall) );
VALUE( STATE_WAITPAGE , STATE_WAITPAGE );
VALUE( ASPACE_PRP     , offsetof(THREAD, aspace_prp) );
VALUE( BLOCKED_ON     , offsetof(THREAD, blocked_on) );
VALUE( RESTART        , offsetof(THREAD, restart) );
VALUE( SIG_BLOCKED    , offsetof(THREAD, sig_blocked) );
VALUE( SIG_PENDING    , offsetof(THREAD, sig_pending) );
VALUE( TIMEOUT_FLAGS  , offsetof(THREAD, timeout_flags) );
VALUE( FPUDATA        , offsetof(THREAD, fpudata) );
VALUE( CPUDATA        , offsetof(THREAD, cpu) );
VALUE( REG_OFF        , offsetof(THREAD, reg) );
VALUE( ARGS           , offsetof(THREAD, args) );
VALUE( ARGS_SMSG      , offsetof(THREAD, args.ms.smsg) );
VALUE( ARGS_RMSG      , offsetof(THREAD, args.ms.rmsg) );
VALUE( ARGS_SPARTS    , offsetof(THREAD, args.ms.sparts) );
VALUE( ARGS_RPARTS    , offsetof(THREAD, args.ms.rparts) );
VALUE( ARGS_MSGLEN    , offsetof(THREAD, args.ms.msglen) );
VALUE( ARGS_SIG_BLOCKED,offsetof(THREAD, args.ss.sig_blocked) );
VALUE( ARGS_SIG_WAIT  , offsetof(THREAD, args.sw.sig_wait) );
VALUE( ARGS_ASYNC_IP  , offsetof(THREAD, args.async.save_ip) );
VALUE( ARGS_ASYNC_TYPE, offsetof(THREAD, args.async.save_type) );
VALUE( ARGS_ASYNC_CODE, offsetof(THREAD, args.async.code) );
VALUE( ARGS_ASYNC_FAULT_TYPE, offsetof(THREAD, args.async.fault_type) );
VALUE( ARGS_ASYNC_FAULT_ADDR, offsetof(THREAD, args.async.fault_addr) );
VALUE( SIZEOF_THREAD  , sizeof(THREAD) );

COMMENT("object PULSE");
VALUE( PULSE_NEXT     , offsetof(PULSE, next) );
VALUE( PULSE_TYPE     , offsetof(PULSE, type) );
VALUE( PULSE_PRIORITY , offsetof(PULSE, priority) );
VALUE( PULSE_COUNT    , offsetof(PULSE, count) );
VALUE( PULSE_CODE     , offsetof(PULSE, code) );
VALUE( PULSE_VALUE    , offsetof(PULSE, value) );
VALUE( PULSE_ID       , offsetof(PULSE, id) );

COMMENT("object INTERRUPT");
VALUE( INTR_NEXT      , offsetof(INTERRUPT, next) );
VALUE( INTR_THREAD    , offsetof(INTERRUPT, thread) );
VALUE( INTR_LEVEL     , offsetof(INTERRUPT, level) );
VALUE( INTR_HANDLER   , offsetof(INTERRUPT, handler) );
VALUE( INTR_AREA      , offsetof(INTERRUPT, area) );
VALUE( INTR_ID        , offsetof(INTERRUPT, id) );
VALUE( INTR_MASK_COUNT, offsetof(INTERRUPT, mask_count) );
VALUE( INTR_CPU       , offsetof(INTERRUPT, cpu) );

COMMENT("object INTRLEVEL");
VALUE( INTRLEVEL_QUEUE,	offsetof(INTRLEVEL, queue) );
VALUE( INTRLEVEL_INFO,	offsetof(INTRLEVEL, info) );
VALUE( INTRLEVEL_LBASE,	offsetof(INTRLEVEL, level_base) );
VALUE( INTRLEVEL_MCOUNT,offsetof(INTRLEVEL, mask_count) );
VALUE( SIZEOF_INTRLEVEL,sizeof(INTRLEVEL) );
VALUE( LOG2_SIZEOF_INTRLEVEL,	LOG2_SIZEOF_INTRLEVEL );

COMMENT("object IOV");
VALUE( IOV_ADDR       , offsetof(IOV, iov_base) );
VALUE( IOV_LEN        , offsetof(IOV, iov_len) );
VALUE( SIZEOF_IOV     , sizeof(IOV) );

COMMENT("object memmgr_entry");
VALUE( SIZEOF_MEMMGR_ENTRY, sizeof(struct memmgr_entry) );
VALUE( MEMMGR_ASPACE  , offsetof(struct memmgr_entry, aspace) );
VALUE( MEMMGR_FAULT   , offsetof(struct memmgr_entry, fault) );

COMMENT("object thread_local_storage");
VALUE( TLS_STACKADDR  , offsetof(struct _thread_local_storage, __stackaddr) );
VALUE( TLS_FPUEMU_DATA , offsetof(struct _thread_local_storage, __fpuemu_data) );

COMMENT("object thread_local_storage");
VALUE( PLS_MATHEMULATOR  , offsetof(struct _process_local_storage, __mathemulator) );

COMMENT("object fault_info");
VALUE(SIZEOF_FAULT_INFO,	sizeof(struct fault_info));
VALUE(FI_PRP,				offsetof(struct fault_info, prp));
VALUE(FI_VADDR,				offsetof(struct fault_info, vaddr));
VALUE(FI_SIGCODE,			offsetof(struct fault_info, sigcode));

COMMENT("misc manifests");
VALUE( IPI_RESCHED,			IPI_RESCHED );
VALUE( IPI_TIMESLICE,		IPI_TIMESLICE );
VALUE( IPI_TLB_FLUSH,		IPI_TLB_FLUSH );
VALUE( IPI_TLB_SAFE,		IPI_TLB_SAFE );
VALUE( IPI_CONTEXT_SAVE,	IPI_CONTEXT_SAVE );
VALUE( IPI_CHECK_INTR,      IPI_CHECK_INTR );
VALUE( IPI_INTR_MASK,	    IPI_INTR_MASK );
VALUE( IPI_INTR_UNMASK,	    IPI_INTR_UNMASK );
VALUE( IPI_CLOCK_LOAD,	    IPI_CLOCK_LOAD );
VALUE( IPI_PARKIT,		    IPI_PARKIT );
VALUE( INTR_FLAG_SMP_BROADCAST_MASK, INTR_FLAG_SMP_BROADCAST_MASK );
VALUE( INTR_FLAG_SMP_BROADCAST_UNMASK, INTR_FLAG_SMP_BROADCAST_UNMASK );
VALUE( PROCESSORS_MAX,	PROCESSORS_MAX );
VALUE( USER_CPUPAGE_PTR, offsetof(struct system_private_entry,user_cpupageptr));
VALUE( PULSE_SIZE	  , sizeof(struct _pulse) );
VALUE( SIZEOF_REG      		, sizeof(CPU_REGISTERS) );
VALUE( KER_ENTRY_SIZE,		KER_ENTRY_SIZE );
VALUE( KERERR_SKIPAHEAD,	KERERR_SKIPAHEAD );
VALUE( ENOSYS,			ENOSYS );
VALUE( STARTUP_STACK_SIZE	, sizeof( startup_stack ) );
VALUE( STACK_INITIAL_CALL_CONVENTION_USAGE,  STACK_INITIAL_CALL_CONVENTION_USAGE	);
VALUE( TIMER_LOAD,		offsetof( struct qtime_entry, timer_load ) );
VALUE( XFER_SRC_FAULT     , XFER_SRC_FAULT );
VALUE( XFER_DST_FAULT     , XFER_DST_FAULT );
VALUE( XFER_SRC_CHECK     , XFER_SRC_CHECK );
VALUE( XFER_DST_CHECK     , XFER_DST_CHECK );
VALUE( ERRNO_EFAULT       , EFAULT );
VALUE( SIGCODE_USER,		SIGCODE_USER );
VALUE( SIGCODE_INTR,		SIGCODE_INTR );
VALUE( SIGCODE_KERNEL,		SIGCODE_KERNEL );
VALUE( SIGCODE_INXFER,		SIGCODE_INXFER );
VALUE( SIGCODE_KEREXIT,		SIGCODE_KEREXIT );
VALUE( SIGCODE_FATAL,		SIGCODE_FATAL );
VALUE( SIGCODE_STORE,		SIGCODE_STORE );
VALUE( SIGCODE_SSTEP,		SIGCODE_SSTEP );
VALUE( SIGCODE_BDSLOT, 		SIGCODE_BDSLOT );
VALUE( AP_INTREVENT_FROM_IO_FLAG, AP_INTREVENT_FROM_IO_FLAG ); 
VALUE( SIGSEGV            , SIGSEGV );
  VALUE( SEGV_MAPERR      , SEGV_MAPERR );
  VALUE( SEGV_ACCERR      , SEGV_ACCERR );
  VALUE( SEGV_STKERR      , SEGV_STKERR );
  VALUE( SEGV_GPERR       , SEGV_GPERR );
VALUE( SIGFPE             , SIGFPE );
  VALUE( FPE_INTDIV       , FPE_INTDIV );
  VALUE( FPE_INTOVF       , FPE_INTOVF );
  VALUE( FPE_FLTDIV,		FPE_FLTDIV );
  VALUE( FPE_FLTOVF,		FPE_FLTOVF );
  VALUE( FPE_FLTUND,		FPE_FLTUND );
  VALUE( FPE_FLTINV,		FPE_FLTINV );
  VALUE( FPE_FLTRES,		FPE_FLTRES );
  VALUE( FPE_NOFPU        , FPE_NOFPU );
VALUE( SIGILL             , SIGILL );
  VALUE( ILL_ILLOPC       , ILL_ILLOPC );
  VALUE( ILL_COPROC       , ILL_COPROC );
  VALUE(ILL_PRVOPC		  ,	ILL_PRVOPC );
VALUE( SIGBUS             , SIGBUS );
  VALUE( BUS_ADRALN       , BUS_ADRALN );
  VALUE( BUS_OBJERR       , BUS_OBJERR );
VALUE( SIGTRAP            , SIGTRAP );
  VALUE( TRAP_BRKPT       , TRAP_BRKPT );
  VALUE( TRAP_TRACE       , TRAP_TRACE );
VALUE( SIGKILL            , SIGKILL );

VALUE( FLTILL             , FLTILL );
VALUE( FLTPRIV            , FLTPRIV );
VALUE( FLTBPT             , FLTBPT );
VALUE( FLTTRACE           , FLTTRACE );
VALUE( FLTACCESS          , FLTACCESS );
VALUE( FLTBOUNDS          , FLTBOUNDS );
VALUE( FLTIOVF            , FLTIOVF );
VALUE( FLTIZDIV           , FLTIZDIV );
VALUE( FLTFPE             , FLTFPE );
VALUE( FLTSTACK           , FLTSTACK );
VALUE( FLTPAGE            , FLTPAGE );

VALUE( FPUDATA_BUSY       , FPUDATA_BUSY );
VALUE( FPUDATA_CPUMASK    , FPUDATA_CPUMASK );
VALUE( FPUDATA_MASK       , FPUDATA_MASK );
VALUE( IFS_BOOTSTRAP_SIGNATURE,	IFS_BOOTSTRAP_SIGNATURE);

COMMENT("struct sigevent");
VALUE( SIGEV_COID         , offsetof(struct sigevent, sigev_coid) );
VALUE( SIGEV_ID           , offsetof(struct sigevent, sigev_id) );
VALUE( SIGEV_NOTIFY       , offsetof(struct sigevent, sigev_notify) );
VALUE( SIGEV_SIGNO        , offsetof(struct sigevent, sigev_signo) );
VALUE( SIGEV_VALUE        , offsetof(struct sigevent, sigev_value) );
VALUE( SIGEV_CODE         , offsetof(struct sigevent, sigev_code) );
VALUE( SIGEV_PRIORITY     , offsetof(struct sigevent, sigev_priority) );

VALUE( __KER_MSG_RECEIVEV , __KER_MSG_RECEIVEV );
VALUE( __KER_MSG_READV    , __KER_MSG_READV );
VALUE( __KER_NOP          , __KER_NOP );
VALUE( __KER_BAD          , __KER_BAD );
VALUE( __KER_SIGNAL_FAULT , __KER_SIGNAL_FAULT );
VALUE( _NTO_NOIOV         , _NTO_NOIOV );
VALUE( INKERNEL_NOW       , INKERNEL_NOW );
VALUE( INKERNEL_LOCK      , INKERNEL_LOCK );
VALUE( INKERNEL_EXIT      , INKERNEL_EXIT );
VALUE( INKERNEL_SPECRET   , INKERNEL_SPECRET );
VALUE( INKERNEL_INTRMASK  , INKERNEL_INTRMASK );

COMMENT("struct cpupage_entry");
VALUE( CPUPAGE_TLS        , offsetof(struct cpupage_entry, tls) );
VALUE( CPUPAGE_PLS        , offsetof(struct cpupage_entry, pls) );
VALUE( CPUPAGE_CPU        , offsetof(struct cpupage_entry, cpu) );
VALUE( CPUPAGE_SYSPAGE    , offsetof(struct cpupage_entry, syspage) );
VALUE( CPUPAGE_STATE      , offsetof(struct cpupage_entry, state) );
VALUE( CPUPAGE_SIZEOF 	  , sizeof(struct cpupage_entry) );

VALUE( PERFREGS_ENABLED_FLAG, PERFREGS_ENABLED_FLAG );
//
// CPU specific defines.... Have to change the COMMENT macro definition
// 	so that we don't get overlapping variables.
//
#undef COMMENT
#define COMMENT( comm ) char NAME( comment, __LINE__, __ )[] = comm
//Can delete above two lines and uncomment following two once the
//new mkasmoff.h has made it's way through the system - bstecher
//#undef COMMENT_SUFFIX
//#define COMMENT_SUFFIX __
#include "cpu_asmoff.h"

__SRCVERSION("asmoff.c $Rev: 199085 $");
