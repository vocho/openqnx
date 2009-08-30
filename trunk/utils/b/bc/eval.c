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
 * evaluator for BC. This comprises a relatively simple stack-based
 * interpretor. It understands relative jumps based upon results, and
 * call/return statements.
 * 
 * Debugging here is a bit tedious.  The best approach i have discovered is to
 * set the debugging flags on, and have bc generate "listings" of the stack
 * code, then follow the flow through here.  A debugging, or judicious use of
 * print statements greatly improve things.
 * 
 */
#include <stdio.h>
#include "bc.h"
#include "y.tab.h"
#include <limits.h>

extern int      disasm(int *startptr, int len);
extern int      _disasm(int offs, int *startptr, int len);
extern int      print_num(const numb_t * num, int outputbase, int eoln);

int bin_op(int op);
int rel_op(int op);
void asg_op(int op);

/*
 * This section manages the numbers.
 */

oprnd_t        *constant_0 = NULL;
oprnd_t        *constant_1 = NULL;

#define    CONSTANT_0    constant_0
#define    CONSTANT_1    constant_1

static oprnd_t *var_tab[MAX_REGS];
static oprnd_t *array_tab[MAX_REGS];
static oprnd_t *state_var[MAX_STATEVAR];

static oprnd_t *templist = NULL;

static FRAME   *op_pool = NULL;
#ifndef    OP_FRAMESIZE
#define    OP_FRAMESIZE    128
#endif

/* These deal with generic variables */

oprnd_t        *
new_op()
{
	oprnd_t        *p;
	if (op_pool == NULL) {
		if ((op_pool = fr_create(OP_FRAMESIZE, sizeof(oprnd_t))) == NULL)
			NO_CORE("new_var");
	}
	if ((p = fr_alloc(op_pool)) == NULL)
		NO_CORE("new_var");
	return p;
}

oprnd_t        *
make_op(numb_t *n, int typ)
{
	oprnd_t        *p;
	p = new_op();
	op_settype(p, typ);
	op_setvalue(p, n);
	return p;
}

#define    new_var()            make_op(dtonum(0),_VAR)
#define    make_scratch(_n)    make_op((_n),_SCRATCH)

oprnd_t        *
make_perm(numb_t *n)
{
	oprnd_t        *p;
	p = new_op();
	op_settype(p, _PERM);
	op_setvalue(p, n);
	return p;
}

oprnd_t        *
make_temp(numb_t *n)
{
	oprnd_t        *p;
	p = make_op(n, _TEMP);
	p->next = templist;
	templist = p;
	return p;
}



int delete_op(oprnd_t *p)
{
	int             ecode = 1;
	if (p == NULL) {
		error(0, "delete_op NULL\n");
		return -1;
	}
	if (op_value(p) != NULL)
		if (delete_num(op_value(p)) < 0) {
			error(0, "delete_op, value failed: type %d\n", op_type(p));
			ecode = -1;
		}
	fr_free(op_pool, p);
	return ecode;
}

void
collect_garbage()
{
	oprnd_t        *p, *c;
	for (p = templist, c = NULL; p != NULL; p = c) {
		c = p->next;
		delete_op(p);
	}
	templist = NULL;
}


/* These deal with the array variables: */
oprnd_t        *
new_array()
{
	oprnd_t        *p;
	p = new_op();
	op_settype(p, _IARRAY);
	op_settable(p, NULL);
	return p;
}

void delete_array(oprnd_t *N)
{
	oprnd_t       **p;
	int             i;
	if (N == NULL)
		return;
	if (op_table(N) != NULL) {
		for (p = op_table(N), i = 0; i < MAX_INDEX; i++) {
			if (p[i] != NULL)
				delete_op(p[i]);
		}
		FREEMEM(op_table(N));
	}
	fr_free(op_pool, N);	/* release to op pool */
}

void assign_array(oprnd_t *X, oprnd_t *N)
{
	int             i;
	register oprnd_t **p, **q;
	if (N == NULL || X == NULL) {
		/* passed a NULL array */
		error(1, "assign_array: NULL array\n");
	}
	if (op_type(N) == _IARRAY) {
		if (op_table(N) != NULL) {
			if (op_table(X) == NULL) {
				if (op_settable(X, ALLOCMEM(oprnd_t *, MAX_INDEX)) == NULL)
					error(1, "create_array: no memory");
			}
			p = op_table(X);
			q = op_table(N);
			for (i = 0; i < MAX_INDEX; i++) {
				if (q[i] != NULL)
					p[i] = make_op(dup_num(op_value(q[i])), _VAR);
			}
		}
	}
}



