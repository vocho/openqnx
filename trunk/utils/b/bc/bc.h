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
	This file contains manifests and structures shared by
	portions of BC, especially the "op-codes" needed by the
	interpretor.
*/


#ifndef	_bc_h_included
#define	_bc_h_included

/*	aliases for the error handlers */
#define	NO_CORE(s)	no_mem(s)


#ifndef _number_h_included
#include "number.h"
#endif

typedef	struct operand	oprnd_t;

/*
	These are the "tag" fields associated with numbers.
	They help the garbage collection routines determine
	when a value is nolonger reachable.
*/

enum {
	_VAR,
	_IARRAY,
	_DISCARD,
	_SCRATCH,
	_PERM,	
	_INBASE,
	_OUTBASE,
	_CSCALE,
	_TEMP
};

enum {	MAX_EVAL_STACK = 100 };
enum {	MAX_CALL_STACK = 20 };
enum {	MAX_REGS       = 26 };
enum {	PREFIX         = 0 };
enum {	POSTFIX	       = 0x1000 };
enum {  BC_OUT_LINE_MAX = 70 };
enum { BC_DEFAULT_SCALE = 0};
#ifndef BC_SCALE_MAX
#define BC_SCALE_MAX 100
#endif
enum { BC_IBASE_MAX = 100};
enum { BC_OBASE_MAX = 10000};


enum {
	_VAR_IBASE,
	_VAR_OBASE,
	_VAR_SCALE,
	MAX_STATEVAR
};

#ifndef	MAX_INDEX
#define	MAX_INDEX	2048
#endif

struct operand	{
	int	  type;		
	numb_t	 *value;		
	oprnd_t **table;	/* this could be unioned with above! */
	oprnd_t  *next;		/* local stack for this var. */
};

#define	op_value(_op)		((_op)->value)
#define	op_setvalue(_op,_v)	((_op)->value = (_v))
#define	op_type(_op)		((_op)->type)
#define	op_settype(_op,_typ)	((_op)->type = (_typ))
#define	op_push(_op1,_op2)	((_op1)->next = (_op2))
#define	op_pop(_op1)		((_op1)->next)
#define	op_table(_op)		((_op)->table)
#define	op_settable(_op,_n)	((_op)->table = (_n))
#define	op_arrayel(_op,_ind)	((_op)->table[(_ind)])
#define	op_setarrayel(_op,_ind,_val)	((_op)->table[(_ind)] = (_val))

int	bc_printf(char *fmt,...);

#endif
