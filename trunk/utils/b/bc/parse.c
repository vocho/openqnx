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



#ifndef lint
char yysccsid[] = "@(#)yaccpar	1.4 (Berkeley) 02/25/90";
#endif
#line 3 "parse.y"


/*-
parser & intermediate format generator for BC language.

description:
	BC is quite straight-forward.	It is a little bizarre about
	newlines, which i have lessened with a linkage to the lexical
	analyzer to tell it whether or not to ignore newline characters.
	Notice that newlines are not sent-forward as such. They are
	either absorbed by yylex or translated into ';'.

		
	The intermediate-generation is straightforward.	The model is
	a stack-machine with jumps,calls,returns.	The label-generation
	logic is a little convoluted, and is annotated later in this file.
*/



#include <libc.h>
#include "bc.h"
#include "bcgen.h"

extern	int	 list;



/*-	
 This table maintains a mini-stack for parameter and local-var
 declarations.
	
 52 is sensible since there are only 26 scalar and 26 array registers,
 any more is a little on the silly side
*/

#define MAX_LOCAL	52
#ifdef static
#undef static
#define static
#endif
static	int	formal_count = 0;
static	int	local_count = 0;
static	int	local_base = 0;
static	int	formal[MAX_PARM];
static	int	local[MAX_LOCAL];
extern	int	trace;

#define ADD_LOCAL(_X)	((local_count < MAX_LOCAL) ? \
			(local[local_count++] = (_X)) : 0);
#define ADD_FORMAL(_X)	((formal_count < MAX_PARM) ? \
			(formal[formal_count++] = (_X)) : 0)



/* values returned by yyparse().  */
enum	{
	_IS_EMPTY = 1,
	_IS_FUNC,
	_IS_STMT,
	_IS_DEBUG,
	_IS_ERROR
};


/*
	these are values used during parsing to keep track of various
	parameters.
*/

#define IS_ARRAY	01
#define _HAS_ASSIGN	0x01000

/*	
	These are for back-patching the labels in the code.
*/

#define LABEL_FUNC	0	/* store the label for a function name */
#define LABEL_LOOP	1	/* top of a loop */
#define LABEL_EXIT	2	/* exit point of a loop */
#define LABEL_FOR1	3	/* target for a for loop */
#define LABEL_FOR2	4	/* extra target for for loop */
#define LABEL_RETURN	5	/* all returns from the bottom of a function */
#define LABEL_IFEXIT	6	/* if statements exit to this point */

#define _LABEL_DEFINED	01
#define _LABEL_USED	02
#define _LABEL_MASK	03
/*
	The NIL_LABEL marks the end of a list.
*/
#define NIL_LABEL	-1

/*	These may be used later, but are a little confusing at first! */
#define is_defined(_c,_lbl) ((_c)->lbl_flags & (_LABEL_DEFINED << (_lbl*2)))

#define is_used(_c,_lbl) ((_c)->lbl_flags & (_LABEL_USED << (_lbl*2)))

#define do_define(_c,_lbl) ((_c)->label[_lbl] = _c->cur_offs, \
					(_c)->lbl_flags &= ~(_LABEL_MASK << (_lbl*2)),\
					(_c)->lbl_flags |= (_LABEL_DEFINED << (_lbl*2)))

#define do_reference(_c,_lbl)	((_c)->lbl_flags &= ~(_LABEL_MASK << (_lbl*2)), \
				(_c)->lbl_flags |= (_LABEL_USED << (_lbl*2)))


int emit(struct icode *, int nwords, ...);

#ifndef	MIN_CODE_ALLOC
#define MIN_CODE_ALLOC	64
#endif
	
/*
	this stack maintains block structures, such as functions,
	if, while, for stmts, nested to a depth of MAX_CSTACK
*/

#ifndef	MAX_CSTACK
#define MAX_CSTACK	50
#endif

static	struct	icode	*code_stack[MAX_CSTACK];
static	int icodeptr = 0;

#define push(p)	((icodeptr >= MAX_CSTACK) ? \
					(struct icode *) NULL\
: (code_stack[icodeptr++] = (p)))
#define pop()	((icodeptr <= 0) ? NULL : code_stack[--icodeptr])
#define stack_top() ((icodeptr <= 0) ? NULL : code_stack[icodeptr-1])
#define stack(_i)	(((_i) >= icodeptr) ? NULL : code_stack[(_i)])
#define clearstk()	(icodeptr = 0)
#define stktop()	(icodeptr)


#define code_space(_c) ((_c)->ilen - (_c)->cur_offs)


static struct icode *newsect(int);
extern oprnd_t	*make_perm(numb_t *);
extern oprnd_t	*make_temp(numb_t *);
extern int	yylex(void);
extern int	yyerror(char *);
extern void	execute(int *,int );
extern int	add_func(int ,int *,int );
extern int	parse_error(char *);
extern void	collect_garbage(void);

int yyparse();
static void make_label(int lbl);
static int add_jmp(int cond, int label);
static void join_section(struct icode *to, struct icode *from);
static void trim_code(struct icode *p, int size);


static	struct	icode	*cur;	/* scratch variable for current code block */