oprnd_t        *
get_indexed(oprnd_t *base, oprnd_t *index)
{
	int             i;
	if (base == NULL)
		error(1, "get_indexed: NIL array");
	if (op_table(base) == NULL)
		if (op_settable(base, ALLOCMEM(oprnd_t *, MAX_INDEX)) == NULL)
			error(1, "delete_array: no memory");
	i = numtod(op_value(index));
	if (i < 0 || i > MAX_INDEX - 1) {
		error(0, "index out of range");
		i = 0;
	}
	if (op_arrayel(base, i) == NULL) {
		op_setarrayel(base, i, new_var());
	}
	return op_arrayel(base, i);
}



/*
 * assign handles the "state variables". It attempts to assign a value to the
 * variable. It ensure it reflects the same value as the state variable.
 */

int assign(oprnd_t *dest, oprnd_t *src)
{
	int             newvalue;
	int             t, r;
	int             ecode = 1;
	switch (op_type(dest)) {
	case _INBASE:
		newvalue = numtod(op_value(src));	/* get the value */
		if (set_ibase(newvalue) != newvalue) {
			error(0, "invalid assignment to ibase");
		} else {
			delete_num(op_value(dest));
			op_setvalue(dest, dtonum(newvalue));
		}
		break;
	case _OUTBASE:
		newvalue = numtod(op_value(src));	/* get the value */
		if (set_obase(newvalue) != newvalue) {
			error(0, "invalid assignment to obase");
		}
		delete_num(op_value(dest));
		op_setvalue(dest, dtonum(get_obase()));	/* get default */
		break;
	case _CSCALE:{
			static int      scale_err = 0;
			newvalue = numtod(op_value(src));	/* get the value */
			t = roundew(newvalue);
			t /= 2;
			if (t < 0) {
				if (!scale_err) {
					scale_err = 1;
					error(0, "scale may not be negative, ignored\n");
				}
				newvalue = -1;
			} else if ((r = set_scale(t)) == -1) {
				if (!scale_err) {
					scale_err = 1;
					error(0, "invalid assignment to scale, ignored\n");
				}
				newvalue = -1;
			} else if (r != t) {
				if (!scale_err) {
					scale_err = 1;
					error(0, "assignment to scale larger than %d\n", BC_SCALE_MAX);
				}
			} else {
				scale_err = 0;	/* clear it */
			}
			delete_num(op_value(dest));
			op_setvalue(dest, dtonum(newvalue));	/* get default */
		}
		break;
	default:
		if (op_value(dest) != NULL) {
			if (delete_num(op_value(dest)) < 0) {
				error(0, "assign: delete_num failed!\n");
				ecode = -1;
			}
		}
		op_setvalue(dest, dup_num(op_value(src)));
	}
	return ecode;
}


oprnd_t        *
op_scale(oprnd_t *p)
{
	int             sc;
	sc = get_precision(op_value(p));
	sc *= 2;
	return make_scratch(dtonum(sc));

}

oprnd_t        *
op_length(oprnd_t *p)
{
	int             len;
	len = get_length(op_value(p));
	return make_scratch(dtonum(len));
}



/* These are for "garbage" collection */

int discard(oprnd_t *p)
{
	if (op_type(p) == _SCRATCH || op_type(p) == _DISCARD) {
		if (delete_op(p) < 0) {
			error(0, "discard: delete op failed\n");
			return -1;
		}
	}
	return 1;
}



/*
 * This section deals with maintaining call stacks, and function tables
 */
struct code {
	int            *start;
	int            *end;
};


#define    MAX_CALL_STACK    20

static struct code func_tab[MAX_REGS];

static struct code call_stack[MAX_CALL_STACK];	/* levels of nesting */
static int      cstkptr = 0;

void C_PUSH(int *strt, int *end)
{
	if (cstkptr < MAX_CALL_STACK) {
		call_stack[cstkptr].start = strt;
		call_stack[cstkptr++].end = end;
	} else {
		error(1, "call stack overflow\n");
	}
}

struct code    *
C_POP()
{
	if (cstkptr > 0) {
		return &call_stack[--cstkptr];
	} else {
		error(1, "call stack underflow\n");
		return 0;
	}
}


int add_func(int findex, int *start, int len)
{
	if (findex < 0 || findex >= MAX_REGS)
		return -1;
	if (func_tab[findex].start != NULL)
		return 0;
	func_tab[findex].start = start;
	func_tab[findex].end = start + len;
	return 1;
}

