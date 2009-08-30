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





/*- $Id: fields.c 211815 2009-01-25 17:01:08Z rmansfield@qnx.com $
 
 
 description: This module contains routines used to manage the field
 descriptions for sort.  there are two processes - 1: transform the
 user-notation into	a more useful representation 2: given a string & the
 field specifications, generate a list of field positions within the
 string.
 
 
 static char *skip_to_field(char *s, int fieldno)

 Given a string and a fieldno, return a pointer to the
 beginning of the fieldno.  Return pointer to end-of-string
 if exhausted before fieldno.
 
 static char *skip_prefix(char *s, int typeflags)

 Given a string and typeflags, return pointer to the first
 char we are 'interested' in s.  Return end-of-string if exhausted.
 
 
 compile_fields(char *s, int *fldtab)

 -using an internal store of field descriptions,
 compile_fields applies the description to the string,
 storing the start and end positions in adjacent entries within 'fldtab'.
 This preprocessing of the strings saves recalculation when actual sorting
 comparisons are made.

 draft9_field(char *s)
 new_field(char *s)
 old_field(char *f0, char *f1)

 Add a new field description based upon either "traditional" sort from
 UNIX, or Posix/2 Draft9 or Posix/2 Std.

 $Log$
 Revision 1.5  2005/06/03 01:38:00  adanko
 Replace existing QNX copyright licence headers with macros as specified by
 the QNX Coding Standard. This is a change to source files in the head branch
 only.

 Note: only comments were changed.

 PR25328

 Revision 1.4  2005/02/03 21:45:18  mshane
 Removed somewhat offensive comments at the request of dkeefe.
 CR:Marcind

 Revision 1.3  2003/08/29 21:01:38  martin
 Add/Update QSSL Copyright.

 Revision 1.2  2001/05/28 16:20:01  kewarken
 changed to make ordering deterministic

 Revision 1.1  1998/10/16 20:06:31  dtdodge
 Adding sort to CVS respository

 Revision 1.3  1998/10/16 19:52:51  dtdodge
 Checkin for CVS

 */

#include	"sort.h"
#if !defined(__MINGW32__)
#include <err.h>
#endif


#ifndef	MAXFLDS
#define	MAXFLDS	40
#endif


#ifndef	CHAR_MAX
#define	CHAR_MAX	255
#endif

extern FILE    *debugging;

static char     fstab[CHAR_MAX + 1];

/*
 * fs_unique means any string of field separators is unique, otherwise, each
 * field separator is the beginning of a new field.
 */

static int      fs_unique = 0;

static char    *class_names[] = {
	"[:alpha:]",		/* isalpha */
	"[:upper:]",		/* isupper */
	"[:lower:]",		/* islower */
	"[:digit:]",		/* isdigit	 */
	"[:xdigit:]",		/* isxdigit */
	"[:alnum:]",		/* isalnum */
	"[:space:]",		/* isspace */
	"[:punct:]",		/* ispunct */
	"[:print:]",		/* isprint */
	"[:graph:]",		/* isgraph */
	"[:cntrl:]",		/* iscntrl */
	NULL
};


//#ifdef __QNXNTO__
/*
	stresc:	extract next character from a string, translating appropriate
			escape sequences.

	int stresc(char *string, char **endptr)
		stresc starts returns the next character from 'string', updating
		endptr to point at next character to extract.  If the character
		starts with the escape character '\', then the escape sequence is
		extracted and the resultant value is returned.   If the next character
		is end-of-string, the value EOF is returned.  This distinguishes an
		actual end-of-string condition from an interpretation of \x00 or
		\000 ...
		by default the following escape sequences are recognized:
			'\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\', '\"'.
		as well as
			'\[0-7]{1,3}'
		and
			'\x[0-9a-bA-B]{1,2}'
		notice that, in keeping with ansi xj11, 8 & 9 are NOT octal digits.
		The first set of strings may be exchanged using 'set_escape_string'
		function.  If they are set to NULL, no escape character mapping will
		be attempted (although the octal & hex mapping are maintained).
		The above string is scanned first, before looking for octal or hex	
		strings, so putting [xX0-9] in the key string should be used with
		caution, as it will superseed scanning for the numeric sequences.


	set_escape_string(const char *match_string, const char *esc_string)
		set_escape_string controls the escape sequence matching.  The first
		string, match_string, specifies the characters which follow the
		'\' character.  esc_string contains the value to return for it.
		The strings are joined by matching indices.


	Steve McPolin

*/