#line 155 "parse.y"
typedef union	{
	 int	i_val;
	 char	*s_val;
} YYSTYPE;
#line 159 "y.tab.c"
#define DEFINE 257
#define IF 258
#define WHILE 259
#define FOR 260
#define BREAK 261
#define QUIT 262
#define AUTO 263
#define RETURN 264
#define CONTINUE 265
#define LEX_ERROR 266
#define PUSHL 267
#define PUSHR 268
#define DISCARD 269
#define SAVEV 270
#define RESTV 271
#define POPV 272
#define JMP 273
#define JMPZ 274
#define JMPNZ 275
#define CALL 276
#define PRINT 277
#define SAVEVAR 278
#define RESTVAR 279
#define SAVEARRAY 280
#define RESTARRAY 281
#define INDEX 282
#define PUSHVAR 283
#define PUSHCONST 284
#define PUSHARRAY 285
#define PUSHSPECIAL 286
#define POPVAR 287
#define POPARRAY 288
#define REPLACE 289
#define CLRSTK 290
#define CONST 291
#define IDENT 292
#define PRINT_LIST 293
#define IBASE 294
#define OBASE 295
#define SCALE 296
#define SQRT 297
#define LENGTH 298
#define EQ 299
#define GE 300
#define LE 301
#define NE 302
#define PLUSPLUS 303
#define MINUSMINUS 304
#define ASG_PLUS 305
#define ASG_MINUS 306
#define ASG_STAR 307
#define ASG_DIV 308
#define ASG_MOD 309
#define ASG_EXP 310
#define UMINUS 311
#define ASSIGN 312
#define LOR 313
#define LAND 314
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    0,    3,    3,    3,    3,    3,    3,    3,
    3,    3,    3,    3,    3,    3,    3,   19,    1,    8,
    8,    6,    6,    6,    5,    5,    5,    9,    9,    7,
    7,   10,    2,    2,    2,   11,    4,    4,   20,   21,
   14,   22,   23,   13,   24,   25,   26,   27,   12,   15,
   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,
   15,   15,   15,   15,   15,   15,   15,   15,   15,   15,
   15,   15,   15,   15,   15,   16,   16,   16,   16,   16,
   17,   17,   17,   18,   18,
};
short yylen[] = {                                         2,
    0,    1,    1,    2,    2,    1,    1,    1,    1,    2,
    2,    3,    2,    1,    1,    1,    1,    0,   11,    1,
    3,    0,    1,    3,    0,    1,    3,    1,    3,    1,
    3,    3,    0,    1,    2,    3,    1,    2,    0,    0,
    7,    0,    0,    7,    0,    0,    0,    0,   13,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    3,    3,
    3,    3,    3,    3,    2,    2,    1,    1,    3,    2,
    2,    2,    2,    4,    1,    1,    4,    1,    1,    1,
    4,    4,    4,    1,    1,
};
short yydefred[] = {                                      0,
   17,    0,   39,   42,    0,    0,   14,    0,    0,   16,
    0,    0,   68,    0,    0,   78,   79,    0,    0,    0,
    0,    0,   85,   84,    0,    0,    0,    3,    2,    6,
    9,    8,    7,    0,    0,   75,   15,    0,    0,    0,
   45,   10,    0,   13,   11,    0,   37,    0,    5,    0,
    0,    0,    0,    0,    0,   80,   70,   71,   66,   65,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    4,   72,   73,    0,    0,    0,
    0,    0,   12,   69,   36,   38,    0,    0,   23,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   26,    0,    0,    0,    0,   74,    0,   77,   82,
   81,   83,    0,   18,    0,   40,   43,    0,   21,   24,
   29,    0,   27,    0,    0,    0,    0,   41,   44,    0,
    0,    0,    0,    0,   34,    0,    0,   30,    0,   35,
    0,    0,   32,   19,   48,   31,    0,   49,
};
short yydgoto[] = {                                      27,
   28,  144,   47,   48,  111,   88,  147,   89,  112,  145,
   30,   31,   32,   33,   34,   35,   36,   37,  132,   39,
  134,   40,  135,   82,  128,  142,  157,
};
short yysindex[] = {                                    475,
    0, -291,    0,    0,  -12,   12,    0,  577,   12,    0,
  423,  636,    0,   12,  -26,    0,    0,    7,    8,   22,
 -160, -160,    0,    0,  423,  423,    0,    0,    0,    0,
    0,    0,    0,  174, -202,    0,    0,   30,   65,   69,
    0,    0,  174,    0,    0,  937,    0,  490,    0,  866,
  423,  423,  423,  423,  -85,    0,    0,    0,    0,    0,
  423,  423,  423,  423,  423,  423,  423,  423,  423,  423,
  423,  423,  423,  423,    0,    0,    0,  423, -181,  423,
  423,  423,    0,    0,    0,    0,  -24,  -15,    0, 1048,
  944,  953,  979, 1003,  -22,  -34,  -34,  -22, 1057, 1078,
  -34,  -34,  -30,  -30,   23,   23,   23,   23, 1048,   29,
    2,    0, 1012, 1019, 1048,  138,    0,  866,    0,    0,
    0,    0,   21,    0, -181,    0,    0,   57,    0,    0,
    0,   -2,    0,  636,  636,  423,  112,    0,    0, 1048,
 -135,   72, -181,  541,    0,  423,    9,    0,  561,    0,
 1041, -181,    0,    0,    0,    0,  636,    0,
};
short yyrindex[] = {                                    139,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -10,    0,    0,   14,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   44,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   25,
    0,    0,    0,    0,   53,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   36,    0,
    0,    0,    0,    0,    0,    0,  663,    0,    0,   38,
    0,    0,    0,    0,  240,  212,  278,  242,  218,  214,
  331,  358,  182,  204,   82,  108,  144,  153,   89,   34,
    0,    0,    0,    0,   81,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   97,
  657,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
    0,    0,  411,   13,    0,    0,    0,   40, -107,   15,
    0,    0,    0,    0, 1356,   20,    0,   -4,    0,    0,
    0,    0,    0,    0,    0,    0,    0,
};
#define YYTABLESIZE 1502
short yytable[] = {                                      76,
   38,   42,   73,   44,   45,   51,   73,   71,   69,   49,
   70,   71,   72,   50,   73,   50,   72,  133,   24,   71,
   69,   24,   70,   80,   72,  117,   76,   41,  118,   75,
   76,   76,   76,   76,   76,  148,   76,   68,   83,   67,
   57,   58,  124,   28,  156,  125,   52,   53,   76,   76,
   80,   76,  152,   67,   80,   80,   80,   80,   80,   74,
   80,   54,   76,   74,   51,   22,  116,   23,   22,   79,
   23,   74,   80,   80,   28,   80,   25,   28,   20,   25,
   67,   20,   76,   76,   67,   67,   67,   67,   67,   76,
   67,   52,   28,   76,   76,   76,   76,   76,   50,   76,
   76,   77,   67,   67,   80,   67,   80,   80,   81,   78,
  110,   76,   76,  131,   76,  136,   74,   53,   52,  123,
  137,  141,   52,   52,   52,   52,   52,  143,   52,   50,
  146,   55,   50,   16,   17,   56,   67,   67,    1,   46,
   52,   52,  153,   52,   53,   76,   76,   50,   53,   53,
   53,   53,   53,   54,   53,   47,  149,  130,  150,    0,
    0,    0,   51,    0,    0,    0,   53,   53,    0,   53,
   26,    0,    0,    0,   52,    0,    0,   11,    0,    0,
   54,   50,   25,   24,   54,   54,   54,   54,   54,   51,
   54,   55,    0,   51,   51,   51,   51,   51,    0,   51,
   53,    0,   54,   54,    0,   54,    0,    0,    0,    0,
   73,   51,   51,   56,   51,   71,   69,    0,   70,    0,
   72,   59,   55,   63,   55,   55,   55,   64,    0,    0,
  129,    0,   23,   68,    0,   67,   54,    0,    0,    0,
   55,   55,    0,   55,   56,   51,   56,   56,   56,   57,
    0,   58,   59,    0,   63,   59,    0,   63,   64,    0,
    0,   64,   56,   56,    0,   56,    0,   74,    0,    0,
   59,   59,   63,   59,   55,    0,   64,   62,   63,    0,
   57,    0,   58,   57,    0,   58,    0,   60,   76,   76,
   76,   76,   76,   76,    0,    0,   56,    0,   57,    0,
   58,   76,   76,   76,   59,    0,   63,    0,    0,    0,
   64,    0,   80,   80,   80,   80,   80,   80,   60,    0,
    0,   60,    0,    0,    0,   80,   80,   80,    0,    0,
    0,    0,   57,    0,   58,    0,   60,   60,    0,   60,
   61,    0,   67,   67,   67,   67,    0,    0,    0,    0,
    0,   76,   76,   76,   76,    0,   67,   67,    0,    0,
    0,    0,    0,    0,    0,   76,   76,   62,    0,    0,
   60,   61,    0,    0,   61,    0,    0,    0,    0,    0,
   52,   52,   52,   52,    0,    0,    0,    0,    0,   61,
   61,    0,   61,    0,   52,   52,    0,    0,   62,    0,
    0,   62,    0,    0,    0,    0,   53,   53,   53,   53,
   29,    0,    0,    0,    0,    0,   62,   62,    0,   62,
   53,   53,    0,   61,    0,    0,    0,    0,   13,   15,
    0,   16,   17,   18,   19,   20,    0,    0,    0,    0,
   21,   22,   54,   54,   54,   54,    0,    0,    0,    0,
   62,   51,   51,   51,   51,   26,   54,   54,   86,    0,
    0,    0,   11,    0,    0,   51,   51,   25,    0,    0,
    0,    0,   61,   62,   63,   64,    0,    0,    0,    0,
   55,   55,   55,   55,   24,    0,   65,   66,    0,    0,
    0,    0,    0,    0,   55,   55,    0,    0,    0,   24,
    0,    0,   56,   56,   56,   56,    0,   26,   14,    0,
   59,   59,   59,   59,   11,    0,   56,   56,    0,   25,
    0,    0,   26,   14,   59,   59,   63,   63,    0,   11,
   64,    0,    0,   23,   25,    0,    0,    0,   57,    0,
   58,   57,    0,   58,  138,  139,    0,    0,   23,    0,
   24,    0,   57,   57,   58,   58,    0,    0,    0,   86,
    0,    0,    0,    0,    0,    0,    0,  158,    0,    0,
   24,    0,    0,   26,   14,    0,   60,   60,   60,   60,
   11,    0,    0,    0,    0,   25,   24,    0,    0,    0,
   60,   60,    0,   26,   14,    0,    0,   12,    0,   23,
   11,    0,    0,    0,    0,   25,    0,    0,    0,   26,
    0,    0,   12,    0,   85,    0,   11,    0,    0,   23,
    0,   25,    0,    0,    0,    0,    0,    0,    0,   61,
   61,   61,   61,    0,    0,   23,    0,    0,    0,    0,
    0,    0,    0,   61,   61,   24,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   62,   62,   62,   62,
    0,    0,    0,   12,    0,    0,   33,    0,   26,   14,
   62,   62,    0,    0,    0,   11,    0,    0,    0,    0,
   25,    0,    0,   12,    0,  154,    0,    0,    0,   33,
   33,    0,    0,    0,   23,    0,   33,    0,    0,   76,
    0,   33,    0,   76,   76,   76,   76,   76,    0,   76,
    0,    0,    0,   13,   15,   33,   16,   17,   18,   19,
   20,    0,   76,    0,   76,   21,   22,    0,    0,    0,
    1,    2,    3,    4,    5,    6,    7,    0,    8,    9,
   10,    0,    0,    0,    0,    1,    0,    3,    4,    5,
    6,    7,    0,    8,    9,   10,   76,    0,   12,    0,
    0,    0,    0,    0,    0,   13,   15,    0,   16,   17,
   18,   19,   20,    0,    0,    0,    0,   21,   22,   33,
   13,   15,    0,   16,   17,   18,   19,   20,    0,    0,
    0,    0,   21,   22,    0,    0,    1,    0,    3,    4,
    5,    6,    7,  143,    8,    9,   10,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    1,    0,    3,    4,
    5,    6,    7,    0,    8,    9,   10,    0,    0,    0,
    0,   13,   15,    0,   16,   17,   18,   19,   20,    0,
    0,    0,    0,   21,   22,    0,    0,    0,    0,    0,
    0,   13,   15,    0,   16,   17,   18,   19,   20,    0,
    0,    0,    0,   21,   22,    0,    0,   13,   15,    0,
   16,   17,   18,   19,   20,    0,    0,    0,    0,   21,
   22,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    1,    0,    3,    4,    5,    6,    7,   26,    8,
    9,   10,    0,    0,    0,   11,    0,    0,    0,    0,
   25,    0,   33,    0,   33,   33,   33,   33,   33,    0,
   33,   33,   33,    0,    0,    0,   13,   15,    0,   16,
   17,   18,   19,   20,    0,    0,    0,    0,   21,   22,
    0,    0,    0,    0,    0,    0,    0,   33,   33,    0,
   33,   33,   33,   33,   33,    0,    0,    0,    0,   33,
   33,   76,   76,   76,   76,   76,   76,    0,    0,    0,
    0,    0,    0,   73,   76,   76,   76,   84,   71,   69,
   73,   70,    0,   72,    0,   71,   69,    0,   70,   73,
   72,    0,    0,  120,   71,   69,   68,   70,   67,   72,
    0,    0,    0,   68,    0,   67,    0,    0,    0,    0,
    0,    0,   68,    0,   67,   73,    0,    0,    0,  121,
   71,   69,    0,   70,    0,   72,    0,    0,    0,    0,
   74,    0,    0,    0,    0,    0,  119,   74,   68,   73,
   67,    0,    0,  122,   71,   69,   74,   70,   73,   72,
    0,    0,  126,   71,   69,   73,   70,    0,   72,  127,
   71,   69,   68,   70,   67,   72,    0,    0,    0,    0,
    0,   68,   74,   67,    0,    0,    0,   73,   68,    0,
   67,  155,   71,   69,   73,   70,    0,   72,    0,   71,
   69,    0,   70,   73,   72,    0,   74,    0,   71,   69,
   68,   70,   67,   72,    0,   74,    0,   68,    0,   67,
    0,    0,   74,    0,   73,    0,   68,    0,   67,   71,
   69,    0,   70,    0,   72,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   74,    0,    0,   68,    0,   67,
    0,   74,    0,    0,    0,    0,    0,    0,    0,    0,
   74,    0,    0,    0,    0,    0,   13,   87,    0,   16,
   17,   18,   19,   20,    0,    0,    0,    0,   21,   22,
    0,   74,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   61,   62,   63,   64,    0,
    0,    0,   61,   62,   63,   64,    0,    0,    0,   65,
   66,   61,   62,   63,   64,    0,   65,   66,    0,    0,
    0,    0,    0,    0,    0,   65,   66,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   61,   62,   63,
   64,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   65,   66,    0,    0,    0,    0,    0,    0,    0,
    0,   61,   62,   63,   64,    0,    0,    0,    0,    0,
   61,   62,   63,   64,    0,   65,   66,   61,   62,   63,
   64,    0,    0,    0,   65,   66,    0,    0,    0,    0,
    0,   65,   66,    0,    0,    0,    0,    0,    0,   61,
   62,   63,   64,    0,    0,    0,   61,   62,   63,   64,
    0,    0,    0,   65,   66,   61,   62,   63,   64,    0,
   65,   66,    0,   43,    0,    0,   46,    0,    0,    0,
   66,    0,    0,    0,    0,    0,   61,   62,   63,   64,
   59,   60,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   90,   91,   92,   93,   94,
    0,    0,    0,    0,    0,    0,   95,   96,   97,   98,
   99,  100,  101,  102,  103,  104,  105,  106,  107,  108,
    0,    0,    0,  109,    0,  113,  114,  115,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   91,    0,   90,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  140,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  151,
};
short yycheck[] = {                                      10,
  292,    6,   37,    8,    9,   91,   37,   42,   43,   14,
   45,   42,   47,   40,   37,   40,   47,  125,   10,   42,
   43,   10,   45,   10,   47,   41,   37,   40,   44,   34,
   41,   42,   43,   44,   45,  143,   47,   60,   43,   62,
   21,   22,   41,   10,  152,   44,   40,   40,   59,   60,
   37,   62,   44,   10,   41,   42,   43,   44,   45,   94,
   47,   40,   10,   94,   91,   41,   91,   59,   44,   40,
   59,   94,   59,   60,   41,   62,   41,   44,   41,   44,
   37,   44,   93,   94,   41,   42,   43,   44,   45,   37,
   47,   10,   59,   41,   42,   43,   44,   45,   10,   47,
  303,  304,   59,   60,   40,   62,   93,   94,   40,  312,
  292,   59,   60,   93,   62,   59,   94,   10,   37,   91,
  123,   10,   41,   42,   43,   44,   45,  263,   47,   41,
   59,  292,   44,  294,  295,  296,   93,   94,    0,   59,
   59,   60,  147,   62,   37,   93,   94,   59,   41,   42,
   43,   44,   45,   10,   47,   59,  144,  118,  144,   -1,
   -1,   -1,   10,   -1,   -1,   -1,   59,   60,   -1,   62,
   33,   -1,   -1,   -1,   93,   -1,   -1,   40,   -1,   -1,
   37,   93,   45,   10,   41,   42,   43,   44,   45,   37,
   47,   10,   -1,   41,   42,   43,   44,   45,   -1,   47,
   93,   -1,   59,   60,   -1,   62,   -1,   -1,   -1,   -1,
   37,   59,   60,   10,   62,   42,   43,   -1,   45,   -1,
   47,   10,   41,   10,   43,   44,   45,   10,   -1,   -1,
   93,   -1,   59,   60,   -1,   62,   93,   -1,   -1,   -1,
   59,   60,   -1,   62,   41,   93,   43,   44,   45,   10,
   -1,   10,   41,   -1,   41,   44,   -1,   44,   41,   -1,
   -1,   44,   59,   60,   -1,   62,   -1,   94,   -1,   -1,
   59,   60,   59,   62,   93,   -1,   59,  300,  301,   -1,
   41,   -1,   41,   44,   -1,   44,   -1,   10,  299,  300,
  301,  302,  303,  304,   -1,   -1,   93,   -1,   59,   -1,
   59,  312,  313,  314,   93,   -1,   93,   -1,   -1,   -1,
   93,   -1,  299,  300,  301,  302,  303,  304,   41,   -1,
   -1,   44,   -1,   -1,   -1,  312,  313,  314,   -1,   -1,
   -1,   -1,   93,   -1,   93,   -1,   59,   60,   -1,   62,
   10,   -1,  299,  300,  301,  302,   -1,   -1,   -1,   -1,
   -1,  299,  300,  301,  302,   -1,  313,  314,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  313,  314,   10,   -1,   -1,
   93,   41,   -1,   -1,   44,   -1,   -1,   -1,   -1,   -1,
  299,  300,  301,  302,   -1,   -1,   -1,   -1,   -1,   59,
   60,   -1,   62,   -1,  313,  314,   -1,   -1,   41,   -1,
   -1,   44,   -1,   -1,   -1,   -1,  299,  300,  301,  302,
    0,   -1,   -1,   -1,   -1,   -1,   59,   60,   -1,   62,
  313,  314,   -1,   93,   -1,   -1,   -1,   -1,  291,  292,
   -1,  294,  295,  296,  297,  298,   -1,   -1,   -1,   -1,
  303,  304,  299,  300,  301,  302,   -1,   -1,   -1,   -1,
   93,  299,  300,  301,  302,   33,  313,  314,   48,   -1,
   -1,   -1,   40,   -1,   -1,  313,  314,   45,   -1,   -1,
   -1,   -1,  299,  300,  301,  302,   -1,   -1,   -1,   -1,
  299,  300,  301,  302,   10,   -1,  313,  314,   -1,   -1,
   -1,   -1,   -1,   -1,  313,  314,   -1,   -1,   -1,   10,
   -1,   -1,  299,  300,  301,  302,   -1,   33,   34,   -1,
  299,  300,  301,  302,   40,   -1,  313,  314,   -1,   45,
   -1,   -1,   33,   34,  313,  314,  313,  314,   -1,   40,
  313,   -1,   -1,   59,   45,   -1,   -1,   -1,  299,   -1,
  299,  302,   -1,  302,  134,  135,   -1,   -1,   59,   -1,
   10,   -1,  313,  314,  313,  314,   -1,   -1,   -1,  149,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  157,   -1,   -1,
   10,   -1,   -1,   33,   34,   -1,  299,  300,  301,  302,
   40,   -1,   -1,   -1,   -1,   45,   10,   -1,   -1,   -1,
  313,  314,   -1,   33,   34,   -1,   -1,  123,   -1,   59,
   40,   -1,   -1,   -1,   -1,   45,   -1,   -1,   -1,   33,
   -1,   -1,  123,   -1,  125,   -1,   40,   -1,   -1,   59,
   -1,   45,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  299,
  300,  301,  302,   -1,   -1,   59,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  313,  314,   10,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  299,  300,  301,  302,
   -1,   -1,   -1,  123,   -1,   -1,   10,   -1,   33,   34,
  313,  314,   -1,   -1,   -1,   40,   -1,   -1,   -1,   -1,
   45,   -1,   -1,  123,   -1,  125,   -1,   -1,   -1,   33,
   34,   -1,   -1,   -1,   59,   -1,   40,   -1,   -1,   37,
   -1,   45,   -1,   41,   42,   43,   44,   45,   -1,   47,
   -1,   -1,   -1,  291,  292,   59,  294,  295,  296,  297,
  298,   -1,   60,   -1,   62,  303,  304,   -1,   -1,   -1,
  256,  257,  258,  259,  260,  261,  262,   -1,  264,  265,
  266,   -1,   -1,   -1,   -1,  256,   -1,  258,  259,  260,
  261,  262,   -1,  264,  265,  266,   94,   -1,  123,   -1,
   -1,   -1,   -1,   -1,   -1,  291,  292,   -1,  294,  295,
  296,  297,  298,   -1,   -1,   -1,   -1,  303,  304,  123,
  291,  292,   -1,  294,  295,  296,  297,  298,   -1,   -1,
   -1,   -1,  303,  304,   -1,   -1,  256,   -1,  258,  259,
  260,  261,  262,  263,  264,  265,  266,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,  258,  259,
  260,  261,  262,   -1,  264,  265,  266,   -1,   -1,   -1,
   -1,  291,  292,   -1,  294,  295,  296,  297,  298,   -1,
   -1,   -1,   -1,  303,  304,   -1,   -1,   -1,   -1,   -1,
   -1,  291,  292,   -1,  294,  295,  296,  297,  298,   -1,
   -1,   -1,   -1,  303,  304,   -1,   -1,  291,  292,   -1,
  294,  295,  296,  297,  298,   -1,   -1,   -1,   -1,  303,
  304,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  256,   -1,  258,  259,  260,  261,  262,   33,  264,
  265,  266,   -1,   -1,   -1,   40,   -1,   -1,   -1,   -1,
   45,   -1,  256,   -1,  258,  259,  260,  261,  262,   -1,
  264,  265,  266,   -1,   -1,   -1,  291,  292,   -1,  294,
  295,  296,  297,  298,   -1,   -1,   -1,   -1,  303,  304,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  291,  292,   -1,
  294,  295,  296,  297,  298,   -1,   -1,   -1,   -1,  303,
  304,  299,  300,  301,  302,  303,  304,   -1,   -1,   -1,
   -1,   -1,   -1,   37,  312,  313,  314,   41,   42,   43,
   37,   45,   -1,   47,   -1,   42,   43,   -1,   45,   37,
   47,   -1,   -1,   41,   42,   43,   60,   45,   62,   47,
   -1,   -1,   -1,   60,   -1,   62,   -1,   -1,   -1,   -1,
   -1,   -1,   60,   -1,   62,   37,   -1,   -1,   -1,   41,
   42,   43,   -1,   45,   -1,   47,   -1,   -1,   -1,   -1,
   94,   -1,   -1,   -1,   -1,   -1,   93,   94,   60,   37,
   62,   -1,   -1,   41,   42,   43,   94,   45,   37,   47,
   -1,   -1,   41,   42,   43,   37,   45,   -1,   47,   41,
   42,   43,   60,   45,   62,   47,   -1,   -1,   -1,   -1,
   -1,   60,   94,   62,   -1,   -1,   -1,   37,   60,   -1,
   62,   41,   42,   43,   37,   45,   -1,   47,   -1,   42,
   43,   -1,   45,   37,   47,   -1,   94,   -1,   42,   43,
   60,   45,   62,   47,   -1,   94,   -1,   60,   -1,   62,
   -1,   -1,   94,   -1,   37,   -1,   60,   -1,   62,   42,
   43,   -1,   45,   -1,   47,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   94,   -1,   -1,   60,   -1,   62,
   -1,   94,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   94,   -1,   -1,   -1,   -1,   -1,  291,  292,   -1,  294,
  295,  296,  297,  298,   -1,   -1,   -1,   -1,  303,  304,
   -1,   94,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  299,  300,  301,  302,   -1,
   -1,   -1,  299,  300,  301,  302,   -1,   -1,   -1,  313,
  314,  299,  300,  301,  302,   -1,  313,  314,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  313,  314,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  299,  300,  301,
  302,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  313,  314,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  299,  300,  301,  302,   -1,   -1,   -1,   -1,   -1,
  299,  300,  301,  302,   -1,  313,  314,  299,  300,  301,
  302,   -1,   -1,   -1,  313,  314,   -1,   -1,   -1,   -1,
   -1,  313,  314,   -1,   -1,   -1,   -1,   -1,   -1,  299,
  300,  301,  302,   -1,   -1,   -1,  299,  300,  301,  302,
   -1,   -1,   -1,  313,  314,  299,  300,  301,  302,   -1,
  313,  314,   -1,    8,   -1,   -1,   11,   -1,   -1,   -1,
  314,   -1,   -1,   -1,   -1,   -1,  299,  300,  301,  302,
   25,   26,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   50,   51,   52,   53,   54,
   -1,   -1,   -1,   -1,   -1,   -1,   61,   62,   63,   64,
   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,
   -1,   -1,   -1,   78,   -1,   80,   81,   82,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  116,   -1,  118,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  136,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  146,
};
#define YYFINAL 27
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 314
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'!'","'\"'",0,0,"'%'",0,0,"'('","')'","'*'","'+'",
"','","'-'",0,"'/'",0,0,0,0,0,0,0,0,0,0,0,"';'","'<'",0,"'>'",0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'['",0,"']'","'^'",0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"DEFINE",
"IF","WHILE","FOR","BREAK","QUIT","AUTO","RETURN","CONTINUE","LEX_ERROR",
"PUSHL","PUSHR","DISCARD","SAVEV","RESTV","POPV","JMP","JMPZ","JMPNZ","CALL",
"PRINT","SAVEVAR","RESTVAR","SAVEARRAY","RESTARRAY","INDEX","PUSHVAR",
"PUSHCONST","PUSHARRAY","PUSHSPECIAL","POPVAR","POPARRAY","REPLACE","CLRSTK",
"CONST","IDENT","PRINT_LIST","IBASE","OBASE","SCALE","SQRT","LENGTH","EQ","GE",
"LE","NE","PLUSPLUS","MINUSMINUS","ASG_PLUS","ASG_MINUS","ASG_STAR","ASG_DIV",
"ASG_MOD","ASG_EXP","UMINUS","ASSIGN","LOR","LAND",
};
char *yyrule[] = {
"$accept : prog",
"prog :",
"prog : stmt",
"prog : function",
"stmt : expr stmt_sep",
"stmt : '\"' stmt_sep",
"stmt : compound_stmt",
"stmt : if_stmt",
"stmt : while_stmt",
"stmt : for_stmt",
"stmt : BREAK stmt_sep",
"stmt : CONTINUE stmt_sep",
"stmt : RETURN expr stmt_sep",
"stmt : RETURN stmt_sep",
"stmt : QUIT",
"stmt : stmt_sep",
"stmt : LEX_ERROR",
"stmt : error",
"$$1 :",
"function : DEFINE IDENT '(' formal_list ')' $$1 '{' '\\n' vardecl stmt_list '}'",
"actual_parm : expr",
"actual_parm : IDENT '[' ']'",
"actual_list :",
"actual_list : actual_parm",
"actual_list : actual_list ',' actual_parm",
"formal_list :",
"formal_list : formal",
"formal_list : formal_list ',' formal",
"formal : IDENT",
"formal : IDENT '[' ']'",
"auto_vlist : formal",
"auto_vlist : auto_vlist ',' formal",
"auto_stmt : AUTO auto_vlist stmt_sep",
"vardecl :",
"vardecl : auto_stmt",
"vardecl : vardecl auto_stmt",
"compound_stmt : '{' stmt_list '}'",
"stmt_list : stmt",
"stmt_list : stmt_list stmt",
"$$2 :",
"$$3 :",
"if_stmt : IF $$2 '(' expr ')' $$3 stmt",
"$$4 :",
"$$5 :",
"while_stmt : WHILE $$4 '(' expr ')' $$5 stmt",
"$$6 :",
"$$7 :",
"$$8 :",
"$$9 :",
"for_stmt : FOR '(' $$6 expr $$7 ';' expr $$8 ';' expr ')' $$9 stmt",
"expr : nexpr ASSIGN expr",
"expr : expr '^' expr",
"expr : expr '*' expr",
"expr : expr '/' expr",
"expr : expr '%' expr",
"expr : expr '+' expr",
"expr : expr '-' expr",
"expr : expr EQ expr",
"expr : expr NE expr",
"expr : expr GE expr",
"expr : expr LE expr",
"expr : expr '>' expr",
"expr : expr '<' expr",
"expr : expr LAND expr",
"expr : expr LOR expr",
"expr : '!' expr",
"expr : '-' expr",
"expr : nexpr",
"expr : CONST",
"expr : '(' expr ')'",
"expr : PLUSPLUS nexpr",
"expr : MINUSMINUS nexpr",
"expr : nexpr PLUSPLUS",
"expr : nexpr MINUSMINUS",
"expr : IDENT '(' actual_list ')'",
"expr : builtins",
"nexpr : IDENT",
"nexpr : IDENT '[' expr ']'",
"nexpr : IBASE",
"nexpr : OBASE",
"nexpr : SCALE",
"builtins : SQRT '(' expr ')'",
"builtins : SCALE '(' expr ')'",
"builtins : LENGTH '(' expr ')'",
"stmt_sep : '\\n'",
"stmt_sep : ';'",
};
#endif
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#ifndef YYSTACKSIZE
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 300
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
#define yystacksize YYSTACKSIZE
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#line 458 "parse.y"



