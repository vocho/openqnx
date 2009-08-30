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





#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include    <unistd.h>

#include	"avl.h"

#include	<unistd.h>

/*
**   M A N I F E S T S
*/
#ifndef	TRUE
#	define	TRUE				1
#	define	FALSE				0
#endif
#define MAX_DIR					256
#define	MIN_BLOCK_FILENAME		64		/* Increase the file name table by */
#define	MIN_BLOCK_FUNCPTRS		8		/* Increase the func name table by */
#define LINE_BUFF_SIZE			750
#define MAX_TOKEN_LEN			64
#define RIGHT_MARGIN			50
#define	FULLPATH_MAX			255

#define LF						0x0a

/*
**	L e x   D e f i n e s
*/
#define	IDENTIFIER				257
#define QNX2_PTR_OP1			258
#define QNX2_PTR_OP2			259

#define KEYWORD					260
#define CONSTANT                261
#define PREPROCESS              262
#define COMMENT                 263
#define STRING                  264
#define OTHER                   265
#define OPENBRACKET				266

#ifdef __cplusplus
extern "C" {
#endif

/*
**	D a t a   S t r u c t u r e s
*/
typedef struct
{
	long	DescPos;
	int		DescLen;

} MIGFUNC_STRUCT;


/*
**   E X T E R N A L   V A R I A B L E S
*/
extern FILE						*yyin;			/* Maintained by yylex() */
extern FILE						*yyout;			/* Maintained by yylex() */
extern char						*yytext;		/* Maintained by yylex() */
extern int						 yyleng;		/* Maintained by yylex() */

extern unsigned int				 LineCount;		/* Maintained by yylex() */

extern char						*(*Files)[];	/* File names to process */
extern int						 FileCount;		/* Number of files processed */
extern int						 Debug;			/* Debug flag */
extern int						 FileCurrent;	/* Current file to be processed */
extern int						 Quiet;			/* Quiet operation */
extern int						 Strict;		/* Strict checking */
extern int						 ReportOnly;	/* Produce report only */
extern char						 MarkedUpName[];/* Output Directory */
extern char						*FNode;			/* Pointer of node area of dir */
extern char						 LineBuffer[ LINE_BUFF_SIZE ];
extern AVL_ROOT_STRUCT			*FunctionTree;  /* The AVL tree of mig ids */
extern AVL_ENTRY_STRUCT			*(*Funcs)[];	/* Functions on this line */
extern int						 FuncCount;		/* Count of funcs this line */
extern char						*MigTabName;	/* Mig table file name */
extern FILE						*MigTabfp;		/* Mig table file pointer */
extern int						 ExitStatus;	/* main() exit status */

extern char					 	*OutofMemory;


/*
**   F U N C T I O N   P R O T O T Y P E S
*/
extern int						yylex( void );
extern int						yywrap( void );
extern void						yyeol( void );
extern int						LoadFunctionTree( void );
extern void						PrintMarkups( char EoLChar );

#ifdef __cplusplus
};
#endif

/* End of File */
