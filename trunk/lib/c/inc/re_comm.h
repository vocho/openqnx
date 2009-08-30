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




#ifdef DEBUGGING
#include	<stdio.h>
#endif

#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>
#include	<malloc.h>
#include	<limits.h>
#include	<ctype.h>
#include	<regex.h>

#define	_tolower(x)	(isupper(x) ? ((x)-'A'+'a') : (x))
#define	_toupper(x)	(islower(x) ? ((x)-'a'+'A') : (x))
/*

	Regular Expression handling.
	
	This first section is common to the routines "regcomp.c" and "regexec.c".
	To avoid polluting namespaces, etc, this was not placed in the standard
	header, so if you modify the first section, you must ensure that you ref
	reflect the changes in both files.....
	The section ends with : --END OF COMMON SECTION-- 

	The regular expressions are either 'extended' or 'basic'. 
	The expressions are translated into an array of 16-bit unsigned values,
	representing a non-deterministic finite state machine, with a simple 
	addition for allowing 'back-referenced' patterns (\1,\2,...).
	A simple attempt at optimisation is made for the 'basic' expressions.
	After compilation, the machine is examined to see if there is a set of
	characters, one of which is necessary to consider a match.  
	If such a set is found ('^.*$' has no such set) then it is used by
	regexec() to skip impossible strings.  In practice this represents a
	reduction in execution time of 50-80% on 'normal' expressions.
	
	The expression may be converted into a Deterministic Finite Automaton
	by generating a table of states and the transition from each state on each
	character thus table size is O(|expression|*|alphabet|).  In general, the
	time to generate such a table is O(|expression| ** 3).  There are well 
	documented algorithms for both compressing the table, and delaying the 
	generation of the state table entries until they are 'needed'.

	The machine is the concatenation of a set of simple op-codes.
	These are described below.
	Each entry, or opcode, is divided into 3 fields: OP_CLASS, OP_CODE and
	OPERAND.   The OP_CLASS may take on the values:
		RE_NORMAL:
			The OP_CODE is to be executed once.
		RE_ALTERNATE:
			Impliments an "OR" of the machines @(IP+1) @(IP+2).
			The OP_CODE is ignored, and the operand is the location of the
			Next operand after choosing the "OR"
		RE_REPEAT:
			If the OP_CODE is RE_NOP, then the machine following will be
			executed from (IP+1) up to (IP+2) times, then branch to the
			OPERAND field.
			Any other OP_CODE will be executed individually (IP+1) to (IP+2)
			times.	
			If (IP+2) is REG_INFINITY, then it will be
			executed up to the maximum possible.
		RE_ENDPAT:
			Marks the end of a pattern or sub-pattern.
	All 'address' fields are relative to the IP of the beginning of the opcode,
	thus the machine may be 'relocated' to start at any index.
	The OP_CODES may take on:
		RE_NOP:
			Nil opcode -- matches epsilon.
		RE_ANY:
			Any character except nil.
		RE_CCL:
			A character class, the operand encodes an index into the 
			"re_classtab" entry in the regex_t.
			Matches any char in the set.
		RE_EOLN:
			Matches the end of a string only.
		RE_BOL:
			Matches the beginning of a string only.
		RE_EOP:
			End of pattern marker.
		RE_BRACK:
			Start sub-pattern (N)
		RE_CBRACK:
			end sub-pattern(N)
		RE_BACK:
			match same string as sub-pattern(N)

	If 'ignore-case' has been specified, it is assumed that all alphabetic
	characters in the machine are represented in lower case.   In classes,
	the characters are represented in both cases, to allow greater compaction.


	You will notice that the routines:
		ere_fork(),
		re_repeat(),
		re_match()
	are very recursive.  For relatively simple patterns, this is not a 
	short coming, however for matches of the form :
		"(steve|john)*(steve|john)*(steve|john)* ...  stevestevejohn'
	The execution speed will slow exponentially with the length of the
	target string. 
	
*/

#define	REG_INFINITY (-1)

#define	REG_MOD(x)	((x) & 0xf000)
#define	REG_OPR(x)	((x) & 0x0f00)
#define	REG_FOP(x)	((x) & 0xff00)
#define	REG_OPRND(x) ((x) & 0xff)

/*
 * define the modes
 */
enum {
	RE_NORMAL	=	0 << 12,
	RE_ALTERNATE	=	1 << 12,
	RE_REPEAT	=	2 << 12,
	RE_ENDPAT	=	3 << 12
};
/*
 * define the operations
 */
enum {
	RE_NOP	=	0 << 8,
	RE_ANY	=	1 << 8,
	RE_CCL	=	2 << 8,
	RE_NCCL =	3 << 8,
	RE_CHAR =	4 << 8,
	RE_EOLN	=	5 << 8,
	RE_BOL	=	6 << 8,
	RE_EOP	=	7 << 8,
	RE_BRACK=	8 << 8,
	RE_CBRACK=	9 << 8,
	RE_BACK=	10<< 8
};

#define	NBRA		10

/*
	These are basic bit-set manipulators.
	This forms a self-sufficient header, but is inlined here
	to make library compiling easier.
*/

#define	BIT_WIDTH	010
#define	BITS_SHIFT	003
#define	BITS_MASK	007

typedef	unsigned char BitEl, *BitVect;

#define	BIT_LEN(_n)	(((_n)>>BITS_SHIFT)+(((_n)&BITS_MASK) != 0))
#define	BitSET(_X,_n)	BitEl _X[BIT_LEN(_n)]

#define	_THIS_BYTE(_i)	((_i) >> BITS_SHIFT)
#define	_THIS_BIT(_i)	(1 << ((_i)&BITS_MASK))

#define	INSET(_X,_i)	((_X)[_THIS_BYTE(_i)] &   _THIS_BIT(_i))
#define	ADDSET(_X,_i)	((_X)[_THIS_BYTE(_i)] |=  _THIS_BIT(_i))
#define	DELSET(_X,_i)	((_X)[_THIS_BYTE(_i)] &= ~_THIS_BIT(_i))

#define	clearset(_X,_n)		memset((_X),0,BIT_LEN((_n)))
#define	clearbits(_X,_n)	memset((_X),0,(_n))

#define	BIT_ALLOC(_x)	(calloc(sizeof(char),BIT_LEN(_x)))

#ifndef LOCAL
#ifdef DEBUGGING
#define	LOCAL
#else
#define	LOCAL static 
#endif
#endif

#include <setjmp.h>

/*    don't allocate in smaller chunks */
#define    MIN_NINCR      32   

LOCAL	jmp_buf	wherewasi;

#define	READY_ERROR()	setjmp(wherewasi)
#define	RAISE_ERROR(ecode)	longjmp(wherewasi,ecode)

/* -- END OF COMMON SECTION --*/

/* __SRCVERSION("re_comm.h $Rev: 153052 $"); */