/*
 * is_loop(), and is_func() are predicates which scan the current stack for
 * nested structures. is_loop() returns 1 if there is a FOR or WHILE loop in
 * in the stack. is_func() returns 1 if there is a FUNCTION DEFINITION in the
 * stack ( note: actually must be at the "bottom" of stack. base_type looks
 * from the bottom of the "compile" stack up to see what the lowest level
 * control structure is.
 */



static 
int is_loop()
{
	int             i;
	for (i = icodeptr - 1; i >= 0; i--) {
		if (code_stack[i]->ityp == WHILE || code_stack[i]->ityp == FOR)
			return 1;
	}
	return 0;
}

static 
int is_func()
{
	int             i;
	for (i = icodeptr - 1; i >= 0; i--)
		if (code_stack[i]->ityp == DEFINE)
			return 1;
	return 0;
}

static 
int base_type()
{
	int             i;
	for (i = 0; i < icodeptr; i++) {
		switch (code_stack[i]->ityp) {
		case DEFINE:
		case FOR:
		case WHILE:
		case IF:
			return code_stack[i]->ityp;
		}
	}
	return 0;
}



static struct icode *
newsect(blk_type)
	int             blk_type;
{
	struct icode   *new;
	register int    i;

	if ((new = ALLOCMEM(struct icode, 1)) == NULL)
		NO_CORE("new_sect");
	new->ityp = blk_type;
	new->cur_offs = 0;
	new->ilen = MIN_CODE_ALLOC;
	for (i = 0; i < NLABELS; i++)
		new->label[i] = NIL_LABEL;
	new->lbl_flags = 0;
	if ((new->code = ALLOCMEM(int, new->ilen)) == NULL) {
		FREEMEM(new);
		NO_CORE("new_sect");
	}
	return push(new);
}


