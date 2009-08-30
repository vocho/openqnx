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




#ifndef FIND_H_INCLUDED

#define FIND_H_INCLUDED

#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <grp.h>
/* #include <i86.h> */
#include <limits.h>
#include <malloc.h>
#include <process.h>
#include <pwd.h>
#include <regex.h>
#include <stdlib.h>
#include <util/stdutil.h>
#include <util/defns.h>
#include <util/stat_optimiz.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#ifndef __QNXNTO__
#include <sys/fsys.h>
#include <sys/fsys_msg.h>
#include <sys/io_msg.h>
#include <sys/kernel.h>
#include <sys/proc_msg.h>
#include <sys/qnx_glob.h>
#endif

#ifdef __QNXNTO__
#include <sys/netmgr.h>
#endif

#include <stdarg.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

//#include <cdefs.h>
#include <inttypes.h>
#include <lib/compat.h>
#include <util/expandbrace.h>
#include <util/find_basename.h>
#include <util/fnmatch2regex.h>
#include <util/lsdashl.h>


#define HEY_DUDE_THAT_IS_BOGUS_HELLO (-1)

#define ORED_SUBEXPR			TRUE
#define NOT_ORED_SUBEXPR		FALSE

#define FAILURE_RC				-1

#define isoctal(c)	(c>='0' && c<='7')

#ifdef __cplusplus
extern "C" {
#endif
struct cmd_struct {
	char *n;
	uint8_t tok;
};
typedef struct cmd_struct cmd;

struct expression {
	uint16_t op;					/* T_AND/T_OR/T_XOR/NULL at root of expression */
	uint8_t  flags;				/* NOT? */
	uint8_t  builtin_fn;			/* 0 when not a builtin */
	struct expression *subexpr;	/* if builtin_fn, this points to data; otherwise
								   it points to a subexpression */
	struct expression *next;	/* points to next expression element */
};
typedef struct expression exprn;

#ifdef __cplusplus
};
#endif
/*
	SAMPLE STRUCTURE LAYOUT FOR TWO CASES 
	-------------------------------------

	-name *.c -o -user ejohnson -group tech

	op=NULL
	builtin_fn = FNMATCH
	subexpr-> "*.c"

	op=OR
	builtin_fn = 0;
	subexpr->   op = NULL
			 	builtin_fn = USER_EQUAL_TO
				subexpr -> "ejohnson"

				op = AND
				builtin_fn = GROUP_EQUAL_TO
				subexpr -> "tech"
				next = NULL

	next = NULL
	

	(-name *.c -o -user ejohnson) -group tech

	op=NULL
	builtin_fn = 0
	subexpr->	op=NULL
				builtin_fn = FNMATCH
				subexpr -> "*.c"

				op=OR
				builtin_fn = 0;
				subexpr->	op = NULL
							builtin_fn = USER_EQUAL_TO
							subexpr-> "ejohnson"
							next = NULL
				next = NULL

	op=AND
	builtin_fn = GROUP_EQUAL_TO
	subexpr -> "tech"
	next = NULL
*/


/* -> builtin expr numbers 
	zzx_BEFORE = (zzx_AFTER = zzx_ON + 1) + 1), ALWAYS, cuz the code
	relies on this relationship between these types of expr numbers
*/

