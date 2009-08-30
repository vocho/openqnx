/* match.s -- optional optimized asm version of longest match in deflate.c

   Copyright (C) 2002, 2006 Free Software Foundation, Inc.
   Copyright (C) 1992-1993 Jean-loup Gailly

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/*
 * The 68020 version has been written by Francesco Potorti` <pot@cnuce.cnr.it>
 * with adaptations by Carsten Steger <stegerc@informatik.tu-muenchen.de>,
 * Andreas Schwab <schwab@lamothe.informatik.uni-dortmund.de> and
 * Kristoffer Eriksson <ske@pkmab.se>
 *
 * The ia64 version has been written by Sverre Jarp (HP Labs) 2001-2002.
 * Unwind directives and some reformatting for better readability added by
 * David Mosberger-Tang <davidm@hpl.hp.com>.
 */

/* $Id: match.c,v 1.1 2006/11/20 08:40:34 eggert Exp $ */

/* Preprocess with -DNO_UNDERLINE if your C compiler does not prefix
 * external symbols with an underline character '_'.
 */
#ifdef NO_UNDERLINE
#  define _prev             prev
#  define _window           window
#  define _match_start	    match_start
#  define _prev_length	    prev_length
#  define _good_match	    good_match
#  define _nice_match	    nice_match
#  define _strstart	    strstart
#  define _max_chain_length max_chain_length

#  define _match_init       match_init
#  define _longest_match    longest_match
#endif

#ifdef DYN_ALLOC
  error: DYN_ALLOC not yet supported in match.s
#endif

#if defined(i386) || defined(_I386) || defined(__i386) || defined(__i386__)

/* This version is for 386 Unix or OS/2 in 32 bit mode.
 * Warning: it uses the AT&T syntax: mov source,dest
 * This file is only optional. If you want to force the C version,
 * add -DNO_ASM to CFLAGS in Makefile and set OBJA to an empty string.
 * If you have reduced WSIZE in gzip.h, then change its value below.
 * This version assumes static allocation of the arrays (-DDYN_ALLOC not used).
 */

	.file   "match.S"

#define MAX_MATCH	258
#define MAX_MATCH2	$128 /* MAX_MATCH/2-1 */
#define MIN_MATCH	3
#define    WSIZE	$32768
#define MAX_DIST	WSIZE - MAX_MATCH - MIN_MATCH - 1

	.globl	_match_init
	.globl  _longest_match

	.text

_match_init:
	ret

/*-----------------------------------------------------------------------
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */

_longest_match:	/* int longest_match(cur_match) */

#define cur_match   20(%esp)
     /* return address */               /* esp+16 */
        push    %ebp                    /* esp+12 */
        push    %edi                    /* esp+8  */
	push	%esi                    /* esp+4  */
	push    %ebx			/* esp    */

/*
 *      match        equ esi
 *      scan         equ edi
 *      chain_length equ ebp
 *      best_len     equ ebx
 *      limit        equ edx
 */
	mov 	cur_match,%esi
        mov 	_max_chain_length,%ebp /* chain_length = max_chain_length */
	mov 	_strstart,%edi
	mov 	%edi,%edx
	sub	MAX_DIST,%edx          /* limit = strstart-MAX_DIST */
	jae	limit_ok
	sub	%edx,%edx              /* limit = NIL */
limit_ok:
        add	$2+_window,%edi        /* edi = offset(window+strstart+2) */
        mov	_prev_length,%ebx      /* best_len = prev_length */
        movw 	-3(%ebx,%edi),%ax      /* ax = scan[best_len-1..best_len] */
        movw 	-2(%edi),%cx           /* cx = scan[0..1] */
	cmp	_good_match,%ebx       /* do we have a good match already? */
        jb      do_scan
	shr 	$2,%ebp                /* chain_length >>= 2 */
        jmp     do_scan

        .align  4
long_loop:
/* at this point, edi == scan+2, esi == cur_match */
        movw	-3(%ebx,%edi),%ax       /* ax = scan[best_len-1..best_len] */
        movw     -2(%edi),%cx           /* cx = scan[0..1] */
