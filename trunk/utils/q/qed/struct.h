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
 * April 1980 by  D. T. Dodge
 */


struct line {
	struct line *next;
	struct line *prev;
	char lflags;
	char textp[1];	/* variable length buffer for line text. */
	} ;


struct macro_entry {
	struct macro_entry *mlink;
	char mchar;
	char mflag;
	char mstr[1];	/* variable length buffer for macro text. */
	} ;


struct macro_stk_entry {
	char macro_flags;
	char *macro_ptr;
	} ;

struct opt {
/*
 * Options storage. Option 'a' MUST be the first. If a new option is
 * added the manifest NUM_OPTIONS must be updated accordingly. Note
 * that opt_e IS NOT counted and must be kept separate!
 */

	char opt_a, opt_b, state, opt_d, opt_f, opt_i, opt_j, opt_l, opt_m, opt_n,
			opt_s, opt_t, opt_w;
	} ;