static 
int endsect()
{
	struct icode   *p0, *p1;

	if ((p1 = stack_top()) == NULL) {
		error(0, "no section to POP\n");
		return -1;
	}
	switch (p1->ityp) {
	case DEFINE:
		break;
	case IF:
		make_label(LABEL_IFEXIT);
		break;
	case WHILE:
	case FOR:
		add_jmp(JMP, LABEL_LOOP);
		make_label(LABEL_EXIT);
		break;
	default:
		;
	}
	p1 = pop();
	if ((p0 = stack_top()) != NULL) {
		join_section(p0, p1);
		return 1;
	}
	trim_code(p1, p1->cur_offs);
	(void) push(p1);
	return 0;
}


static void 
trim_code(p, size)
	struct icode   *p;
	int             size;
{
	if ((p->code = MEMCHSIZE(p->code, (p->ilen = size) * sizeof(int))) == NULL)
		NO_CORE("trim_code");
}


static 
void more_code(p, delta)
	struct icode   *p;
	int             delta;
{
	if ((p->code = MEMCHSIZE(p->code, (p->ilen += delta) * sizeof(int))) == NULL)
		NO_CORE("more_code");
}
#include <stdarg.h>

int emit(struct icode * p, int n,...)
{
	va_list         va;
	int             i;
	if (!p) {
		if ((p = stack_top()) == 0) {
			if ((p = newsect(IMMED)) == 0) {
				error(1, "parser: out of stack space\n");
			}
		}
	}
	if (code_space(p) < n) {
		more_code(p, MIN_CODE_ALLOC);
	}
	va_start(va, n);
	for (i = 0; i < n; i++) {
		p->code[p->cur_offs++] = va_arg(va, int);
	}
	va_end(va);
	return 0;
}