#define	DEFAULT_MATCH_STRING	"abfnrtv\\\'\""
#define	DEFAULT_ESC_STRING		"\a\b\f\n\r\t\v\\\'\""



static	char	*esc_key = DEFAULT_MATCH_STRING,
				*esc_rpl = DEFAULT_ESC_STRING;


int	set_escape_string(char *match, char *xlat)
{
	esc_key = match;
	esc_rpl = xlat;
	return 1;
}



#define	add_hex(_lval,_h)	(((_lval) << 4) + (_h))
#define	add_oct(_lval,_o)	(((_lval) << 3) + (_o))

static	int	rplexc(int	c)
{
char	*p;

	if (esc_key == NULL)
		return -1;
	for (p=esc_key; *p && *p != c; p++)
		;
	if (!*p)
		return -1;
	return esc_rpl[p-esc_key];
}
	

int stresc(char *s, char **update)
{
int		x;
int		i = 0;	
int		c;
int		ival;

	/*	end of string */
    if (!(c=*s++)) {
		if (update)	*update = s-1;
		return EOF;
	}
	/*	normal character, no escape. */
	if (c != '\\') {
		if (update)	
			*update = s;
		return c;
	}
	if ((ival=rplexc(c=*s++)) > 0) {
		if (update)		
			*update = s;
		return ival;
	}

	if (toupper(c) == 'X') {		/*	try for hex const */
		x =c;
		ival = 0;
		for (i=0; i < 2 && isxdigit(c=s[i]); i++)
			ival = add_hex(ival,c <= '9' ? c-'0' : toupper(c)-'A'+10);
		if (update)	*update = s+i;	
		return i == 0 ? x : ival;		
	}
	if (isdigit(c) && c < '8'){
		s--;
		ival = 0;
		for (i=0; i < 3 && isdigit(c=s[i]) && c < '9'; i++) {
			ival = add_oct(ival,c-'0');
		}
		if (update) *update = s+i;
		return ival;
	}
	/*	just an overly cautious person ;-) */
	if (update)	*update = s;
	return c;
}

//#endif


int 
newfs(char *fspec, int unique)
{
	int             class_id, i;
	char           *p;

	fs_unique = unique;
	memset(fstab, 0, sizeof(fstab));
	if (*fspec == '[' && fspec[1] == ':') {	/* could be start sequence */
		for (i = 0; class_names[i] != NULL; i++) {
			if (strcmp(fspec, class_names[i]) == 0)
				break;
		}
		if (class_names[class_id = i] == NULL) {
			return -1;
		}
		for (i = 0, p = fstab; i <= CHAR_MAX; i++, p++) {
			switch (class_id) {
			case 0: *p = isalpha(i) != 0; break;
			case 1: *p = isupper(i) != 0; break;
			case 2: *p = islower(i) != 0; break;
			case 3: *p = isdigit(i) != 0; break;
			case 4: *p = isxdigit(i) != 0; break;
			case 5: *p = isalnum(i) != 0; break;
			case 6: *p = isspace(i) != 0; break;
			case 7: *p = ispunct(i) != 0; break;
			case 8: *p = isprint(i) != 0; break;
			case 9: *p = isgraph(i) != 0; break;
			case 10: *p = iscntrl(i) != 0; break;
			default:
				break;
			}
		}
	} else {
		i = stresc(fspec, NULL);
		fstab[i & CHAR_MAX] = 1;
	}

	return(0);
}

#define isfs(_c)	(fstab[(_c)&CHAR_MAX])

void print_fs()
{
	int             i;
	if (debugging) {
		for (i = 0; i < CHAR_MAX; i++) {
			if (isfs(i))
				fprintf(debugging, "fs = %d (%c)\n", i, i);
		}
	}
}