short_loop:
/*
 * at this point, di == scan+2, si == cur_match,
 * ax = scan[best_len-1..best_len] and cx = scan[0..1]
 */
        and     WSIZE-1, %esi
        movw    _prev(%esi,%esi),%si    /* cur_match = prev[cur_match] */
                                        /* top word of esi is still 0 */
        cmp     %edx,%esi		/* cur_match <= limit ? */
        jbe     the_end
        dec     %ebp                    /* --chain_length */
        jz      the_end
do_scan:
        cmpw    _window-1(%ebx,%esi),%ax/* check match at best_len-1 */
        jne     short_loop
        cmpw    _window(%esi),%cx       /* check min_match_length match */
        jne     short_loop

        lea     _window+2(%esi),%esi    /* si = match */
        mov     %edi,%eax               /* ax = scan+2 */
        mov 	MAX_MATCH2,%ecx         /* scan for at most MAX_MATCH bytes */
        rep;    cmpsw                   /* loop until mismatch */
        je      maxmatch                /* match of length MAX_MATCH? */
mismatch:
        movb    -2(%edi),%cl        /* mismatch on first or second byte? */
        subb    -2(%esi),%cl        /* cl = 0 if first bytes equal */
        xchg    %edi,%eax           /* edi = scan+2, eax = end of scan */
        sub     %edi,%eax           /* eax = len */
	sub	%eax,%esi           /* esi = cur_match + 2 + offset(window) */
	sub	$2+_window,%esi     /* esi = cur_match */
        subb    $1,%cl              /* set carry if cl == 0 (cannot use DEC) */
        adc     $0,%eax             /* eax = carry ? len+1 : len */
        cmp     %ebx,%eax           /* len > best_len ? */
        jle     long_loop
        mov     %esi,_match_start       /* match_start = cur_match */
        mov     %eax,%ebx               /* ebx = best_len = len */
        cmp     _nice_match,%eax        /* len >= nice_match ? */
        jl      long_loop
the_end:
        mov     %ebx,%eax               /* result = eax = best_len */
	pop     %ebx
        pop     %esi
        pop     %edi
        pop     %ebp
        ret
maxmatch:
        cmpsb
        jmp     mismatch

#else

/* ======================== 680x0 version ================================= */

#if defined(m68k)||defined(mc68k)||defined(__mc68000__)||defined(__MC68000__)
#  ifndef mc68000
#    define mc68000
#  endif
#endif

#if defined(__mc68020__) || defined(__MC68020__) || defined(sysV68)
#  ifndef mc68020
#    define mc68020
#  endif
#endif

#if defined(mc68020) || defined(mc68000)

#if (defined(mc68020) || defined(NeXT)) && !defined(UNALIGNED_OK)
#  define UNALIGNED_OK
#endif

#ifdef sysV68  /* Try Motorola Delta style */

#  define GLOBAL(symbol)	global	symbol
#  define TEXT			text
#  define FILE(filename)	file	filename
#  define invert_maybe(src,dst)	dst,src
#  define imm(data)		&data
#  define reg(register)		%register

#  define addl			add.l
#  define addql			addq.l
#  define blos			blo.b
#  define bhis			bhi.b
#  define bras			bra.b
#  define clrl			clr.l
#  define cmpmb			cmpm.b
#  define cmpw			cmp.w
#  define cmpl			cmp.l
#  define lslw			lsl.w
#  define lsrl			lsr.l
#  define movel			move.l
#  define movew			move.w
#  define moveb			move.b
#  define moveml		movem.l
#  define subl			sub.l
#  define subw			sub.w
#  define subql			subq.l

#  define IndBase(bd,An)	(bd,An)
#  define IndBaseNdxl(bd,An,Xn)	(bd,An,Xn.l)
#  define IndBaseNdxw(bd,An,Xn)	(bd,An,Xn.w)
#  define predec(An)		-(An)
#  define postinc(An)		(An)+

#else /* default style (Sun 3, NeXT, Amiga, Atari) */

#  define GLOBAL(symbol)	.globl	symbol
#  define TEXT			.text
#  define FILE(filename)	.even
#  define invert_maybe(src,dst)	src,dst
#  if defined(sun) || defined(mc68k)
#    define imm(data)		#data
#  else
#    define imm(data)		\#data
#  endif
#  define reg(register)		register

#  define blos			bcss
#  if defined(sun) || defined(mc68k)
#    define movel		movl
#    define movew		movw
#    define moveb		movb
#  endif
#  define IndBase(bd,An)	An@(bd)
#  define IndBaseNdxl(bd,An,Xn)	An@(bd,Xn:l)
#  define IndBaseNdxw(bd,An,Xn)	An@(bd,Xn:w)
#  define predec(An)		An@-
#  define postinc(An)		An@+