/*
 * These are both kind of kludgy.	The problem is that constants must be
 * "transformed" from strings into numbers at this stage, and assigned to
 * some form of literal pool.
 * 
 * add_const: if (there is an enclosing function) const->type = _PERMANENT. else
 * if (there is any enclosing structure) const->type = _TEMPORARY. else
 * const->type = _SCRATCH. Note: Currently there is no distinction between
 * _PERMANENT and _TEMPORARY.	A garbage collection function will have to be
 * added. (ie.	for(i=1231; i< 2422; i++) ....). The constant (2422) cannot
 * be deleted after it is used, since it is required for the next iteration.
 */

static 
int add_const(char *s)
{
	numb_t         *n;
	char           *t;
	int             inbase;
	char           *ytext = s;

	if ((*ytext == '0') && (ytext[1] == 'x' || ytext[1] == 'X')) {
		inbase = 16;
		ytext += 2;
	} else {
		inbase = get_ibase();
	}
	n = strtonum(ytext, &t, inbase);
	emit(0, 1 + sizeof(void *) / sizeof(int), PUSHCONST,
	     is_func() ? make_perm(n) : make_temp(n));
	free(s);
	return 0;
}

static 
void add_zeroval()
{
	extern void    *constant_0;
	emit(0, 1 + sizeof(void *) / sizeof(int), PUSHCONST, constant_0);
}



