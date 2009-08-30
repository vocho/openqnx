/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include "kdebug.h"
#include <sys/image.h>
#include <mkasmoff.h>

VALUE( SIZEOF_REG,			sizeof(CPU_REGISTERS) );
VALUE( SIGCODE_USER,		SIGCODE_USER );
VALUE( SIGCODE_INTR,		SIGCODE_INTR );
VALUE( SIGCODE_KERNEL,		SIGCODE_KERNEL );
VALUE( SIGCODE_FATAL,		SIGCODE_FATAL );
VALUE( SIGSEGV            , SIGSEGV );
  VALUE( SEGV_MAPERR      , SEGV_MAPERR );
  VALUE( SEGV_ACCERR      , SEGV_ACCERR );
  VALUE( SEGV_STKERR      , SEGV_STKERR );
  VALUE( SEGV_GPERR       , SEGV_GPERR );
VALUE( SIGFPE             , SIGFPE );
  VALUE( FPE_INTDIV       , FPE_INTDIV );
  VALUE( FPE_INTOVF       , FPE_INTOVF );
  VALUE( FPE_NOFPU        , FPE_NOFPU );
VALUE( SIGILL             , SIGILL );
  VALUE( ILL_ILLOPC       , ILL_ILLOPC );
  VALUE( ILL_COPROC       , ILL_COPROC );
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

VALUE(IFS_BOOTSTRAP_SIGNATURE,	IFS_BOOTSTRAP_SIGNATURE);

#if defined(__X86__)
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
	VALUE( REG_GS         , offsetof(X86_CPU_REGISTERS, fs) );
	VALUE( REG_FS         , offsetof(X86_CPU_REGISTERS, gs) );
	VALUE( REG_ES         , offsetof(X86_CPU_REGISTERS, es) );
	VALUE( REG_DS         , offsetof(X86_CPU_REGISTERS, ds) );
	#endif
	VALUE( REG_EAX        , offsetof(X86_CPU_REGISTERS, eax) );
	VALUE( REG_EBX        , offsetof(X86_CPU_REGISTERS, ebx) );
	VALUE( REG_ECX        , offsetof(X86_CPU_REGISTERS, ecx) );
	VALUE( REG_EDX        , offsetof(X86_CPU_REGISTERS, edx) );
	VALUE( REG_EXX        , offsetof(X86_CPU_REGISTERS, exx) );
	VALUE( REG_EDI        , offsetof(X86_CPU_REGISTERS, edi) );
	VALUE( REG_ESI        , offsetof(X86_CPU_REGISTERS, esi) );
	VALUE( REG_EIP        , offsetof(X86_CPU_REGISTERS, eip) ); 
	VALUE( REG_CS         , offsetof(X86_CPU_REGISTERS, cs) );
	VALUE( REG_EFL        , offsetof(X86_CPU_REGISTERS, efl) );
	VALUE( REG_ESP        , offsetof(X86_CPU_REGISTERS, esp) );
	VALUE( REG_SS         , offsetof(X86_CPU_REGISTERS, ss) );