#endif	/* styles */

#define Best_Len	reg(d0)		/* unsigned */
#define Cur_Match	reg(d1)		/* Ipos */
#define Loop_Counter	reg(d2)		/* int */
#define Scan_Start	reg(d3)		/* unsigned short */
#define Scan_End	reg(d4)		/* unsigned short */
#define Limit		reg(d5)		/* IPos */
#define Chain_Length	reg(d6)		/* unsigned */
#define Scan_Test	reg(d7)
#define Scan		reg(a0)		/* *uch */
#define Match		reg(a1)		/* *uch */
#define Prev_Address	reg(a2)		/* *Pos */
#define Scan_Ini	reg(a3)		/* *uch */
#define Match_Ini	reg(a4)		/* *uch */
#define Stack_Pointer	reg(sp)

#define MAX_MATCH 	258
#define MIN_MATCH	3
#define WSIZE		32768
#define MAX_DIST 	(WSIZE - MAX_MATCH - MIN_MATCH - 1)

	GLOBAL	(_match_init)
	GLOBAL	(_longest_match)

	TEXT

	FILE	("match.S")

_match_init:
	rts

/*-----------------------------------------------------------------------
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */

/* int longest_match (cur_match) */

#ifdef UNALIGNED_OK
#  define pushreg	15928		/* d2-d6/a2-a4 */
#  define popreg	7292
#else
#  define pushreg	16184		/* d2-d7/a2-a4 */
#  define popreg	7420
#endif

_longest_match:
	movel	IndBase(4,Stack_Pointer),Cur_Match
	moveml	imm(pushreg),predec(Stack_Pointer)
	movel	_max_chain_length,Chain_Length
	movel	_prev_length,Best_Len
	movel	imm(_prev),Prev_Address
	movel	imm(_window+MIN_MATCH),Match_Ini
	movel	_strstart,Limit
	movel	Match_Ini,Scan_Ini
	addl	Limit,Scan_Ini
	subw	imm(MAX_DIST),Limit
	bhis	L__limit_ok
	clrl	Limit
L__limit_ok:
	cmpl	invert_maybe(_good_match,Best_Len)
	blos	L__length_ok
	lsrl	imm(2),Chain_Length
L__length_ok:
	subql	imm(1),Chain_Length
#ifdef UNALIGNED_OK
	movew	IndBase(-MIN_MATCH,Scan_Ini),Scan_Start
	movew	IndBaseNdxw(-MIN_MATCH-1,Scan_Ini,Best_Len),Scan_End
#else
	moveb	IndBase(-MIN_MATCH,Scan_Ini),Scan_Start
	lslw	imm(8),Scan_Start
	moveb	IndBase(-MIN_MATCH+1,Scan_Ini),Scan_Start
	moveb	IndBaseNdxw(-MIN_MATCH-1,Scan_Ini,Best_Len),Scan_End
	lslw	imm(8),Scan_End
	moveb	IndBaseNdxw(-MIN_MATCH,Scan_Ini,Best_Len),Scan_End
#endif
	bras	L__do_scan

L__long_loop:
#ifdef UNALIGNED_OK
	movew	IndBaseNdxw(-MIN_MATCH-1,Scan_Ini,Best_Len),Scan_End
#else
	moveb	IndBaseNdxw(-MIN_MATCH-1,Scan_Ini,Best_Len),Scan_End
	lslw	imm(8),Scan_End
	moveb	IndBaseNdxw(-MIN_MATCH,Scan_Ini,Best_Len),Scan_End
#endif

L__short_loop:
	lslw	imm(1),Cur_Match
	movew	IndBaseNdxl(0,Prev_Address,Cur_Match),Cur_Match
	cmpw	invert_maybe(Limit,Cur_Match)
	dbls	Chain_Length,L__do_scan
	bras	L__return

L__do_scan:
	movel	Match_Ini,Match
	addl	Cur_Match,Match
#ifdef UNALIGNED_OK
	cmpw	invert_maybe(IndBaseNdxw(-MIN_MATCH-1,Match,Best_Len),Scan_End)
	bne	L__short_loop
	cmpw	invert_maybe(IndBase(-MIN_MATCH,Match),Scan_Start)
	bne	L__short_loop
