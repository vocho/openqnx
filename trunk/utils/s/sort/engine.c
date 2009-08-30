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

engine.c:	a sort engine.

int fcompare(linedesc **p1,linedesc **p2);
	returns p1 <= p2.
	calls the appropriate comparison function for each field in each
	line.

static int compare_field(linedesc *p1, int field1, linedesc *p2, int field2);
	compare_field return:
		p1 == p2 ->    0
		p1 <  p2 ->   <0
		p1 >  p2 ->   >0
	The field flags are checked for the various conditions (ie.
	nocase, nospace, .....). 
	If the flags are numeric, 
static	compare_numeric(char *s1, char *s2, int lim1, int lim2)
	is called.
	If the flags are "backwards" (special for rob oakley)
static	compare_backwards(char *s1, char *s2, int lim1, int lim2)
	is called.

int merge_sort(FILE *outfile, fdesc *ftab, int nfiles)
	merges 'nfiles' into a single output file.  The input files are assumed
	to be sorted, and merge_sort() will provide an inappropriate output if
	they are not.

int file_ordered(fdesc *fd)
	returns non-zero if the file is "ordered", 0 if not.
	(empty files are ordered).

write_file(FILE *f, linedesc **lines, int nentries)
	writes a set of lines out to file f.
	checks the external variable (unique_keys) to "squeeze" identical 
	lines.

*/

#include	"sort.h"
#include	<malloc.h>
#include	<locale.h>





static	int compare_backwards(char *s1, char *s2, int lim1, int lim2)
{
int i,j;
#ifdef DEBUGGING
	fprintf(stderr,"comparing_back(%*.*s to %*.*s)\n",
		lim1,lim1,s1,lim2,lim2,s2);
#endif
	for (i=lim1,j=lim2; i && j && s1[i] == s2[j]; i--,j--)
		;
	switch ((i != 0) | (j != 0)<<1) {
	case 0:	return 0;
	case 1: return -1;
	case 2: return 1;
	case 3: return s1[i] - s2[j];
	}
	return -1;
}

static int
compare_numeric(char *s1, char *s2, int lim1, int lim2)
{
	double    v0, v1;
	char      save1, save2;

	save1 = s1[lim1];
	save2 = s2[lim2];

	s1[lim1] = 0;
	s2[lim2] = 0;

#ifdef DEBUGGING
	if (debugging) {
		fprintf(debugging,"compare_num %*.*s(%f) vs %*.*s(%f)\n",
		                 lim1,lim1,s1, strtod(s1,NULL),
		                 lim2,lim2,s2, strtod(s2,NULL));
	}
#endif


/*
   This used to return the difference, but integral conversion in
   16 bit caused far-separated values to get tromped.
*/
	v0 = strtod(s1, 0); v1 = strtod(s2, 0);
	s1[lim1] = save1;
	s2[lim2] = save2;
	return v0==v1?0:v0>v1?1:-1;
}

#if 0
static int
compare_integral(char *s1, char *s2, int lim1, int lim2)
{
#ifdef DEBUGGING
	if (debugging) {
		fprintf(debugging,"compare_num %*.*s(%f) vs %*.*s(%f)\n",
		                 lim1,lim1,s1, strtol(s1,NULL,10),
		                 lim2,lim2,s2, strtol(s2,NULL,10));
	}
#endif
	return strtol(s1,NULL,0) - strtol(s2,NULL,0);
}
#endif