static int      nfields = 0;	/* global -- number of fields */
static struct fieldspec fieldtab[MAXFLDS];

static int      global_flag = 0;


int 
add_flag(int flag)
{
	char           *p;
	char           *base = ORDER_RULES;

	if ((p = strchr(base, flag)) == NULL)
		return 0;
	return (global_flag |= 1 << (p - base));
}

int 
get_flags(int fldno)
{
	const struct fieldspec *fsp = fieldtab + fldno;
	return fldno < nfields ? (fsp->start[FLD_TYP] | fsp->end[FLD_TYP])
		: 0;
}

#define	GET_NFIELDS()	nfields

int 
get_nfields(void)
{
	return GET_NFIELDS();
}

int 
alloc_field(void)
{
	return nfields < MAXFLDS ? nfields++ : -1;
}

int 
dealloc_field(void)
{
	return nfields ? --nfields : -1;
}


/*
 * starting at s, return the start position of the 'nth' field. note that an
 * initial string of "fs" is considered part of field 0 "fs_unique".
 */

static char    *
field_start(char *s, int fldno)
{
	while (fldno-- && *s) {
		/* look forward for one */
		if (fs_unique) {
			while (*s && isfs(*s))
				s++;
		}
		while (*s && !isfs(*s))
			s++;	/* find one */
		if (*s)
			s++;
	}
	return s;
}


static char    *
skip_prefix(char *s, int typeflag)
{
	while (*s) {
		if ((typeflag & FLD_NOBLANKS) && isspace(*s)) {
			s++;
		} /* else if ((typeflag & FLD_NOPUNCT) && ispunct(*s)) {
			s++;
		} */ else if ((typeflag & FLD_PRTCHARS) && !isprint(*s)) {
			s++;
		} else {
			break;
		}
	}
	return s;
}

static int
get_field(char *s, struct fieldspec * fs, int *tab)
{
	char           *p, *p0;
	int             t;
	if (fs->start[FLD_NO] == -1 && fs->end[FLD_NO] == -1) {
		*tab++ = skip_prefix(s, fs->start[FLD_TYP]) - s;
		*tab = strlen(s);
		return 1;
	}
	p0 = p = field_start(s, fs->start[FLD_NO]);
	if (*p) {		/* the field was found */
		if (fs_unique && *p) {
			while (*p && isfs(*p))
				p++;
		}
		p = skip_prefix(p, fs->start[FLD_TYP]);
		for (t = 0; t < fs->start[FLD_OFFS] && *p; t++, p++);
	}
	*tab++ = p - s;
	p = field_start(p0, fs->end[FLD_NO] - fs->start[FLD_NO]);
	if (*p) {
		if (fs_unique && *p) {
			while (*p && isfs(*p))
				p++;
		}
		p = skip_prefix(p, fs->end[FLD_TYP]);
		for (t = 0; t < fs->end[FLD_OFFS] && *p; t++, p++);
	}
	*tab = p - s;
	return 1;
}





int
compile_fields(char *s, int *fldtab)
{
	int             i;
	int             max;
	for (i = 0, max = GET_NFIELDS(); i < max; i++) {
		get_field(s, fieldtab + i, fldtab + 2 * i);
	}
	if (debugging) {
		fprintf(debugging, "str = %s\n", s);
		for (i = 0; i < max; i++) {
			int             x, y;
			x = fldtab[2 * i];
			y = fldtab[2 * i + 1];
			fprintf(debugging, "field %d{%d,%d} = [%*.*s]\n", i, x, y,
				y - x, y - x, s + x);
		}
	}
	return 1;
}



static  int
build_map(char *base, char *source, char **update)
{
	int             bitmap = 0;
	char           *p;
	while (*source && (p = strchr(base, *source)) != NULL) {
		bitmap |= 1 << (p - base);
		source++;
	}
	if (update)
		*update = source;
	return bitmap;
}