#else
	moveb	IndBaseNdxw(-MIN_MATCH-1,Match,Best_Len),Scan_Test
	lslw	imm(8),Scan_Test
	moveb	IndBaseNdxw(-MIN_MATCH,Match,Best_Len),Scan_Test
	cmpw	invert_maybe(Scan_Test,Scan_End)
	bne	L__short_loop
	moveb	IndBase(-MIN_MATCH,Match),Scan_Test
	lslw	imm(8),Scan_Test
	moveb	IndBase(-MIN_MATCH+1,Match),Scan_Test
	cmpw	invert_maybe(Scan_Test,Scan_Start)
	bne	L__short_loop
#endif

	movew	imm((MAX_MATCH-MIN_MATCH+1)-1),Loop_Counter
	movel	Scan_Ini,Scan
L__scan_loop:
	cmpmb	postinc(Match),postinc(Scan)
	dbne	Loop_Counter,L__scan_loop

	subl	Scan_Ini,Scan
	addql	imm(MIN_MATCH-1),Scan
	cmpl	invert_maybe(Best_Len,Scan)
	bls	L__short_loop
	movel	Scan,Best_Len
	movel	Cur_Match,_match_start
	cmpl	invert_maybe(_nice_match,Best_Len)
	blos	L__long_loop
L__return:
	moveml	postinc(Stack_Pointer),imm(popreg)
	rts

#else

# if defined (__ia64__)

/* ======================== ia64 version ================================= */

/*
 * 'longest_match.S' (assembly program for gzip for the IA-64 architecture)
 *
 * Optimised for McKinley, but with Merced-compatibility, such as MIB+MIB, used wherever
 * possible.
 *
 * Copyright: Sverre Jarp (HP Labs) 2001-2002
 *
 * See deflate.c for c-version
 * Version 2 - Optimize the outer loop
 */

#include <endian.h>

#if __BYTE_ORDER == ____BIG_ENDIAN
#define  first  shl
#define  second shr.u
#define  count  czx1.l
#else
#define  first  shr.u
#define  second shl
#define  count  czx1.r
#endif

// 24 rotating register (r32 - r55)

#define s_vmatch0		r32
#define s_vmatch1		r33
#define s_vmatbst		r34
#define s_vmatbst1		r35
#define s_amatblen		r36

#define s_tm1			r56
#define s_tm2			r57
#define s_tm3			r58
#define s_tm4			r59
#define s_tm5			r60
#define	s_tm6			r61
#define s_tm7			r62
#define s_tm8			r63

#define s_vlen			r31
#define s_vstrstart		r30
#define s_vchainlen		r29
#define s_awinbest		r28
#define s_vcurmatch		r27
#define s_vlimit		r26
#define s_vscanend		r25
#define s_vscanend1		r24
#define s_anicematch		r23
#define	s_vscan0		r22
#define s_vscan1		r21

#define s_aprev			r20
#define s_awindow		r19
#define s_amatchstart		r18
#define s_ascan			r17
#define s_amatch		r16
#define s_wmask			r15
#define s_ascanend		r14

#define s_vspec_cmatch		r11		// next iteration
#define s_lcsave		r10
#define s_prsave		r9
#define s_vbestlen		r8		// return register

#define s_vscan3		r3
#define s_vmatch3		r2

#define p_no			p2
#define p_yes			p3
#define p_shf			p4		//
#define p_bn2			p5		// Use in loop (indicating bestlen != 2)

#define p_nbs			p9		// not new best_len
#define p_nnc			p10		// not nice_length
#define p_ll			p11
#define p_end			p12

#define MAX_MATCH		258
#define MIN_MATCH		  4
#define WSIZE		      32768
#define MAX_DIST		WSIZE - MAX_MATCH - MIN_MATCH - 1

#define	R_INPUT			1
#define R_LOCAL			31
#define	R_OUTPUT		0
#define R_ROTATING		24
#define MLAT			3
#define SHLAT			2

#define	mova			mov
#define movi0			mov
#define cgtu			cmp.gt.unc
#define cgeu			cmp.ge.unc
#define cneu			cmp.ne.unc

	.global longest_match
	.proc longest_match
	.align 32