/*
 * jumps and labels: This bears some explanation: 1.	A label may be jumped
 * to before it is defined. When This occurs, the label is marked "used", and
 * the contents of the label field is a "relative linked list" of all
 * occurances of the label.
 * 
 * 2.	When a label is defined, it may have already been used. If so, one
 * must traverse the list, replacing the links with the relative location of
 * the label.
 * 
 * 3.	A jump may be made to a label which lies in another block (ie. a
 * BREAK in an IF nested in a FOR ).	When the block is complete, the
 * "used" lists may be joined, and the "defined" lists may be resolved.
 * 
 * Since BC is technincally an REC grammer, there are a fixed number of possible
 * labels available at any block ( Basically corresponding to "BREAK",
 * "CONTINUE", "RETURN", "IF-FAIL", "FOR-1", "FOR-2"). Thus a small
 * label-table is maintained in the code-gen structure, and the algorithms
 * presented will only work for this style of grammer.
 * 
 * add_jump:	first check to see if the target has been defined. if yes,
 * then simply insert the relative address of the target. if no, then insert
 * this at the head of the list by placing into the target the difference of
 * the current head (or NIL_LABEL if appropriate), and update the head to
 * reference here.
 * 
 * add_label:	first check to see if the label has been used. if yes, then
 * traverse the list, starting at the head and moving by the offset of each
 * target.	at each point in the list, generate a reference from the
 * current position. if no, simply insert the current position in the head.
 * 
 * join_sections:	for each label used in the lessor block switch (corresponding
 * label in greater block) DEFINED: resolve list in lessor for given label.
 * USED: find end of list in greater. link last element in greater to first
 * in lessor. NIL: mark as used, and link to lessor.
 * 
 * 
 * Notice that the NIL_LABEL is defined as -1.	0 was not an appropriate
 * choice.
 * 
 * Note:	bug in join_label, whereby the following statement: while (1) { if
 * (1)	break if (1)	break if (1)	break } would mess up the lists.
 * fixed.
 * 
 * 
 */

static 
int add_jmp(cond, label)
	int             cond, label;
{
	struct icode   *top;
	int             where;
	int             prev;

	if ((top = stack_top()) == NULL) {
#ifdef	DEBUGGING
		error(0, "invalid jump location\n");
#endif
		return 0;
	}
	emit(top, 1, cond);
	where = top->cur_offs;
	if (is_defined(top, label)) {
		/* make relative */
		emit(top, 1, top->label[label] - where);
	} else {
		/* link into list */
		do_reference(top, label);
		if ((prev = top->label[label]) == NIL_LABEL) {
			emit(top, 1, NIL_LABEL);
		} else {
			emit(top, 1, prev - where);
		}
		top->label[label] = where;
	}
	return 1;
}


static 
void make_label(lbl)
	int             lbl;
{
	struct icode   *top;
	register int    cur, next;

	if ((top = stack_top()) == NULL) {
#ifdef DEBUGGING
		error(0, "invalid label \n");
#endif
	}
	if (is_used(top, lbl)) {
		if ((cur = top->label[lbl]) == NIL_LABEL) {
			error(1, "usage list with NIL_LABEL\n");
		}
		while (1) {
			next = top->code[cur];
			top->code[cur] = top->cur_offs - cur;	/* generate the label */
			if (next == NIL_LABEL)
				break;
			cur += next;
		}
	}
	top->label[lbl] = top->cur_offs;	/* place the label */
	do_define(top, lbl);
}

/*
 * This impliments the back-patching logic to concatenate to code sections.
 * 
 */

static 
void join_label(to, from, label)
	struct icode   *to, *from;
	int             label;
{
	int             old_offs = to->cur_offs - from->cur_offs;
	int             cur, next, prev;


	if (is_used(from, label)) {	/* if it is used */
		if (is_defined(to, label)) {	/* and already defined */
			/*
			 * this "fixes up" the lessor block, by patching in
			 * the defined label.
			 */
			for (cur = from->label[label] + old_offs; cur != NIL_LABEL; cur += next) {
				next = to->code[cur];
				to->code[cur] = to->label[label] - cur;
				if (next == NIL_LABEL)
					break;
			}
		} else if (is_used(to, label)) {
			/*
			 * this just concatenates the two lists, arbitrarily,
			 * the greater block goes first (easier to follow
			 * debug statements.)
			 */
			prev = NIL_LABEL;
			cur = to->label[label];
			while (to->code[prev = cur] != NIL_LABEL)
				cur += to->code[prev];

			if (prev == NIL_LABEL) {
				error(1, "corrupt label-table\n");
			}
			to->code[prev] = (from->label[label] + old_offs) - prev;
		} else {
			do_reference(to, label);
			to->label[label] = from->label[label] + old_offs;
		}
	}
}