#define TYPE_EQUAL_TO			1
#define PERMS_EQUAL_TO			2
#define HAS_PERMS				3
#define LAST_ACCESSED_ON		4
#define	LAST_ACCESSED_AFTER		5
#define LAST_ACCESSED_BEFORE	6
#define LAST_MODIFIED_ON	    7
#define LAST_MODIFIED_AFTER		8
#define LAST_MODIFIED_BEFORE	9
#define LAST_STATCHG_ON         10
#define LAST_STATCHG_AFTER		11
#define LAST_STATCHG_BEFORE		12
#ifdef CREATION_TIME_SUPPORTED
#define CREATED_ON              13
#define CREATED_AFTER			14
#define CREATED_BEFORE			15
#endif
#define SIZE_EQUAL_TO			16
#define SIZE_LESS_THAN			17
#define SIZE_GREATER_THAN		18
#define GROUP_EQUAL_TO			19
#define GROUP_LESS_THAN			20
#define GROUP_GREATER_THAN		21
#define USER_EQUAL_TO			22
#define USER_LESS_THAN			23
#define USER_GREATER_THAN		24
#define	LINKS_EQUAL_TO			25
#define LINKS_LESS_THAN			26
#define LINKS_GREATER_THAN		27
#define DEV_EQUAL_TO			28
#define FNMATCH					29
#define NOUSER					30
#define NOGROUP					31
#define XDEV					32
#define PRUNE					33
#define DASHOK					34
#define PRINT					35
#define EXEC					36
#define MNEWER					37
#define DEPTH					38
#ifdef EXTENTS_SUPPORTED
#define EXTENTS_EQUAL_TO		39
#define EXTENTS_LESS_THAN		40
#define EXTENTS_GREATER_THAN	41
#endif
#define INODE_EQUAL_TO			42
#define INODE_LESS_THAN			43
#define INODE_GREATER_THAN		44
#define SPAWN					45
#define LS						46
#define BYTES_EQUAL_TO			47
#define BYTES_LESS_THAN			48
#define BYTES_GREATER_THAN		49
#define LEVEL_EQUAL_TO			50
#define LEVEL_LESS_THAN			51
#define LEVEL_GREATER_THAN		52
#define ABORT					53
#define XERR					54
#ifdef MOUNT_INFO_SUPPORTED
#define MOUNTDEV				55
#define MOUNTPOINT				56
#endif
#define EXACTNAME               57
#define ECHOSPAM				58
#define ERRMSG					59
#define LOGICAL					60
#define EXISTS	                61
#define FMNEWER	                62
#define LAST_ACCESSED_ON_MIN		63
#define	LAST_ACCESSED_AFTER_MIN		64
#define LAST_ACCESSED_BEFORE_MIN	65
#define LAST_MODIFIED_ON_MIN	    66
#define LAST_MODIFIED_AFTER_MIN		67
#define LAST_MODIFIED_BEFORE_MIN	68
#define LAST_STATCHG_ON_MIN         69
#define LAST_STATCHG_AFTER_MIN		70
#define LAST_STATCHG_BEFORE_MIN		71
#ifdef CREATION_TIME_SUPPORTED
#define CREATED_ON_MIN              72
#define CREATED_AFTER_MIN			73
#define CREATED_BEFORE_MIN			74
#endif
#define MINDEPTH                75
#define MAXDEPTH                76
#define USED_EQUAL_TO           77
#define USED_LESS_THAN          78
#define USED_GREATER_THAN       79
#define REGEX                   80
#define PRINTF                  81
#define FPRINT                  82
#define FPRINTF                 83
#define FSMANAGER               84
#define ANEWER                  85
#define FANEWER                 86
#define CNEWER                  87
#define FCNEWER                 88
#define FNEWER                  89
#ifdef CREATION_TIME_SUPPORTED
#define FFNEWER                 90
#endif
#define DAYSTART                91
#define EMPTY                   92
#define FLS                     93
#define FPRINT0                 94
#define ILNAME                  95
#define IFNMATCH                96
#define IPATH                   97 
#define IREGEX                  98 
#define LNAME                   99 
#define NOLEAF                  100
#define PRINT0                  101
#define XTYPE                   102
#define PFNMATCH				103
#define IPFNMATCH               104
#define RENAME                  105
#define CHMOD                   106
#define CHOWN                   107
#define CHGRP                   108
#define UNLINK                  109
#define STATUS                  110



/* used in token stage only */
#define NO_TOKEN				0
#define T_LEFT_PARENTHESES		255
#define T_RIGHT_PARENTHESES		254
#define T_AND					253
#define T_OR					252
#define T_XOR					251
#define T_NOT					250

#define FLAG_NOT				1

#define ALWAYS_FALSE			200
#define ALWAYS_TRUE 			201
#define ALWAYS_TRUE_NOP			202


#endif 