longest_match:
// --  Cycle: 0
	.prologue
{.mmi
	 alloc r2=ar.pfs,R_INPUT,R_LOCAL,R_OUTPUT,R_ROTATING
	.rotr scan[MLAT+2], match[MLAT+2], shscan0[SHLAT+1], \
	      shscan1[SHLAT+1], shmatch0[SHLAT+1], shmatch1[SHLAT+1]
	.rotp lc[MLAT+SHLAT+2]
	mova s_vspec_cmatch=in0 // cur_match from input register
	add s_tm1=@gprel(strstart),gp // a(a(strstart))
}{.mmi
	add s_tm3=@gprel(prev_length),gp // a(a(prev_length))
	add s_tm5=@ltoff(window),gp // a(a(window))
	add s_tm6=@ltoff(prev),gp // a(a(prev))
	;;
}{.mmb	//  Cycle: 1
	ld4 s_vstrstart=[s_tm1] // strstart
	ld4 s_vbestlen=[s_tm3] // best_len = prev_length
	brp.loop.imp .cmploop,.cmploop+48
}{.mli
	add s_tm2=@gprel(max_chain_length),gp // a(a(max_chain_length))
	movl s_wmask=WSIZE-1
	;;
}{.mmi	//  Cycle: 2
	ld8 s_aprev=[s_tm6] // a(prev)
	ld8 s_awindow=[s_tm5] // a(window)
	.save pr, s_prsave
	movi0 s_prsave=pr // save predicates
}{.mmi
	add s_tm4=@gprel(good_match),gp // a(a(good_match))
	add s_tm7=@ltoff(nice_match),gp // a(a(nice_match))
	add s_tm8=@ltoff(match_start),gp // a(match_start)
	;;
}{.mmi	//  Cycle: 3
	ld8 s_anicematch=[s_tm7] // a(nice_match)
	ld8 s_amatchstart=[s_tm8] // a(match_start)
	.save ar.lc, s_lcsave
	movi0 s_lcsave=ar.lc // save loop count register
}{.mmi
	.body
	add s_tm1=-(MAX_MATCH + MIN_MATCH),s_wmask // maxdist
	cmp.eq p_ll,p0=r0,r0 // parallel compare initialized as 'true'
	mova s_vcurmatch=s_vspec_cmatch
	;;
}{.mmi	//  Cycle: 4
	ld4 s_vchainlen=[s_tm2] // chain_length=max_chain_length
	ld4 s_tm4=[s_tm4] // v(good_match)
	add s_ascan=s_awindow,s_vstrstart // scan=window + strstart
}{.mmi
	sub s_vlimit=s_vstrstart, s_tm1 // limit=strstart - MAX_DIST
	add s_amatch=s_awindow,s_vspec_cmatch // match=window + cur_match
	and s_vspec_cmatch =s_vspec_cmatch,s_wmask
	;;
}{.mmi	//  Cycle: 5
	add s_amatblen=s_amatch,s_vbestlen //
	cneu p_bn2,p0=2,s_vbestlen // set if bestlen != 2
	add s_ascanend=s_ascan,s_vbestlen // compute a(scan) + best_len
}{.mmi
	ld1 s_vscan0=[s_ascan],1 // NB: s_ascan++
	ld1 s_vmatch0=[s_amatch],1
	cgtu p0,p_no=s_vlimit,r0 // is result positive ?
	;;
}{.mmi	//  Cycle: 6
	ld1.nt1 s_vscan1=[s_ascan],2 // NB: s_ascan+3 in total
	ld1.nt1 s_vmatch1=[s_amatch],2
	add s_awinbest=s_awindow,s_vbestlen //
	;;
}{.mmi	//  Cycle: 7
	ld1.nt1 s_vscanend=[s_ascanend],-1 // scan_end=scan[best_len]
	ld1.nt1 s_vmatbst=[s_amatblen],-1
(p_no)	mova s_vlimit=r0
	;;
}{.mmi	//  Cycle: 8
(p_bn2)	ld1.nt1 s_vscanend1=[s_ascanend],1 // scan_end1=scan[best_len-1]
(p_bn2)	ld1.nt1 s_vmatbst1=[s_amatblen]
	shladd s_vspec_cmatch =s_vspec_cmatch,1,s_aprev
}{.mmi
	cgeu p_shf,p0=s_vbestlen,s_tm4 // is (prev_length >= good_match) ?
	;;
}{.mmi	//  Cycle: 9
	ld1.nt1 s_vscan3=[s_ascan]
	ld2.nt1 s_vspec_cmatch=[s_vspec_cmatch]
	mova	s_vlen=3
}{.mmi
(p_shf)	shr.u s_vchainlen=s_vchainlen,2 // (cur_len) >> 2
	;;
}{.mmi	//  Cycle: 10
	ld1.nt1 s_vmatch3=[s_amatch]
	// p_ll switched on as soon as we get a mismatch:
	cmp.eq.and p_ll,p0=s_vmatch0,s_vscan0
	cmp.eq.and p_ll,p0=s_vmatbst,s_vscanend
}{.mib
	cmp.eq.and p_ll,p0=s_vmatch1,s_vscan1
(p_bn2)	cmp.eq.and p_ll,p0=s_vmatbst1,s_vscanend1
(p_ll)	br.cond.dpnt.many .test_more
	;;
}

