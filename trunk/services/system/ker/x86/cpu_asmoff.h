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

VALUE( FLTNMI             , FLTNMI );
VALUE( FLTDBLFLT          , FLTDBLFLT );
VALUE( FLTOLDFPE          , FLTOLDFPE );
VALUE( FLTTSS             , FLTTSS );
VALUE( FLTSEG             , FLTSEG );
VALUE( FLTRSVD            , FLTRSVD );
VALUE( FLTNOFPU           , FLTNOFPU );
VALUE( FLTMACHCHK         , FLTMACHCHK );

COMMENT("object X86_CPU_REGISTERS");
#ifdef __SEGMENTS__
VALUE( REG_GS         , offsetof(X86_CPU_REGISTERS, gs) );
VALUE( REG_FS         , offsetof(X86_CPU_REGISTERS, fs) );
VALUE( REG_ES         , offsetof(X86_CPU_REGISTERS, es) );
VALUE( REG_DS         , offsetof(X86_CPU_REGISTERS, ds) );
#endif
VALUE( REG_EAX        , offsetof(X86_CPU_REGISTERS, eax) );
VALUE( REG_EBX        , offsetof(X86_CPU_REGISTERS, ebx) );
VALUE( REG_ECX        , offsetof(X86_CPU_REGISTERS, ecx) );
VALUE( REG_EDX        , offsetof(X86_CPU_REGISTERS, edx) );
VALUE( REG_EDI        , offsetof(X86_CPU_REGISTERS, edi) );
VALUE( REG_ESI        , offsetof(X86_CPU_REGISTERS, esi) );
VALUE( REG_EIP        , offsetof(X86_CPU_REGISTERS, eip) ); 
VALUE( REG_CS         , offsetof(X86_CPU_REGISTERS, cs) );
VALUE( REG_EFL        , offsetof(X86_CPU_REGISTERS, efl) );
VALUE( REG_EBP        , offsetof(X86_CPU_REGISTERS, ebp) );
VALUE( REG_ESP        , offsetof(X86_CPU_REGISTERS, esp) );
VALUE( REG_SS         , offsetof(X86_CPU_REGISTERS, ss) );

VALUE( V86_STUBCODE   , offsetof(struct _v86_memory, stubcode) );
VALUE( V86_EXECCODE   , offsetof(struct _v86_memory, execcode) );
VALUE( V86_REG        , offsetof(struct _v86_memory, reg) );
VALUE( SIZEOF_V86REG  , sizeof(struct _v86reg) );
VALUE( X86_FAULT_WRITE, X86_FAULT_WRITE);
VALUE( X86_PSW_IF     , X86_PSW_IF);
VALUE( X86_PSW_TF     , X86_PSW_TF);
VALUE( X86_PSW_VM     , X86_PSW_VM);
VALUE(SYSENTER_EFLAGS_BIT, SYSENTER_EFLAGS_BIT);	
VALUE( X86_CPU_SEP    , X86_CPU_SEP);
VALUE( X86_CPU_FXSR   , X86_CPU_FXSR);
VALUE( X86_V86_GPF_CS , 0x78); // CS value used for handling V86 GPF exceptions
VALUE( TSS_IOMAP_DATA , offsetof(X86_TSS, iomap_data));

/* __SRCVERSION("cpu_asmoff.h $Rev: 202117 $"); */
