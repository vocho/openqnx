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
	These are constants/data structures to help the parser/generator
	process.

	They are separated into this include primarily to reduce the
	"clutter" of the parser table.

*/

#ifndef	_bcgen_h_included
#define	_bcgen_h_included

#ifdef __cplusplus
extern "C" {
#endif

#define	_LVALUE		1000
#define	_RVALUE		2000

#ifndef	MAX_CSTACK
#define	MAX_CSTACK	50
#endif

#define	code_space(_c)	((_c)->ilen - (_c)->cur_offs)


#define	IS_ARRAY	01		/* flag to show parameter is an array */
#define	MAX_PARM	255
    

/*	
	These are for back-patching the labels in the code.
*/

#define	LABEL_FUNC		0	/* store the label for a function name */
#define	LABEL_LOOP		1	/* top of a loop */
#define	LABEL_EXIT		2	/* exit point of a loop */
#define	LABEL_FOR1		3	/* target for a for loop */
#define	LABEL_FOR2		4	/* extra target for for loop */
#define	LABEL_RETURN	5	/* all returns from the bottom of a function */
#define	LABEL_IFEXIT	6	/* if statements exit to this point */

#define	_LABEL_DEFINED	01
#define	_LABEL_USED		02
#define	_LABEL_MASK		03
/*
	The NIL_LABEL marks the end of a list.
*/
#define	NIL_LABEL	-1

/*	These may be used later, but are a little confusing at first! */
#define	is_defined(_c,_lbl) ((_c)->lbl_flags & (_LABEL_DEFINED << (_lbl*2)))

#define	is_used(_c,_lbl) ((_c)->lbl_flags & (_LABEL_USED << (_lbl*2)))

#define	do_define(_c,_lbl) ((_c)->label[_lbl] = _c->cur_offs, \
					(_c)->lbl_flags &= ~(_LABEL_MASK << (_lbl*2)),\
					(_c)->lbl_flags |= (_LABEL_DEFINED << (_lbl*2)))

#define	do_reference(_c,_lbl)	((_c)->lbl_flags &= ~(_LABEL_MASK << (_lbl*2)), \
				(_c)->lbl_flags |= (_LABEL_USED << (_lbl*2)))


#define	NLABELS		7

/*
	This the structure for intermediate code.
*/

#define	IMMED		0

struct	icode {
		short		ityp;
		short		ilen;		/* the length of the code section */
		short		cur_offs;		/* the current instruction */
		int			*code;		/* the code itself */
		unsigned	lbl_flags;	/* flags indicating which labels are used */
		int			label[NLABELS];	
	};


#ifndef	MIN_CODE_ALLOC
#define	MIN_CODE_ALLOC	64
#endif

#ifdef __cplusplus
};
#endif

#endif
