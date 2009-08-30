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

//
// Various useful debugging macros
//

#define DEBUGKDBREAK	break 9
#define DEBUGBREAK 		break 10

//#define SHOW_ADDR	0xc7000000
//#define SHOW_ADDR	0xb600ffa0
#define SHOW_ADDR	0xb5f0ffa0

#define SHOWREG(reg1, reg2)		\
		li reg1,SHOW_ADDR; 	\
		sb reg2,(reg1)

#define SHOWNUM(reg1, reg2, num)	\
		li reg2,num;			\
		SHOWREG(reg1,reg2)

#define SHOWCAUSE(reg1, reg2)	\
		mfc0	reg2,CP0_CAUSE;	\
		 nop;					\
		andi	reg2,reg2,MIPS_CAUSE_MASK; \
		srl		reg2,reg2,2;	\
		SHOWREG(reg1,reg2)		\

/*
 * [S,X]DP()
 *	Print out a character to the console
 */
#define BASE_ADDR	0x1e820800	//C4500
//#define BASE_ADDR	0x1e840800	//C7200
#define SER_CHANNEL		0x00		//A channel
//#define SER_CHANNEL	0x40		//B channel

#define XDP(ch,reg1,reg2) \
	li	reg1,0xA0000000+BASE_ADDR+SER_CHANNEL ;\
60:	lb	reg2,0xC(reg1) ;\
	andi	reg2,reg2,4 ;\
	beq	reg2,zero,60b ;\
	 li	reg2,ch ;\
	sb	reg2,0x1C(reg1)
	
#define SDP(ch) \
	addiu	sp,sp,-8; \
	sw		t0,0(sp); \
	sw		t1,4(sp); \
	XDP(ch, t0,t1); \
	lw		t0,0(sp); \
	lw		t1,4(sp); \
	addiu	sp,sp,8
	
#define DP(ch) XDP(ch,k0,k1)

/*
 * HEXSTEP()
 *	Print out hex digit "idx" from t2
 *
 * Burns k0, k1, t0, and t1.  Doesn't modify t2.
 */
#define HEXSTEP(idx) \
	srl	t0,t2,idx ;\
	andi	t0,t0,0xF ;\
	slti	t1,t0,10 ;\
	bne	t1,zero,90f ;\
	 nop ;\
	j	80f ;\
	 addiu	t0,('A'-10) ;\
90:	addiu	t0,'0' ;\
80:	li	k0,0x1E820800+0xA0000000 ;\
70:	lb	k1,0xC(k0) ;\
	andi	k1,4 ;\
	beq	k1,zero,70b ;\
	 nop ;\
	sb	t0,0x1c(k0)

/*
 * HEX()
 *	Print out hex value in t2
 */
#define HEX() \
	HEXSTEP(28); HEXSTEP(24); HEXSTEP(20); HEXSTEP(16);  \
	HEXSTEP(12); HEXSTEP(8); HEXSTEP(4); HEXSTEP(0)

/*
 * REC()
 * Record a number in a ring buffer (useful for execution traces)
 */
#define REC_SAVE() \
		nop; \
		addiu	sp,sp,-16; \
		sw		t0,0(sp); \
		sw		t1,4(sp); \
		sw		t2,8(sp); \
		sw		t3,12(sp)
		
#define REC_RECORD() \
		lui 	t2,%hi(ridx); \
	999: ;\
		ll		t1,%lo(ridx)(t2); \
		addiu	t3,t1,1; \
		andi	t3,0xff; \
		sc		t3,%lo(ridx)(t2); \
		beq		t3,zero,999b; \
		 sll	t1,2; \
		lui		t2,%hi(rbuff); \
		addu	t2,t1; \
		sw		t0,%lo(rbuff)(t2)
		
#define REC_RESTORE() \
		lw		t0,0(sp); \
		lw		t1,4(sp); \
		lw		t2,8(sp); \
		lw		t3,12(sp); \
		addiu	sp,sp,16
		
	
#define REC(v) REC_SAVE(); li t0,v; REC_RECORD(); REC_RESTORE()
#define RECREG(r) REC_SAVE(); move t0,r; REC_RECORD(); REC_RESTORE()
#define RECSMP(v) \
	REC_SAVE(); \
	mfc0 t0,CP0_PRID; \
	li	t2,0xff000000; \
	and t0,t2; \
	ori t0,v; \
	REC_RECORD(); \
	REC_RESTORE()

#define RECINKERNEL(v)	\
	REC_SAVE(); \
	mfc0 t1,CP0_PRID; \
	lui t0,%hi(inkernel); \
	li	t2,0xfe000000; \
	lw t0,%lo(inkernel)(t0); \
	and t1,t2; \
	srl t1,9; \
	li t2,(v) << 12; \
	or t0,t2; \
	or  t0,t1; \
	REC_RECORD(); \
	REC_RESTORE()
	
#define SHOWPROGRESS(off,smp,r1,r2)	\
	.if smp;			\
	mfc0 r1,CP0_PRID;	\
	 li r2,0xb00a0000;	\
	srl r1,25 - 3;		\
	andi r1,0x3 << 3;	\
	addu r2,r1;			\
	.else;				\
	 li r2,0xb00a0000;	\
	.endif;				\
	mfc0 r1,CP0_COUNT;	\
	 nop;				\
	sb r1,off(r2)
	
#define SHOWME(c,off,smp,r1,r2)	\
	.if smp;			\
	mfc0 r1,CP0_PRID;	\
	 li r2,0xb00a0000;	\
	srl r1,25 - 3;		\
	andi r1,0x3 << 3;	\
	addu r2,r1;			\
	.else;				\
	 li r2,0xb00a0000;	\
	.endif;				\
	li r1,c;			\
	sb r1,off(r2)

#define CSHOWME(v, off, smp) \
	*(volatile char *)(0xb00a0000 + (off) + ((smp) ? 8 * RUNCPU : 0)) = (v)

#define CSHOWPROGRESS(off, smp) \
	CSHOWME(getcp0_count(), off, smp)

/* __SRCVERSION("dbgmacros.h $Rev: 153052 $"); */