static int
compare_field(p1,f1,p2,f2)
linedesc	*p1, *p2;
int			f1, f2;
{
char	*s1, *s2;
int		mlen, tlen;
int	t0,t1;

int	flags;

	flags = get_flags(f1) | get_flags(f2);
#ifdef DEBUGGING	
if (debugging) {
	fprintf(debugging,"comparing fields: [%s] to [%s]\n",STR_FLD(p1,f1),STR_FLD(p2,f2));
	fprintf(debugging,"lengths = [%d],[%d]\n",STR_FLDLEN(p1,f1),STR_FLDLEN(p2,f2));
}
#endif
	s1 = STR_FLD(p1,f1);
	s2 = STR_FLD(p2,f2);
	mlen = STR_FLDLEN(p1,f1);
	tlen = STR_FLDLEN(p2,f2);

	if (flags & FLD_NUMERIC) {
		t0 = compare_numeric(s1,s2,mlen,tlen);
		return (flags & FLD_REVERSE) ? -t0 : t0;
	} else if (flags & FLD_BACKWARDS) {
		t0 = compare_backwards(s1,s2,mlen,tlen);
		return (flags & FLD_REVERSE) ? -t0 : t0;
	}
	t0 = 0; /* sometimes we get no chars */
	while (mlen > 0 && tlen > 0) {
		if (flags & FLD_NOPUNCT) {
			if (ispunct(*s1)) {
				s1++;
				mlen--;
				continue;
			}
			if (ispunct(*s2)) {
				s2++; 
				tlen--;
				continue;
			}
		}
		if (flags & FLD_NOBLANKS) {
			if (isspace(*s1)) {
				s1++;
				mlen--;
				continue;
			}
			if (isspace(*s2)) {
				s2++;
				tlen--;
				continue;
			}
		}
		if (flags & FLD_PRTCHARS) {
			if (!isprint(*s1)) {
				s1++; mlen--; continue;
			}
			if (!isprint(*s2)) {
				s2++; tlen--; continue;
			}
		}
		t0 = *s1;
		t1 = *s2;

if (debugging) {
	fprintf(debugging,"comparing chars: [%c] to [%c]\n",t0, t1);
}
		if ((flags & FLD_NOCASE)) {
			if (islower(t0))
				t0 = toupper(t0);
			if (islower(t1))
				t1 = toupper(t1);
		}
		if ((t0 -= t1) != 0) {
			break;
		}
		s1++; s2++;
		mlen--;
		tlen--;
	}
	if(t0 == 0) t0 = mlen - tlen;

	return (flags & FLD_REVERSE) ? -t0 : t0;
}



/*	
	returns a 0 or 1 based on whether the lines should be swapped ,
	early out for speed.
*/

int	fcompare(const void *v1, const void *v2)
{
int	i;
int	nfld;
int	t = 0;
linedesc *const *p1 = v1;
linedesc *const *p2 = v2;
	nfld = get_nfields();
	for (i=0; i < nfld; i++) {
		if ((t=compare_field(*p1,i,*p2,i)) != 0) {
			break;
		}
	}
#ifdef DEBUGGING
if (debugging) {
	fprintf(debugging,"comparing [%s] to [%s] = %d\n",STR_BEGIN(*p1),STR_BEGIN(*p2),t);
}
#endif
	return t;
}




int merge_files(outfile, filelist, nfiles)
FILE	*outfile;
fdesc	**filelist;
int		nfiles;
{
/*
	the merge algorithm looks like this:
		(f: output file).
		(g[n] : input files).
		(p : inptr).

	find first file with more data
*/

int	i;
int	t;
linedesc	**linetab;
linedesc	*temp;
int	r;

	if ((linetab = alloca(sizeof *linetab * nfiles)) == NULL) {
		fprintf(stderr,"no room on stack for merge\n");
		exit(2);
	}
	
	for (i=0; i < nfiles; i++) {
		linetab[i] = INPUT_LINE(filelist[i]);
	}

	while (1) {
		for (i=0; i < nfiles && linetab[i] == NULL; i++) 	
			;
		if (i == nfiles)	/* all files exhausted */
			break;
		t = i;
		while (++i < nfiles) {			
			if (linetab[i] == NULL)
				continue;
			if ((r=fcompare(&linetab[t],&linetab[i])) == 0) {
				if (unique_keys) {
					while ((temp=INPUT_LINE(filelist[i]))) {
						if (fcompare(&linetab[i],&temp))
							break;
						line_free(temp);
					}
					line_free(linetab[i]);
					linetab[i] = temp;
					i--;
				} 
			} else if (r > 0) {
				t = i;
			}
		}
		fwrite(STR_BEGIN(linetab[t]),1,linetab[t]->len,outfile);
		fprintf(outfile,"\n");
		while ((temp=INPUT_LINE(filelist[t]))) {
			if (!unique_keys || fcompare(&linetab[t],&temp))
				break;
			line_free(temp);
		}
		line_free(linetab[t]);
		linetab[t] = temp;
	}
	return 1;
}


