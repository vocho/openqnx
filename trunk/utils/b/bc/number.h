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




/*- number.h:
	access routines for arbitrary precision numeric library.
	Contains manifests for defaults, upper limits
	Internal data structures.
	Access routines.
*/

#ifndef	_number_h_included
#define	_number_h_included

#include <sys/types.h>
#include <limits.h>

#include "mem.h"
#include "vlong.h"


typedef struct _num numb_t;


struct _num {
	short	decpos,			/* decimal position */
		len;			/* total len of number */
	short	sign;			/* is negative or positive */
	digit_t	*digit;
};

int	get_scale(void), set_scale(int);
int	get_ibase(void), set_ibase(int);
int	get_obase(void), set_obase(int);

numb_t *get_align(numb_t *, numb_t *);

numb_t	*add_num( numb_t *, numb_t *);
numb_t	*sub_num( numb_t *, numb_t *);
numb_t	*mult_num( numb_t *, numb_t *);
numb_t	*div_num( numb_t *, numb_t *);
numb_t  *rem_num( numb_t *, numb_t *);
numb_t	*raise_num( numb_t *, numb_t *);
numb_t	*num_sqrt(numb_t *);


int	num_iszero(numb_t *);
int	num_compare(numb_t *, numb_t *);
void	num_negate(numb_t *);

numb_t	*normal(numb_t *);
digit_t	*unnormal(numb_t *,int len, int decpos);
int	trim_num(numb_t *);
numb_t	*clear_num(numb_t *);
numb_t	*dup_num(numb_t *);
int	get_size(numb_t *);
int	get_length(numb_t *);
int	get_precision(numb_t *);
int	set_length(numb_t *, int len);
int	copy_num(numb_t *to, numb_t *from);

numb_t	*strtonum(char *s, char **end, int base);
int	 numtos(numb_t *n, char *s, int limit, int base);

numb_t	*dtonum(int n);		/* integer to numb_t  converter */
int	 numtod(numb_t *n);	/* numb_t to integer converter */

int	 delete_num(numb_t *);
numb_t	*create_num(void);


/*-
 These were elected as macroes since they are simple, often used utilities.
*/
/* number of digits required to represent */
#define	GETSIZE(_r) (MAX((_r)->len,(_r)->decpos))

#define	NEWHDR()	create_num()
#define NEWNUM(_r)	(_r->digit=ALLOCMEM(digit_t,_r->len))
#define	CHGSIZ(_r,_l)	(_r->digit=MEMCHSIZE(_r->digit,_r->len=_l))
#define	PADNUM(_r,_l)	set_length(_r,_l)
#define	SETPREC(_r,_l)	set_prec(_r,_l)

#define	is_negative(_r)	((_r)->sign ? 1 : 0)
#define	is_positive(_r)	(! is_negative(_r))

#define	isodd(_x)	((_x)&01)
#define	iseven(_x)	(!isodd(_x))
#define	roundew(_x)	(iseven(_x) ? (_x) : (_x)+1)
#define	roundow(_x)	(isodd(_x)  ? (_x) : (_x)+1)

int  error(int ecode, char *fmt, ...);
void no_mem(char *where);
void program_exit(void);

#endif