static 
void join_section(to, from)
	struct icode   *to, *from;
{
	int             i;

	if (from->cur_offs > to->ilen - to->cur_offs) {
		more_code(to, max(MIN_CODE_ALLOC, from->cur_offs));
	}
	/* concatenate the code. */
	memcpy(to->code + to->cur_offs, from->code, from->cur_offs * sizeof(int));

	to->cur_offs += from->cur_offs;

	/* Now, go and fix all labels not resolved. */

	for (i = 0; i < NLABELS; i++) {
		join_label(to, from, i);

	}
	FREEMEM(from->code);
	FREEMEM(from);
	return;
}

int do_bc()
{
	struct icode   *p = NULL;
	char           *msg = NULL;
	static int      debug = 0;
	int             t;
	int             i;
	/* init variables */
	local_count = formal_count = 0;

	t = yyparse();
	if (trace) {
		fprintf(stderr, "yyparse() = %d\n", t);
	}
	switch (t) {
	case 0:		/* end-of-file from yyparse */
		return -1;
		break;
	case _IS_DEBUG:
		debug = !debug;
		return 0;
		break;
	case _IS_FUNC:
		if ((p = pop()) == NULL) {
			error(0, "func is NULL\n");
			return 0;
		}
		if (add_func(p->label[LABEL_FUNC], p->code, p->cur_offs) <= 0) {
			error(0, "cannot define function 0x%04.4x \n",
			      p->label[LABEL_FUNC]);
			FREEMEM(p->code);
		}
		if (trace) {
			fprintf(stderr, "installing function '%c'\n",
				p->label[LABEL_FUNC] + 'a');
		}
		FREEMEM(p);
		return 0;
		break;
	case _IS_STMT:
		if ((p = pop()) == NULL)
			return 0;
		execute(p->code, p->cur_offs);
		FREEMEM(p->code);
		FREEMEM(p);
		collect_garbage();
		return 1;
		break;
	case _IS_EMPTY:
		return 0;
		break;
	case _IS_ERROR:
		switch (base_type()) {
		case FOR:
			msg = "for loop ";
			break;
		case WHILE:
			msg = "while loop ";
			break;
		case DEFINE:
			msg = "function ";
			break;
		default:
			msg = "statement";
		}
		error(0, "syntax error: %s deleted\n", msg);
		for (i = 0; i < stktop(); i++) {
			p = stack(i);
			FREEMEM(p->code);
			FREEMEM(p);
		}
		clearstk();
		break;
	default:
		FREEMEM(p->code);
		FREEMEM(p);
		return 0;
	}
	return 0;
}
#line 1203 "y.tab.c"
#define YYABORT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if ((yys = getenv("YYDEBUG")))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, reading %d (%s)\n", yystate,
                    yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: state %d, shifting to state %d\n",
                    yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
    goto yynewerror;