.next_iter:
{.mmi	// Cycle 0
	add s_amatch=s_awindow,s_vspec_cmatch  	// match=window + cur_match
	mov s_vcurmatch=s_vspec_cmatch		// current value
	add s_vchainlen=-1,s_vchainlen 		// --chain_length
}{.mib
	cmp.le.unc p_end,p0=s_vspec_cmatch,s_vlimit
	and s_vspec_cmatch=s_vspec_cmatch,s_wmask
(p_end)	br.cond.dptk.many .terminate
	;;
}{.mmi	// Cycle 1
	ld1 s_vmatch0=[s_amatch],1		// load match[0]
	// compute prev[cur_match]:
	shladd s_vspec_cmatch=s_vspec_cmatch,1,s_aprev
	cmp.eq.unc p_end,p0=s_vchainlen,r0
} {.mib
	nop.m 0
	add s_amatblen=s_awinbest,s_vcurmatch	// match=window + cur_match
(p_end)	br.cond.dptk.many .terminate
	;;
}{.mmi	// Cycle 2 (short)
	ld2.nt1 s_vspec_cmatch=[s_vspec_cmatch]		// get next cur_match
	;;
}{.mmi	// Cycle 3 (short)
	ld1.nt1 s_vmatbst=[s_amatblen],-1	// load match[best_len]
	cmp.ne.unc p_ll,p0=r0,r0     // parallel compare initialized as 'false'
	;;
}{.mmi	// Cycle 4 (short)
	// load match[1] - - note: match += 3 (in total):
	ld1.nt1 s_vmatch1=[s_amatch],2
	;;
	// Cycle 5  (short)
(p_bn2)	ld1.nt1 s_vmatbst1=[s_amatblen]		// load match[best_len-1]
}{.mib	// Here we (MOST LIKELY) pay a L2-fetch stall
	// p_ll switched on as soon as we get a mismatch:
	cmp.ne.or p_ll,p0=s_vmatch0,s_vscan0
	cmp.ne.or p_ll,p0=s_vmatbst,s_vscanend
(p_ll)	br.cond.dptk.many .next_iter
	;;
}{.mmi	// Cycle 6
	ld1.nt1 s_vmatch3=[s_amatch]
	mova s_vlen=3
	nop.i 0
}{.mib
	cmp.ne.or p_ll,p0=s_vmatch1,s_vscan1
(p_bn2)	cmp.ne.or p_ll,p0=s_vmatbst1,s_vscanend1
(p_ll)	br.cond.dptk.many .next_iter
	;;
}

// We have passed the first hurdle - Are there additional matches ???

.test_more:
{.mmi	// Cycle 0
	and s_tm3=7,s_ascan			// get byte offset
	and s_tm4=7,s_amatch			// get byte offset
	movi0 ar.ec=MLAT+SHLAT+2		// NB: One trip more than usual
}{.mib
	cmp.ne.unc p_no,p0=s_vscan3,s_vmatch3	// does not next one differ?
(p_no)  br.cond.dptk.many .only3
	;;
}{.mmi	// Cycle 1
	and s_tm1=-8,s_ascan	// get aligned address
	shladd s_tm3=s_tm3,3,r0
	movi0 ar.lc=31		// 32 times around the loop (8B at a time)
}{.mib
	and s_tm2=-8,s_amatch			// get aligned address
	shladd s_tm4=s_tm4,3,r0
	nop.b 0
	;;
}{.mmi	// Cycle 2
	ld8.nt1 scan[1]=[s_tm1],8			// load first chunk
	sub s_tm5=64,s_tm3				// 64 - amount
	movi0 pr.rot=1<<16
}{.mmi
	ld8.nt1 match[1]=[s_tm2],8	// load first chunk
	sub s_tm6=64,s_tm4		// 64 - amount
	add s_vlen=-8,s_vlen		// will be updated at least once
	;;
}
	.align 32
