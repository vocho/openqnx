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






#ifndef	_sort_h_included
#define	_sort_h_included

#ifdef __MINGW32__
#include <lib/compat.h>
#endif

#include	<stdio.h>
#include	<ctype.h>
#include	<locale.h>
#include	<limits.h>

#ifdef __STDC__

#include	<stdlib.h>
#ifndef __QNXNTO__
#include	<unistd.h>
#endif
#include	<string.h>

#endif


#ifndef	SORT_LINE_MAX
#define	SORT_LINE_MAX	512
#endif


#define	SORT_TMPNAM_BASE	"_sort.XXXXXX"

#ifndef	SORT_MAX_FILES
#define	SORT_MAX_FILES	8
#endif

#ifndef	SORT_MAX_RUN
#define	SORT_MAX_RUN	10000
#endif



/*-
 Important: 
	the following defines are dependant upon the order of
	these flags.  You may reorder the string, only if you
	make the corresponding changes to the bitmap masks.
*/

#define	ORDER_RULES	"dfinrbB"
#define	FLD_NOPUNCT	001
#define	FLD_NOCASE	002
#define	FLD_PRTCHARS	004
#define	FLD_NUMERIC	050
#define	FLD_REVERSE	020
#define	FLD_NOBLANKS	040
#define FLD_BACKWARDS   0100


#define	DEFAULT_FS  '\t'

enum {
	FLD_NO = 0,
	FLD_OFFS,
	FLD_TYP,
	FLD_DUMMY	/* a dummy, to help compiler address */
};

#define	FLD_SIZE 4

struct fieldspec {
	int		start[FLD_SIZE];
	int		end[FLD_SIZE];
};



/*-
	some explanation is in order here.

	Each line is read into a "struct sort_line"; which concatenates
	a map of the fields and the data from the line.
	Every line has a field 0, which is the entire line.
	The member str_start is the offset from the union data member "str"
	to the first character in the line; thus the string itself is at
	"l->str+l->str_start".  This is hidden by the macro "STR_BEGIN".
	The number of fields is determined by the command line; although
	the internal structures support a different number for each line.
	The two integers stab[0] and stab[1] are the offsets of the beginning
	of each field (stab[n]) and the field terminator (stab[n+1]).
	len is the line length, redundant as "l->stab[1]+1" is the same
	thing.

	In practice, the code for this is rather cryptic, so the high runner
	examples are explained here rather than cluttering the code.

	for a given struct sort_line *p;
		p->len :
			the number of bytes in the string.
	
		p->data.str[p->str_start] :
			is the first char in the string.

		p->data.str[p->str_start+p->data.stab[fieldno*2]] :
			is the first char of the string in field 'fieldno'

		p->data.str[p->str_start+p->data.stab[fieldno*2+1]] :
			is the last char of the string in field 'fieldno'
		
*/

typedef	struct sort_line linedesc;
struct sort_line {
	linedesc       *next;
	int             len;		/* line length */
	int             str_start;
	union	{
		char    str[1];
		int     stab[2];
	} data;
};


/*-
	forgive the C++ style of this, but i can't think of
	anything else to help keep it clear.
*/

#define	linealloc(_nf,_ll) \
		calloc(1,sizeof(linedesc)+2*(_nf)*sizeof(int)+(_ll)+1)

#define	line_free(_ll)	free((_ll))

#define	STR_BEGIN(_p)	        ((_p)->data.str+(_p)->str_start)
#define	STR_LEN(_p)             ((_p)->len)
#define	STR_FLD(_p,_fno)        ((_p)->data.str+(_p)->str_start+(_p)->data.stab[(_fno)*2])
#define	STR_FLDEND(_p,_fno)	((_p)->data.str+(_p)->str_start+(_p)->data.stab[(_fno)*2+1])
#define	STR_FLDLEN(_p,_fno)	((_p)->data.stab[(_fno)*2+1] - (_p)->data.stab[(_fno)*2])



struct	_fstruct {
	int	state;
	FILE	*f;
	int		buflen;
	char	buffer[SORT_LINE_MAX+1];
#ifdef	__STDC__
	linedesc	*(*readline)(struct _fstruct *);
#else
	linedesc	*(*readline)();
#endif
};

typedef	struct	_fstruct	fdesc;

#define	BUF_VALID	001
#define	END_INPUT	002
#define	NEED_INPUT(_f)	(((_f)->state & BUF_VALID) == 0)
#define	AT_EOF(_f)	(((_f)->state & END_INPUT) != 0)
#define	INPUT_LINE(_f)	((_f)->readline)((_f))

/*
	text emitted by sort.
*/

#define SORT   "sort "
#define	TXT(_x)	(SORT _x "\n")

#define T_BAD_OFILE     "error: can't access output file (%s): %s"
#define T_RSV_OFILE     "error: can't reserve FILE pointer for output file (%s): %s"
#define T_MERGE_ARGS    "error: merge requires at least one file"
#define	T_BAD_KEY	"error: malformed key <%s>"
#define	T_BAD_ARGUMENT  "error: unknown argument (%c)"
#define	T_NOMEMORY	"error: not enough memory"

#define	T_IGNORE_FILES  "warning: extraneous files ignored"
#define T_SIGNAL SORT   "warning: unable to catch signal %d"
#define	T_NUMFILES	"warning: must have at least 2 files, ignored"
#define	T_NUMLINES	"warning: must have at least 2 lines, ignored"


/*	engine.c	*/
extern int fcompare(const void *, const void *);
extern int merge_files(FILE *, fdesc **, int);
extern int file_ordered(fdesc *);
extern int write_file(FILE *, linedesc **, int);

/*	files.c			*/
extern int ungetline(fdesc *,linedesc *); 
extern fdesc *open_fdesc(FILE *, int );
extern int close_fdesc(fdesc *);


/*	fields.c		*/
extern int newfs(char *,int);
extern int add_flag(int);
extern int get_flags(int);
extern int get_nfields(void);
extern int alloc_field(void);
extern int dealloc_field(void);

extern int draft9_field(char *spec);
extern int new_field(char *spec);
extern int old_field(char *spec0, char *spec1);
extern int compile_fields(char *, int *);
extern int global_field();

 
extern int unique_keys;
extern FILE *verbose;
extern FILE *debugging;

#endif