static char    *
extract_spec(char *s, int *itab, int *override)
{
	while (isspace(*s))
		s++;
	if (!isdigit(*s))
		return NULL;
	itab[FLD_NO] = (int) strtol(s, &s, 10);
	if (override != NULL)
		override[FLD_NO] = 1;
	if (*s == '.') {
		itab[FLD_OFFS] = (int) strtol(s + 1, &s, 10);
		if (override != NULL)
			override[FLD_OFFS] = 1;
	} else {
		itab[FLD_OFFS] = 0;
	}
	itab[FLD_TYP] = build_map(ORDER_RULES, s, &s) | global_flag;
	return s;
}

int
global_field()
{
	struct fieldspec	*fs;
	int			t;
	if ((t=alloc_field()) < 0) {
		return -1;
	}
	fs = fieldtab + t;
	fs->start[FLD_NO]	= -1;
	fs->start[FLD_OFFS]	= 0;
	fs->start[FLD_TYP]	= global_flag;
	fs->end[FLD_NO]		= -1;
	fs->end[FLD_OFFS]	= 0;
	fs->end[FLD_TYP]	= global_flag;
	return 0;
}

/*-
 * Between POSIX draft 9 and release, the specifications were changed.
 * The field "x.m,y.n" means:
 *
 * - draft 9:  From the field starting at the "mth" character after the
 *             "xth" field separator to the "nth" character after the
 *             "y+1th" field separator.
 *
 * Now it reads:  From the "mth" character in the "xth" field to the
 *             "nth" character in the "yth" field".
 *
 * The draft-9 definition was more than a bit bizarre.
 *
 * Anyways, we support now the "old" field specification
 *  "+x.m -y.n", which is based at 0.
 * The "old new" (draft 9) specification, which is based at 0
 * and the "new new" (POSIX) specification, which is based at 1
 */

int
draft9_field(char *s)
{
	struct fieldspec *fs;
	int             t;

	if ((t = alloc_field()) < 0) {
		return 0;
	}
	fs = fieldtab + t;

	fs->start[FLD_NO]	= 0;
	fs->start[FLD_OFFS]	= 0;
	fs->start[FLD_TYP]	= 0;
	if ((s = extract_spec(s, fs->start, NULL)) == NULL) {
		dealloc_field();
		return -1;
	}
	/*- end of field */
	fs->end[FLD_NO] = fs->start[FLD_NO] + 1;
	fs->end[FLD_OFFS] = 0;
	fs->end[FLD_TYP] = fs->start[FLD_TYP];
	if (*s == ',') {
		if ((s = extract_spec(s + 1, fs->end, NULL)) == NULL) {
			dealloc_field();
			return -1;
		}
		fs->end[FLD_NO] += 1;
	}
	if (debugging) {
		fprintf(debugging,
		"field [fno %d, start %d, type%x ... fno %d, end %d, type %x]\n",
		fs->start[FLD_NO], fs->start[FLD_OFFS], fs->start[FLD_TYP], 
		fs->end[FLD_NO], fs->end[FLD_OFFS], fs->end[FLD_TYP]);
	}
	return t;
}

