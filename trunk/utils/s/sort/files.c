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

	input.c:

	these routines manage the input streams for sort. 
	There are 2 input choices, get_text() and get_ldesc().

	get_text() reads a line of text, builds a table delineating
	the fields, and puts both into a 'linedesc' structure.

	get_ldesc() reads a linedesc structure from a file -- used
	during merge-sort operations.

	compiling:

	DEBUGGING enables a routine print_ln which prints, on stdout,
		the contents of a linedesc structure.

	STAND_ALONE enables print_ln & main & compile_fields to allow
		this module to run without linkages for testing.

*/


#include	"sort.h"




int ungetline(fdesc *f, linedesc *l)
{
	if (l == NULL) {
		return 0;
	}
	free(l);
	f->state |= BUF_VALID;
	return 1;
}



static linedesc	*get_text(fdesc *f)
{
	linedesc        *p;
	int              t;
	int              nfld;
	if (AT_EOF(f))
		return NULL;
	
	nfld = get_nfields();

	if (NEED_INPUT(f)) {
		register int	i;
		register int	c = 0;
		for (i=0; i < SORT_LINE_MAX && (c=getc(f->f)) != EOF;)
			if ((f->buffer[i++] = c) == '\n'){
				i--;
				if(f->buffer[i-1] == '\r')
					i--;
				break;
			}
		if (i == 0 && c == EOF) {
			f->state |= END_INPUT;
			return 0;
		}
		f->buffer[i] = 0;
		f->state |= BUF_VALID;
	}
	t = strlen(f->buffer);
	if ((p=linealloc(nfld,t)) == NULL) {
		return NULL;
	}
	f->state &= ~BUF_VALID;
	p->len = t;
	p->str_start = nfld*sizeof(int)*2;
	strcpy(STR_BEGIN(p),f->buffer);
	compile_fields(STR_BEGIN(p),p->data.stab);
	return p;
}




fdesc *open_fdesc(FILE *f,int type)
{
fdesc	*fd;

	if ((fd=calloc(sizeof(fdesc),1)) == NULL) {
		return NULL;
	}
    fd->f = f;
	fd->state = 0;
	if (type != 0) {
		fprintf(stderr,"ERROR: UNSUPPORTED  FUNCTION CALLED .....\n");
		exit(2);
	}
	fd->readline = get_text;
	return fd;
}



int close_fdesc(fdesc *p)
{
	fclose(p->f);
	free(p);
	return 1;
}




#if defined(DEBUGGING) || defined(STAND_ALONE)

print_ln(linedesc *p)
{
int	i;

	printf("len = %d, start_str = %d\n",STR_LEN(p),p->str_start);
	printf("input = <%s>\n",STR_BEGIN(p));
	for (i=0; i < get_nfields(); i++) {
		printf("field %d : [<%*.*s>]\n",i,STR_FLDLEN(p,i),STR_FLDLEN(p,i),STR_FLD(p,i));
#if 0
		printf("field %d : begin %d end %d\n",i,p->data.stab[i*2],p->data.stab[i*2+1]);
#endif
	}
}

#endif

#if defined(STAND_ALONE)
get_nfields()	{	return nfields;	}
int	nfields = 20;

/*	garbage just to set it up.... */
compile_fields(char *s, int *p)
{
int	i;
	for (i=0; i < nfields; i++) {
		p[i*2] = i;
		p[i*2+1] = -i;
	}
}

main(int argc, char **argv)
{
linedesc *p;
fdesc *f;

	if ((f=open_fdesc(stdin,0)) == NULL)
		exit(2);
	while ((p=(*(f->readline))(f)) != NULL) {
		print_ln(p);
	}
}
#endif