/* disassemble a whole function */
#if 0
disfunc(int findex)
{
	if (func_tab[findex].start == 0) {
		return -1;
	}
	disasm(func_tab[findex].start, func_tab[findex].end - func_tab[findex].start);
	return 0;
}
#endif

int func_defined(int findex)
{
	if (findex < 0 || findex >= MAX_REGS)
		return -1;
	return func_tab[findex].start != NULL;
}


/*
 * This section manages the evaluation stack.
 */
#define    MAX_EVAL_STACK    100
static oprnd_t *eval_stack[MAX_EVAL_STACK];
static int      esp = 0;

oprnd_t        *
push(oprnd_t *x)
{
	if (esp < MAX_EVAL_STACK) {
#if 0
		/* use this only if you are desparate    */
		error(1, "PUSH:op=%p, op->type = %d\n", x, x->type);

#endif
		return eval_stack[esp++] = x;
	}
	error(1, "eval stack overflow\n");
	return NULL;
}

oprnd_t        *
peekn(int n)
{
	if (esp > n)
		return eval_stack[esp - 1 - n];
	error(0, "peek stack underflow\n");
	return 0;
}

oprnd_t        *
pop()
{
	if (esp > 0)
		return eval_stack[--esp];
	error(1, "eval stack underflow\n");
	return NULL;
}

oprnd_t        *
tos()
{
	if (esp > 0)
		return eval_stack[esp - 1];
	error(1, "eval stack underflow\n");
	return NULL;
}

int stack_empty()
{
	return esp == 0;
}

void clear()
{
	esp = 0;
}



void init_eval()
{
	static int      initial = 0;
	int             i;
	numb_t         *n;

	if (initial)
		return;
	for (i = 0; i < MAX_REGS; i++) {
		func_tab[i].start = NULL;
		var_tab[i] = NULL;
		array_tab[i] = NULL;
	}
	n = dtonum(0);		/* generate constant 0 */
	constant_0 = make_perm(n);
	n = dtonum(1);		/* and constant 1 */
	constant_1 = make_perm(n);
	/* initialize the "state" variables */
	n = dtonum(get_scale());/* use default scale value */
	state_var[_VAR_SCALE] = make_op(n, _CSCALE);
	n = dtonum(get_ibase());
	state_var[_VAR_IBASE] = make_op(n, _INBASE);
	n = dtonum(get_obase());
	state_var[_VAR_OBASE] = make_op(n, _OUTBASE);
	initial = 1;
}


/*
 * This is the interpreter.
 */

