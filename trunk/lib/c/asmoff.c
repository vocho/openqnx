/*
Copyright 2001, QNX Software Systems Ltd. All Rights Reserved
 
This source code has been published by QNX Software Systems Ltd. (QSSL).
However, any use, reproduction, modification, distribution or transfer of
this software, or any software which includes or is based upon any of this
code, is only permitted under the terms of the QNX Realtime Plaform End User
License Agreement (see licensing.qnx.com for details) or as otherwise
expressly authorized by a written license agreement from QSSL. For more
information, please email licensing@qnx.com.
*/
#include <stddef.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/neutrino.h>
#include <sys/kercalls.h>
#include <mkasmoff.h>
#include <sys/syspage.h>

COMMENT("object struct _sighandler_info");
VALUE(SIGSTACK_SIGINFO, offsetof(struct _sighandler_info, siginfo));
VALUE(SIGSTACK_SIGNO,   offsetof(struct _sighandler_info, siginfo.si_signo)); 
VALUE(SIGSTACK_SIGCODE, offsetof(struct _sighandler_info, siginfo.si_code)); 
VALUE(SIGSTACK_SIGVALUE,offsetof(struct _sighandler_info, siginfo.si_value)); 
VALUE(SIGSTACK_HANDLER, offsetof(struct _sighandler_info, handler));
VALUE(SIGSTACK_CONTEXT, offsetof(struct _sighandler_info, context));

COMMENT("object siginfo_t");
VALUE(SIGINFO_SIGNO,    offsetof(siginfo_t, si_signo));
VALUE(SIGINFO_SIGCODE,  offsetof(siginfo_t, si_code));
VALUE(SIGINFO_SIGVALUE, offsetof(siginfo_t, si_value));

COMMENT("object ucontext_t");
VALUE(UCONTEXT_CPU,    offsetof(ucontext_t, uc_mcontext.cpu));
extern ucontext_t	dummy; //Doesn't really exist, just using for sizeof
VALUE(SIZE_CONTEXTCPU, sizeof(dummy.uc_mcontext.cpu));
VALUE(UCONTEXT_FPU,    offsetof(ucontext_t, uc_mcontext.fpu));

COMMENT("object struct _thread_local_storage");
VALUE(TLS_STACKADDR,	offsetof(struct _thread_local_storage, __stackaddr));
VALUE(TLS_FPUEMU_DATA,	offsetof(struct _thread_local_storage, __fpuemu_data));

COMMENT("Signal kercall number");
VALUE(KER_SIGNAL_RETURN,__KER_SIGNAL_RETURN);

COMMENT("CPU flags");
#if defined(__X86__)
VALUE(X86_CPU_SEP, X86_CPU_SEP);
#elif defined(__MIPS__)
VALUE(MIPS_CPU_FLAG_64BIT, MIPS_CPU_FLAG_64BIT);
#elif defined(__PPC__)
VALUE(PPC_CPU_SPE, PPC_CPU_SPE);
#elif defined(__ARM__)
VALUE(ARM_CPU_FLAG_V6, ARM_CPU_FLAG_V6);
#endif