int
new_field(char *s)
{
	struct fieldspec *fs, override;
	int             t;

	if ((t = alloc_field()) < 0) {
		return 0;
	}

	memset(&override, 0x00, sizeof(override));

	fs = fieldtab + t;

	fs->start[FLD_NO]	= 0;
	fs->start[FLD_OFFS]	= 0;
	fs->start[FLD_TYP]	= 0;
	if ((s = extract_spec(s, fs->start, override.start)) == NULL) {
		dealloc_field();
		return -1;
	}
	if (override.start[FLD_NO] && fs->start[FLD_NO] <= 0) {
		errx(1, "field numbers must be positive");
	}
	if (override.start[FLD_OFFS] && fs->start[FLD_OFFS] <= 0) {
		errx(1, "illegal offset");
	}
	if (fs->start[FLD_NO]) fs->start[FLD_NO] -= 1;
	if (fs->start[FLD_OFFS]) fs->start[FLD_OFFS] -= 1;

	/*- end of line */
	fs->end[FLD_NO] = -1;
	fs->end[FLD_OFFS] = 0;
	fs->end[FLD_TYP] = fs->start[FLD_TYP];
	if (*s == ',') {
		if ((s = extract_spec(s + 1, fs->end, override.end)) == NULL) {
			dealloc_field();
			return -1;
		}
		if (override.end[FLD_NO]) {
			if (fs->end[FLD_NO] <= 0) {
				errx(1, "field numbers must be positive");
			}
			fs->end[FLD_NO]--;
		}
		if (override.end[FLD_OFFS]) {
			/* Zero is valid on end offset (end of field) */
			if (fs->end[FLD_OFFS] < 0) {
				errx(1, "illegal offset");
			}
		}
	}

	if (fs->end[FLD_NO] == fs->start[FLD_NO] && fs->end[FLD_OFFS] == 0) {
		fs->end[FLD_NO]++; /* End of start field */
	}
	if (debugging) {
		fprintf(debugging,
		"field [fno %d, start %d, type%x ... fno %d, end %d, type %x]\n",
		fs->start[FLD_NO], fs->start[FLD_OFFS], fs->start[FLD_TYP], 
		fs->end[FLD_NO], fs->end[FLD_OFFS], fs->end[FLD_TYP]);
	}
	return t;
}

int
old_field(char *f0, char *f1)
{
	struct fieldspec	*fs;
	int			t;
	int			count = 0;

	if (*f0 != '+' || !isdigit(f0[1])) {
		return -1;
	} else {
		f0++;
	}
	if (!f1 || *f1 != '-' || !isdigit(f1[1])) {
		f1 = "";
	} else {
		f1++;
		count++;
	}

	if ((t = alloc_field()) < 0) {
		return 0;
	}
	fs = fieldtab + t;

	fs->start[FLD_NO]	= 0;
	fs->start[FLD_OFFS]	= 0;
	fs->start[FLD_TYP]	= 0;
	if (extract_spec(f0, fs->start, NULL) == NULL) {
		dealloc_field();
		return -1;
	}
	/*- default to end of line */
	fs->end[FLD_NO]		= -1;
	fs->end[FLD_OFFS]	= 0;
	fs->end[FLD_TYP]	= fs->start[FLD_TYP];
	if (*f1 && extract_spec(f1, fs->end, NULL) == NULL) {
		dealloc_field();
		return -1;
	}
	if (debugging) {
		fprintf(debugging, "field [fno %d, start %d, type=%x... fno %d, end %d, type %x]\n",
			fs->start[FLD_NO], fs->start[FLD_OFFS], fs->start[FLD_TYP], 
			fs->end[FLD_NO], fs->end[FLD_OFFS], fs->end[FLD_TYP]);
	}
	return count;
}



#if defined(STAND_ALONE) || defined(DEBUGGING)

void
put_spec(s, p)
	char           *s;
	struct fieldspec *p;
{
	fprintf(debugging, "%s ", s);
	fprintf(debugging, "start = {%d,%d,%d}\n", p->start[0], p->start[1], p->start[2]);
	fprintf(debugging, "end = {%d,%d,%d}\n", p->end[0], p->end[1], p->end[2]);
}

void
dump_fields()
{
	int             i;
	for (i = 0; i < get_nfields(); i++)
		put_spec("field: ", fieldtab + i);
}

#endif

#ifdef STAND_ALONE
FILE           *debugging;

main(int argc, char **argv)
{
	char            buffer[128];
	int             i;
	int             t;
	int             itab[256];

	debugging = stdout;
	newfs("[:space:]", 0);
	while (--argc > 0) {
		t = add_fieldspec(*++argv, 0);
		put_spec("spec:\n", &fieldtab[t]);
	}
	dump_fields();
	while (gets(buffer)) {
		compile_fields(buffer, itab);
		fprintf(debugging, "str=%s\n", buffer);
		for (i = 0; i < nfields; i++) {
			fprintf(debugging, "{%d,%d}\n", itab[i * 2], itab[i * 2 + 1]);
		}
	}
}

#endif