void execute(int *start, int len)
{
	register int   *exe_cur;
	register int   *exe_end;

	struct code    *retptr;

	oprnd_t        *op1, *op2;
	extern int      trace;

	exe_cur = start;
	exe_end = start + len;
	if (trace)
		_disasm(0, exe_cur, len);

	while (exe_cur < exe_end) {
		if (trace)
			_disasm(-1, exe_cur, 1);
		switch (*exe_cur++) {
			/* first the control transfer group */
		case JMP:
			exe_cur += *exe_cur;	/* jump relative to a
						 * location */
			break;
		case CALL:
			if (func_tab[*exe_cur].start != NULL) {
				C_PUSH(exe_cur + 1, exe_end);
				exe_end = func_tab[*exe_cur].end;
				exe_cur = func_tab[*exe_cur].start;
			} else {
				error(1, "invalid: function %d not implemented", *exe_cur);
				exe_cur++;
			}
			break;
		case RETURN:
			retptr = C_POP();
			exe_cur = retptr->start;
			exe_end = retptr->end;
			break;
		case JMPZ:
			op1 = pop();
			if (num_iszero(op_value(op1)))
				exe_cur += *exe_cur;	/* jump relative */
			else
				exe_cur++;
			discard(op1);
			break;
		case JMPNZ:
			op1 = pop();
			if (!num_iszero(op_value(op1)))
				exe_cur += *exe_cur;
			else
				exe_cur++;
			discard(op1);
			break;
			/* miscelaneous: */
		case PRINT_LIST:
			{
				int             i, n = *exe_cur++;
				int             ob = get_obase();
				for (i = 0; i < n; i++) {
					op1 = peekn(n - i - 1);
					print_num(op1 ? op_value(op1) : 0, ob, i == n - 1);
					if (i != n - 1) {
						bc_printf(" ");
					}
				}
				for (i = 0; i < n; i++)
					discard(pop());
			}
			break;
		case PRINT:
			op1 = pop();
			print_num(op1 ? op_value(op1) : 0, get_obase(), 1);
			discard(op1);
			break;
		case '\"':
			/* print a string */
			bc_printf("%s", *(char **) exe_cur);
			exe_cur += sizeof(char *) / sizeof(*exe_cur);
			break;
		case DISCARD:
			op1 = pop();
			discard(op1);
			break;
		case CLRSTK:
			while (!stack_empty()) {
				op1 = pop();
				discard(op1);
			}
			break;
		case REPLACE:
			op1 = pop();
			op2 = make_scratch(dup_num(op_value(op1)));
			push(op2);
			discard(op1);
			break;
			/*
			 * These all have one word-operand, which is the
			 * index into one of the tables:
			 */
		case SAVEVAR:
			op1 = var_tab[*exe_cur];	/* get the var ! */
			op2 = new_var();	/* create a new one */
			op_push(op2, op1);	/* link it in */
			var_tab[*exe_cur++] = op2;	/* and store the result */
			break;
		case RESTVAR:
			op1 = var_tab[*exe_cur];	/* the exe_currently
							 * visible one */
			var_tab[*exe_cur++] = op_pop(op1);
			delete_op(op1);	/* discard the previous value */
			break;
		case SAVEARRAY:
			op1 = array_tab[*exe_cur];	/* save a whole array */
			op2 = new_array();	/* make an empty one */
			op_push(op2, op1);
			array_tab[*exe_cur++] = op2;
			break;
		case RESTARRAY:
			op1 = array_tab[*exe_cur];
			array_tab[*exe_cur++] = op_pop(op1);
			delete_array(op1);
			break;
		case INDEX:
			op1 = pop();	/* base */
			op2 = pop();	/* index */
			if ((op1 = get_indexed(op1, op2)) == NULL) {
				error(1, "ivalid index\n");
				push(CONSTANT_0);
			} else {
				push(op1);
			}
			discard(op2);	/* in case it is a temporary */
			break;

			/* stack operators: */
		case PUSHVAR:
			if (var_tab[*exe_cur] == NULL)
				var_tab[*exe_cur] = new_var();
			push(var_tab[*exe_cur++]);
			break;
		case PUSHCONST:
			op1 = *(oprnd_t **) exe_cur;
			exe_cur += sizeof(oprnd_t *) / sizeof(*exe_cur);
#if 0
			op1 = *((oprnd_t **) exe_cur++);
#endif
			push(op1);
			break;
		case PUSHARRAY:
			if (array_tab[*exe_cur] == NULL)
				array_tab[*exe_cur] = new_array();
			push(array_tab[*exe_cur++]);
			break;
		case PUSHSPECIAL:
			switch (*exe_cur++) {
			case IBASE:
				push(state_var[_VAR_IBASE]);
				break;
			case OBASE:
				push(state_var[_VAR_OBASE]);
				break;
			case SCALE:
				push(state_var[_VAR_SCALE]);
				break;
			default:
				error(1, "Unknown special type %d\n", exe_cur[-1]);
			}
			break;
		case POPARRAY:
			op2 = pop();
			assign_array(array_tab[*exe_cur++], op2);
			discard(op2);
			/*
			 * silly because you can't yet have a temporary array
			 * ??
			 */
			break;
		case POPVAR:
			op1 = var_tab[*exe_cur++];
			op2 = pop();
			assign(op1, op2);
			discard(op2);
			break;
			/* unary operators: */
		case UMINUS:
			op1 = pop();
			op2 = make_scratch(dup_num(op_value(op1)));
			num_negate(op_value(op2));
			push(op2);
			discard(op1);
			break;
		case '!':
			op1 = pop();
			if (num_iszero(op_value(op1))) {
				push(CONSTANT_1);
			} else {
				push(CONSTANT_0);
			}
			discard(op1);
			break;
		case PLUSPLUS | PREFIX:
			op1 = pop();
			push(op1);
			push(CONSTANT_1);
			asg_op(ASG_PLUS);
			break;
		case MINUSMINUS | PREFIX:
			op1 = pop();
			push(op1);
			push(CONSTANT_1);
			asg_op(ASG_MINUS);
			break;
		case PLUSPLUS | POSTFIX:
			op1 = pop();
			op2 = make_scratch(dup_num(op_value(op1)));
			push(op2);	/* leave copy of old value on stack */
			push(op1);
			push(CONSTANT_1);
			asg_op(ASG_PLUS);
			op1 = pop();	/* new value  */
			break;
		case MINUSMINUS | POSTFIX:
			op1 = pop();
			op2 = make_scratch(dup_num(op_value(op1)));
			push(op2);
			push(op1);
			push(CONSTANT_1);
			asg_op(ASG_MINUS);
			op1 = pop();
			break;
			/* assignment operators */
		case '=':
			op1 = pop();
			op2 = pop();
			assign(op2, op1);
			discard(op1);
			push(op2);
			break;
		case ASG_PLUS:
		case ASG_MINUS:
		case ASG_STAR:
		case ASG_DIV:
		case ASG_MOD:
		case ASG_EXP:
			asg_op(exe_cur[-1]);
			break;

			/* binary operators */
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '^':
			bin_op(exe_cur[-1]);
			break;
			/* relational operators */
		case EQ:
		case NE:
		case LE:
		case GE:
		case '<':
		case '>':
			rel_op(exe_cur[-1]);
			break;
		case LAND:
			op1 = pop();
			op2 = pop();
			if (!num_iszero(op_value(op1)) && !num_iszero(op_value(op1))) {
				push(CONSTANT_1);
			} else {
				push(CONSTANT_0);
			}
			discard(op1);
			discard(op2);
			break;
		case LOR:
			op1 = pop();
			op2 = pop();
			if (!num_iszero(op_value(op1)) || !num_iszero(op_value(op1))) {
				push(CONSTANT_1);
			} else {
				push(CONSTANT_0);
			}
			discard(op1);
			discard(op2);
			break;

			/* built-ins */
		case SQRT:
			op1 = pop();
			push(make_scratch(num_sqrt(op_value(op1))));
			discard(op1);
			break;
		case SCALE:
			op1 = pop();
			push(op_scale(op1));
			discard(op1);
			break;
		case LENGTH:
			op1 = pop();
			push(op_length(op1));
			discard(op1);
			break;
		default:
			error(1, "[%4.4x] unknown opcode %x\n", exe_cur - start, exe_cur[-1]);
			error(1, "invalid opcode\n");
		}
	}

}

