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
	This file, along with mkasmoff, is used to generate an assembler
	include file that contains the offsets of fields and values of
	manifest constants. The strings in the "COMMENT" macro will appear
	as comments in the resulting assembler include file. The first
	parameter of the "VALUE" macro will be the name of the assembler
	symbolic constant while the second parameter will be its value.

	Things are done in this roundabout way so that the correct
	values are generated even if the host system used to compile the
	kernel is not the same as the target system it's going to run
	on.
*/

/* These extra macros are needed because of ANSI - trust me. */
#define _NAME( pref, line, suff ) pref##line##suff
#define NAME( pref, line, suff ) _NAME( pref, line, suff )

#define COMMENT_SUFFIX	_
#define COMMENT( comm ) char NAME( comment, __LINE__, COMMENT_SUFFIX )[] = comm
#define VALUE( name, val ) unsigned NAME( value, __LINE__, ____##name ) = val