#elif defined(__PPC__)
	#include <ppc/403cpu.h>
	#include <ppc/800cpu.h>
	#include <ppc/603cpu.h>
	#include <ppc/440cpu.h>
	#include <ppc/e500cpu.h>

	VALUE( FLTNOFPU           , FLTNOFPU );
	VALUE( FLTMACHCHK         , FLTMACHCHK );
	VALUE( FLTBUSERR		  , FLTBUSERR );
	VALUE( FLTBUSTIMOUT		  , FLTBUSTIMOUT );

	VALUE( PPCINT,				sizeof( ppcint ) );
	VALUE( PPC_MSR_EE,			PPC_MSR_EE );
	VALUE( PPC_MSR_FP,			PPC_MSR_FP );
	VALUE( PPC_MSR_IR,			PPC_MSR_IR );
	VALUE( PPC_MSR_DR,			PPC_MSR_DR );
	VALUE( PPC_MSR_DE,			PPC_MSR_DE );
	VALUE( PPC403_BESR_DSES,	PPC403_BESR_DSES );

	VALUE( PPC_CPU_EAR,		PPC_CPU_EAR );

	VALUE( PPC400_SPR_ESR,		PPC400_SPR_ESR );
	VALUE( PPC400_SPR_DEAR,		PPC400_SPR_DEAR );
	VALUE( PPC400_SPR_PID,		PPC400_SPR_PID );

	VALUE( PPC400_DCR_BESR,		PPC403_DCR_BESR );
	VALUE( PPC400_DCR_BEAR,		PPC403_DCR_BEAR );

	VALUE(PPCBKE_SPR_DEAR,		PPCBKE_SPR_DEAR);
	VALUE(PPCBKE_SPR_ESR,		PPCBKE_SPR_ESR);
	VALUE(PPCBKE_SPR_CSRR0,		PPCBKE_SPR_CSRR0);
	VALUE(PPCBKE_SPR_CSRR1,		PPCBKE_SPR_CSRR1);
	VALUE(PPCE500_SPR_MCSRR0,	PPCE500_SPR_MCSRR0);
	VALUE(PPCE500_SPR_MCSRR1,	PPCE500_SPR_MCSRR1);

	VALUE(PPCBKEM_SPR_MAS0,		PPCBKEM_SPR_MAS0);
	VALUE(PPCBKEM_SPR_MAS1,		PPCBKEM_SPR_MAS1);
	VALUE(PPCBKEM_SPR_MAS2,		PPCBKEM_SPR_MAS2);
	VALUE(PPCBKEM_SPR_MAS3,		PPCBKEM_SPR_MAS3);

	VALUE(PPC440_ESR_MCI,		PPC440_ESR_MCI);
	VALUE(PPCBKE_ESR_ST,		PPCBKE_ESR_ST);

	COMMENT("object PPC_CPU_REGISTERS");
	VALUE( REG_GPR         , offsetof(PPC_CPU_REGISTERS, gpr[0]) );
	VALUE( REG_CTR         , offsetof(PPC_CPU_REGISTERS, ctr) );
	VALUE( REG_LR          , offsetof(PPC_CPU_REGISTERS, lr) );
	VALUE( REG_MSR         , offsetof(PPC_CPU_REGISTERS, msr) );
	VALUE( REG_MSR_U       , offsetof(PPC_CPU_REGISTERS, u.msr_u) );
	VALUE( REG_IAR         , offsetof(PPC_CPU_REGISTERS, iar) );
	VALUE( REG_CR          , offsetof(PPC_CPU_REGISTERS, cr) );
	VALUE( REG_XER         , offsetof(PPC_CPU_REGISTERS, xer) );
	VALUE( REG_EAR         , offsetof(PPC_CPU_REGISTERS, ear) );

	COMMENT("Book E TLB");
	VALUE(PPC440_SPR_MMUCR,		PPC440_SPR_MMUCR);
	VALUE(PPC440_MMUCR_STS_SHIFT,PPC440_MMUCR_STS_SHIFT);
	
	VALUE(PPCBKE_TLB_RPN,		offsetof(ppcbke_tlb_t, rpn));
	VALUE(PPCBKE_TLB_EPN,		offsetof(ppcbke_tlb_t, epn));
	VALUE(PPCBKE_TLB_TID,		offsetof(ppcbke_tlb_t, tid));
	VALUE(PPCBKE_TLB_ATTR,		offsetof(ppcbke_tlb_t, attr));
	VALUE(PPCBKE_TLB_ACCESS,	offsetof(ppcbke_tlb_t, access));
	VALUE(PPCBKE_TLB_SIZE,		offsetof(ppcbke_tlb_t, size));
	VALUE(PPCBKE_TLB_TS,		offsetof(ppcbke_tlb_t, ts));
	VALUE(PPCBKE_TLB_V,			offsetof(ppcbke_tlb_t, v));
#elif defined(__MIPS__)
	VALUE( FLTCACHERR,		FLTCACHERR);
	VALUE( FLTUTLBREFILL,	FLTUTLBREFILL);
	VALUE( FLTXTLBREFILL,	FLTXTLBREFILL);
#elif defined(__C6X__)
#elif defined(__SH__)
	VALUE( SH_MMR_CCN_INTEVT,	SH_MMR_CCN_INTEVT);
	VALUE( SH_MMR_CCN_EXPEVT,	SH_MMR_CCN_EXPEVT);
	VALUE( SH_SR_BL,			SH_SR_BL);
	VALUE( SH_SR_RB,			SH_SR_RB);

#elif defined(__ARM__)
	VALUE( ARM_CPSR_MODE_MASK,	ARM_CPSR_MODE_MASK);
	VALUE( ARM_CPSR_MODE_SVC,	ARM_CPSR_MODE_SVC);
	VALUE( ARM_CPSR_I,			ARM_CPSR_I);
	VALUE( ARM_CPSR_F,			ARM_CPSR_F);
	VALUE( ARM_REG_SPSR,		ARM_REG_SPSR);
	VALUE( ARM_REG_PC,			ARM_REG_PC);
	VALUE( ARM_REG_SP,			ARM_REG_SP);
	VALUE( SIGINT,				SIGINT);
	VALUE( SIGSYS,				SIGSYS);
#else
	#error not configured for system
#endif