yynewerror:
    yyerror("syntax error");
    goto yyerrlab;
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: state %d, error recovery shifting\
 to state %d\n", *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: error recovery discarding state %d\n",
                            *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, error recovery discards token %d (%s)\n",
                    yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("yydebug: state %d, reducing by rule %d (%s)\n",
                yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 202 "parse.y"
{ yyval.i_val  = _IS_EMPTY; }
break;
case 2:
#line 203 "parse.y"
{ return yyvsp[0].i_val ; }
break;
case 3:
#line 204 "parse.y"
{ return yyvsp[0].i_val ; }
break;
case 4:
#line 210 "parse.y"
{
			if ((cur=stack_top()) != NULL) {
				emit(cur,1,(yyvsp[-1].i_val  & _HAS_ASSIGN) ? DISCARD : PRINT);
				if (cur->ityp == IMMED)
					endsect();
			}
			yyval.i_val  = _IS_STMT;	
		}
break;
case 5:
#line 219 "parse.y"
{	
			if (yyvsp[-1].s_val ) {
				emit(0,1+sizeof(yyvsp[-1].s_val )/sizeof(int),'\"',yyvsp[-1].s_val );
			} else	{
				error(1,"space exhausted\n");
			}
			if ((cur=stack_top()) != NULL && cur->ityp == IMMED)
				endsect();
			yyval.i_val  = _IS_STMT;
		}
break;
case 6:
#line 229 "parse.y"
{ yyval.i_val  = _IS_STMT; }
break;
case 7:
#line 230 "parse.y"
{ yyval.i_val  = _IS_STMT; }
break;
case 8:
#line 231 "parse.y"
{ yyval.i_val  = _IS_STMT; }
break;
case 9:
#line 232 "parse.y"
{ yyval.i_val  = _IS_STMT; }
break;
case 10:
#line 234 "parse.y"
{	
			if (is_loop()) {
				add_jmp(JMP, LABEL_EXIT);
				yyval.i_val  = _IS_STMT;
			} else	{
				parse_error("Must be in while or for loop\n");
				yyval.i_val  = _IS_ERROR;
			}
		}
break;
case 11:
#line 245 "parse.y"
{	
			if (is_loop()) {
				add_jmp(JMP, LABEL_LOOP);
				yyval.i_val  = _IS_STMT;
			} else	{
				parse_error("Must be in while or for loop\n");
				yyval.i_val  = _IS_ERROR;
			}
		}
break;
case 12:
#line 255 "parse.y"
{
			if (is_func()) {
				emit(0,1,REPLACE);
				add_jmp(JMP,LABEL_RETURN);
				yyval.i_val  = _IS_STMT;
			} else	{
				parse_error("Must be in function\n");
				yyval.i_val  = _IS_ERROR;
			}
		}
break;
case 13:
#line 266 "parse.y"
{
			add_zeroval();
			if (is_func()) {
				add_jmp(JMP,LABEL_RETURN);
				yyval.i_val  = _IS_STMT;
			} else	{
				parse_error("Must be in function\n");
				yyval.i_val  = _IS_ERROR;
			}
		}
break;
case 14:
#line 276 "parse.y"
{ program_exit(); }
break;
case 15:
#line 277 "parse.y"
{ yyval.i_val  = _IS_STMT;	}
break;
case 16:
#line 278 "parse.y"
{ yyval.i_val  = _IS_ERROR; }
break;
case 17:
#line 279 "parse.y"
{ yyerror("syntax error");yyval.i_val  = _IS_ERROR; }
break;
case 18:
#line 284 "parse.y"
{
				if ((cur = newsect(DEFINE)) != NULL) {
					int		i;
					if (code_space(cur) < formal_count * 4)
						more_code(cur,max(formal_count*4,MIN_CODE_ALLOC));
						for (i=0; i < formal_count; i++) {
						emit(cur,2,formal[i] & IS_ARRAY ? SAVEARRAY :SAVEVAR,
							formal[i]>>1);
					}
					while (--i >= 0) {
						emit(cur,2,formal[i] & IS_ARRAY ? POPARRAY:POPVAR,
							formal[i] >> 1);
					}		
					cur->label[LABEL_FUNC] = yyvsp[-3].i_val ;
				} else	{
					NO_CORE("yyparse");
				}
			 }
break;
case 19:
#line 306 "parse.y"
{
			int	i;
				if ((cur = stack_top()) == NULL) {
					return _IS_ERROR;
				}
				/* generate a return zero if fall off end */
				add_zeroval();	
				make_label(LABEL_RETURN);
				/* restore variables */
				for (i=local_count; --i >= 0;) {
					emit(cur,2,local[i]&IS_ARRAY ? RESTARRAY : RESTVAR,
								local[i]>>1);
				}
				for (i=formal_count; --i >= 0;) {
					emit(cur,2,formal[i] & IS_ARRAY ? RESTARRAY : RESTVAR,
								formal[i]>>1);
				}
				formal_count = local_base = local_count = 0;
				emit(cur,1,RETURN);
				endsect();
				yyval.i_val  = _IS_FUNC;
			 }
break;
case 20:
#line 333 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
case 21:
#line 334 "parse.y"
{ emit(0,2,PUSHARRAY,yyvsp[-2].i_val ); }
break;
case 22:
#line 338 "parse.y"
{ yyval.i_val  = 0; }
break;
case 23:
#line 339 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
case 24:
#line 340 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
case 25:
#line 345 "parse.y"
{ yyval.i_val  = 0; }
break;
case 26:
#line 346 "parse.y"
{ (void) ADD_FORMAL(yyvsp[0].i_val ); }
break;
case 27:
#line 347 "parse.y"
{ (void) ADD_FORMAL(yyvsp[0].i_val );	}
break;
case 28:
#line 351 "parse.y"
{	yyval.i_val  = yyvsp[0].i_val  << 1;	}
break;
case 29:
#line 352 "parse.y"
{	yyval.i_val  = (yyvsp[-2].i_val  << 1)| IS_ARRAY;	}
break;
case 30:
#line 357 "parse.y"
{ (void) ADD_LOCAL(yyvsp[0].i_val );	}
break;
case 31:
#line 358 "parse.y"
{ (void) ADD_LOCAL(yyvsp[0].i_val );	}
break;
case 32:
#line 363 "parse.y"
{			
			for (;local_base < local_count; local_base++) {
				emit(0,2,local[local_base] & IS_ARRAY ? SAVEARRAY:SAVEVAR,
							local[local_base] >> 1);
			}
		}
break;
case 33:
#line 371 "parse.y"
{ yyval.i_val  = 0; }
break;
case 34:
#line 372 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
case 35:
#line 373 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
case 36:
#line 377 "parse.y"
{ yyval.i_val  = yyvsp[-1].i_val ; }
break;
case 37:
#line 381 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
case 38:
#line 382 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
case 39:
#line 386 "parse.y"
{ newsect(yyvsp[0].i_val ); }
break;
case 40:
#line 387 "parse.y"
{ add_jmp(JMPZ,LABEL_IFEXIT); }
break;
case 41:
#line 388 "parse.y"
{ endsect(); }
break;
case 42:
#line 390 "parse.y"
{ newsect(yyvsp[0].i_val ); make_label(LABEL_LOOP); }
break;
case 43:
#line 391 "parse.y"
{ add_jmp(JMPZ,LABEL_EXIT); }
break;
case 44:
#line 392 "parse.y"
{ endsect(); }
break;
case 45:
#line 394 "parse.y"
{ newsect(yyvsp[-1].i_val ); }
break;
case 46:
#line 395 "parse.y"
{ emit(0,1,DISCARD);
							make_label(LABEL_FOR2); }
break;
case 47:
#line 397 "parse.y"
{ add_jmp(JMPZ,LABEL_EXIT);
							add_jmp(JMP,LABEL_FOR1);
							make_label(LABEL_LOOP); }
break;
case 48:
#line 400 "parse.y"
{ emit(0,1,DISCARD);
							add_jmp(JMP,LABEL_FOR2);
							make_label(LABEL_FOR1); }
break;
case 49:
#line 403 "parse.y"
{ endsect(); }
break;
case 50:
#line 410 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val  | _HAS_ASSIGN; }
break;
case 51:
#line 411 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 52:
#line 412 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 53:
#line 413 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 54:
#line 414 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 55:
#line 415 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 56:
#line 416 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 57:
#line 417 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 58:
#line 418 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 59:
#line 419 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 60:
#line 420 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 61:
#line 421 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 62:
#line 422 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 63:
#line 423 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 64:
#line 424 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-2].i_val  | yyvsp[0].i_val ; }
break;
case 65:
#line 425 "parse.y"
{ emit(0,1,yyvsp[-1].i_val ); yyval.i_val  = yyvsp[-1].i_val  | yyvsp[0].i_val ; }
break;
case 66:
#line 426 "parse.y"
{ emit(0,1,UMINUS); yyval.i_val  = yyvsp[0].i_val ; }
break;
case 67:
#line 427 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
case 68:
#line 428 "parse.y"
{ add_const(yyvsp[0].s_val ); yyval.i_val  = 0;}
break;
case 69:
#line 429 "parse.y"
{ yyval.i_val  = yyvsp[-1].i_val ; }
break;
case 70:
#line 430 "parse.y"
{ emit(0,1,yyvsp[-1].i_val |PREFIX); yyval.i_val  = yyvsp[0].i_val ; }
break;
case 71:
#line 431 "parse.y"
{ emit(0,1,yyvsp[-1].i_val |PREFIX); yyval.i_val  = yyvsp[0].i_val ; }
break;
case 72:
#line 432 "parse.y"
{ emit(0,1,yyvsp[0].i_val |POSTFIX); yyval.i_val  = yyvsp[-1].i_val ;}
break;
case 73:
#line 433 "parse.y"
{ emit(0,1,yyvsp[0].i_val |POSTFIX); yyval.i_val  = yyvsp[-1].i_val ;}
break;
case 74:
#line 434 "parse.y"
{ emit(0,2,CALL,yyvsp[-3].i_val ); yyval.i_val  = 0; }
break;
case 75:
#line 435 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
case 76:
#line 439 "parse.y"
{ emit(0,2,PUSHVAR,yyvsp[0].i_val );yyval.i_val  = 0; }
break;
case 77:
#line 440 "parse.y"
{ emit(0,3,PUSHARRAY,yyvsp[-3].i_val ,INDEX); yyval.i_val  = 0; }
break;
case 78:
#line 441 "parse.y"
{ emit(0,2,PUSHSPECIAL,yyvsp[0].i_val ); yyval.i_val  = 0;}
break;
case 79:
#line 442 "parse.y"
{ emit(0,2,PUSHSPECIAL,yyvsp[0].i_val );yyval.i_val  = 0; }
break;
case 80:
#line 443 "parse.y"
{ emit(0,2,PUSHSPECIAL,yyvsp[0].i_val ); yyval.i_val  = 0;}
break;
case 81:
#line 447 "parse.y"
{ emit(0,1,yyvsp[-3].i_val ); yyval.i_val  = yyvsp[-1].i_val ; }
break;
case 82:
#line 448 "parse.y"
{ emit(0,1,yyvsp[-3].i_val ); yyval.i_val  = yyvsp[-1].i_val ; }
break;
case 83:
#line 449 "parse.y"
{ emit(0,1,yyvsp[-3].i_val ); yyval.i_val  = yyvsp[-1].i_val ; }
break;
case 84:
#line 453 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
case 85:
#line 454 "parse.y"
{ yyval.i_val  = yyvsp[0].i_val ; }
break;
#line 1781 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#ifdef YYDEBUG
        if (yydebug)
            printf("yydebug: after reduction, shifting from state 0 to\
 state %d\n", YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("yydebug: state %d, reading %d (%s)\n",
                        YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#ifdef YYDEBUG
    if (yydebug)
        printf("yydebug: after reduction, shifting from state %d \
to state %d\n", *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