/*
 * binop handles binary operations, and cleans up any residual values.
 */

int bin_op(int op)
{
	numb_t         *op1, *op2; 
   numb_t         *res = NULL;
	oprnd_t        *opr1, *opr2;

	opr2 = pop();
	opr1 = pop();

	op1 = op_value(opr1);
	op2 = op_value(opr2);

	switch (op) {
	case '+':
		res = add_num(op1, op2);
		break;
	case '^':
		res = raise_num(op1, op2);
		break;
	case '-':
		res = sub_num(op1, op2);
		break;
	case '*':
		res = mult_num(op1, op2);
		break;
	case '%':
		res = rem_num(op1, op2);
		break;
	case '/':
		res = div_num(op1, op2);
		break;
	default:
		error(1, "unknown binary operator %d\n", op);
		error(1, "binop");
		break;
	}
	discard(opr1);
	discard(opr2);
	push(make_scratch(res));
	return 1;
}

int rel_op(int op)
{
	oprnd_t        *opr1, *opr2;
	int             res, t;

	opr2 = pop();
	opr1 = pop();

	res = num_compare(op_value(opr1), op_value(opr2));
	switch (op) {
	case NE:
		t = res;
		break;
	case EQ:
		t = res == 0;
		break;
	case GE:
		t = res >= 0;
		break;
	case '>':
		t = res > 0;
		break;
	case '<':
		t = res < 0;
		break;
	case LE:
		t = res <= 0;
		break;
	default:
		error(1, "unknown relational operator %d\n", op);
		error(1, "relop");
		return 0;
	}
	discard(opr1);
	discard(opr2);
	push(t ? CONSTANT_1 : CONSTANT_0);
	return 1;
}




void asg_op(int op)
{
	oprnd_t        *op1, *op2;

	op1 = pop();
	op2 = tos();		/* remember this one */
	push(op1);
	switch (op) {
	case ASG_PLUS:
		bin_op('+');
		break;
	case ASG_MINUS:
		bin_op('-');
		break;
	case ASG_STAR:
		bin_op('*');
		break;
	case ASG_DIV:
		bin_op('/');
		break;
	case ASG_MOD:
		bin_op('%');
		break;
	case ASG_EXP:
		bin_op('^');
		break;
	default:
		error(1, "unknown assignment operator %d\n", op);
		error(1, "asgop");
	}
	op1 = pop();		/* the result of the "binop" */
	assign(op2, op1);
	discard(op1);
	push(op2);
}