.cmploop:
{.mmi	// Cycle 0
(lc[0])			ld8 scan[0]=[s_tm1],8		// next scan chunk
(lc[MLAT+SHLAT+1]) 	add s_vlen=8,s_vlen
(lc[MLAT])		first shscan0[0]=scan[MLAT+1],s_tm3
}{.mib
(lc[MLAT+SHLAT+1]) 	cmp.ne.unc p_no,p0=s_tm7,s_tm8	// break search if !=
(lc[MLAT])		first shmatch0[0]=match[MLAT+1],s_tm4
(p_no)			br.cond.dpnt.many .mismatch
			;;
}{.mii  // Cycle 1
(lc[0])			ld8 match[0]=[s_tm2],8
			// shift left(le) or right(be):
(lc[MLAT])		second shscan1[0]=scan[MLAT],s_tm5
(lc[MLAT])		second shmatch1[0]=match[MLAT],s_tm6
}{.mmb
(lc[MLAT+SHLAT])	or s_tm7=shscan0[SHLAT],shscan1[SHLAT]
(lc[MLAT+SHLAT])	or s_tm8=shmatch0[SHLAT],shmatch1[SHLAT]
			br.ctop.dptk.many .cmploop
			;;
}{.mfi
	mov s_vlen=258
	nop.f 0
}{.mfi
	nop.f 0    // realign
	;;
}
.mismatch:
{.mii	// Cycle 0 (short)
(p_no)	pcmp1.eq s_tm2=s_tm7,s_tm8 	// find first non-matching character
	nop.i 0
	;;
	// Cycle 1 (short)
(p_no)	count s_tm1=s_tm2
	;;
}{.mib	// Cycle 2 (short)
(p_no)	add s_vlen=s_vlen,s_tm1		// effective length
	nop.i 0
	clrrrb
	;;
}

.only3:
{.mib	// Cycle 0 (short)
	cmp.gt.unc p0,p_nbs=s_vlen,s_vbestlen		// (len > best_len) ?
(p_nbs)	br.cond.dpnt.many .next_iter			// if not, re-iternate
	;;
}{.mmi	// Cycle 1 (short)
	ld4 s_tm7=[s_anicematch] 			// nice_match
	st4 [s_amatchstart]= s_vcurmatch
	add s_ascanend=s_ascan,s_vlen			// reset with best_len
	;;
}{.mmi	// Cycle 2 (short)
	mova s_vbestlen=s_vlen
	add s_ascanend=-3,s_ascanend		// remember extra offset
	;;
}{.mmi	// Cycle 3 (short)
	ld1 s_vscanend=[s_ascanend],-1		// scan_end=scan[best_len]
	add s_awinbest=s_awindow,s_vbestlen	// update with new best_len
  	cmp.ne.unc p_bn2,p0=2,s_vbestlen	// set if bestlen != 2
	;;
}{.mib	// Cycle 4 (short)
	// scan_end1=scan[best_len-1] NB: s_ascanend reset:
	ld1.nt1 s_vscanend1=[s_ascanend],1
	cmp.lt.unc p_nnc,p0=s_vlen,s_tm7	// compare with nice_match
(p_nnc)	br.cond.dptk.many .next_iter
	;;
}
.terminate:
{.mii	// Cycle 0/1
	nop.m 0
	movi0 ar.lc=s_lcsave
	movi0 pr=s_prsave,-1
}{.mbb
	nop.m 0
	nop.b 0
	br.ret.sptk.many rp	// ret0 is identical to best_len
	;;
}
	.endp

	.global match_init
	.proc match_init
match_init:
	sub ret0=ret0,ret0
	br.ret.sptk.many rp
	.endp

# else
 error: this asm version is for 386 or 680x0 or ia64 only
# endif /* __ia64__ */
#endif /* mc68000 || mc68020 */
#endif /* i386 || _I386   */
