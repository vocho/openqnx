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





#include <malloc.h>
#include <stdio.h>
#include <util.h>

static char *__fgl_line;
static size_t __fgl_bufsize;

char *
fgetln_r(FILE *stream, size_t *lenp, char **bufp, size_t *buflenp)
{
	int base_size, p_size, len_tot, ndone; 
	char *base, *p;
	off_t pos_prev, pos_cur;
	char *temp;

	if (bufp == NULL) {
		bufp = &__fgl_line;
		buflenp = &__fgl_bufsize;
	}

	base = *bufp;
	base_size = *buflenp;

	p = base;
	len_tot = 0;

	flockfile(stream);

	pos_prev = ftello(stream);

	for (;;) {
		/*
		 * fgets add a '\0'.
		 * Need signed comparison for first time through.
		 */
		if (len_tot >= base_size - 1) {
			/* need more room */
			if ((temp = realloc(base, base_size + BUFSIZ)) == NULL) {
				len_tot = 0;
				break;
			}

			base = temp;
			base_size += BUFSIZ;
		}

		p = base + len_tot;
		p_size = base + base_size - p;

		if (fgets(p, p_size, stream) == NULL)
			break;

		pos_cur = ftello(stream);

		ndone = pos_cur - pos_prev;
		pos_prev = pos_cur;

		len_tot += ndone;

		/*
		 * checking for eof avoids an unecessary realloc()
		 * in the high runner case: more often file will end 
		 * without a newline than it will have lines longer
		 * than BUFSIZ,
		 */
		if (p[ndone - 1] == '\n' || feof(stream))
			break;
	} 

	funlockfile(stream);

	*bufp = base;
	*buflenp = base_size;
		
	if (len_tot) {
		*lenp = len_tot;
		return (base);
	}

	return (NULL);
}

char *
fgetln(FILE *stream, size_t *len)
{
	return (fgetln_r(stream, len, NULL, NULL));
}