int
file_ordered(fdesc *fd)
{
linedesc	*l[2];
	
	if ((l[0] = INPUT_LINE(fd)) == NULL) {
		return 1;	/* empty files are ordered? */
	}
	while ((l[1] = INPUT_LINE(fd)) != NULL) {
		if (fcompare(&l[0],&l[1]) <= 0) {
			line_free(l[0]);
			l[0] = l[1];
		} else {
#ifdef DEBUGGING
			fprintf(stderr,"line mismatch: '%s' - '%s' = %d\n",
				STR_BEGIN(l[0]), STR_BEGIN(l[1]), fcompare(&l[0],&l[1]));
#endif
			break;
		}
	}
	line_free(l[0]);
	if (l[1] == NULL) {
		return 1;
	}
	line_free(l[1]);
	return 0;		/*	not ordered */
}


int write_file(FILE *f, linedesc **lines, int nentries)
{
int	i;

	if (nentries == 0)
		return 0;
	if (unique_keys) {
	int	mark = 0;
		fwrite(STR_BEGIN(lines[0]),1,lines[0]->len,f);
		fprintf(f,"\n");
		for (i=1; i < nentries; i++) {
			if (fcompare(lines+mark,lines+i) == 0) {
				line_free(lines[i]);
				lines[i] = NULL;
				continue;
			} 
			fwrite(STR_BEGIN(lines[i]),1,lines[i]->len,f);
			fprintf(f,"\n");
			line_free(lines[mark]);
			lines[mark] = NULL;
			mark = i;
		}
		if (lines[mark]) {
			line_free(lines[mark]);
			lines[mark] = NULL;
		}
	} else {
		for (i=0; i < nentries; i++) {
			fwrite(STR_BEGIN(lines[i]),1,lines[i]->len,f);
			fprintf(f,"\n");
			line_free(lines[i]);
			lines[i] = NULL;
		}
	}
	return i;
}

/*
 * some of the following source should be integrated in a later version
 * of sort.   The additions (would|should) be:
 *   - floating point comparisons.
 *   - locale conventions.
 *
 * The shell sort is left for reference
 */
#if 0
static	long todec(char *s, int lim, struct lconv *p)
{
int	i;
char	r[25 + 1];
int	j = 0;

	for (i=0; i < lim && isspace(s[i]); i++)
		;
	
	while (i < lim && j < 25) {
		if (isdigit(s[i]) || isspace(s[i]) || s[i] == 'x' || s[i] == 'X') {
         	r[j++] = s[i++];
			continue;
		}
		if (s[i] == *(p->thousands_sep)) {
			i++;
			continue;
		}
		break;
	}
	r[i] = '\0';
	return strtol(r,NULL,0);
}

static	int compare_numeric(char *s1, char *s2, int lim1, int lim2)
{
long t;
struct	lconv	*p;
	if ((p=localeconv()) == NULL)
		return 0;		/*	equivalent */
	if (!(t = todec(s1,lim1,p) - todec(s2,lim2,p)))
		return 0;
	return t > 0 ? 1 : -1;
}
/*
	use a simple shell sort, can be replaced by heap or quick sort
	if required.

*/

#include <malloc.h>

void shellsort(linedesc **lines, int n, int siz, int (*f)(void *, void *))
{

int	gap,i,j;
void	*temp;
	temp = alloca(siz);
	for (gap=n/2; gap > 0; gap /= 2) {
		for (i=gap; i < n; i++) {
			for (j=i-gap; j >= 0; j -= gap) {
				if ((*f)(&lines[j],&lines[j+gap]) > 0) 
					break;
				memcpy(temp,lines[j], siz);
				memcpy(lines[j],lines[j+gap],siz);
				memcpy(lines[j+gap],temp,siz);
			}
		}
	} 
}

#endif
