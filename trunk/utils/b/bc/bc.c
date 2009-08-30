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




#include <libc.h>
#include <stdarg.h>
#include "bc.h"
#include "number.h"

int             trace = 0;
int             list = 0;
static int      scale = BC_DEFAULT_SCALE;	/* default scale = 0 */
static int      ibase = 10;	/* default input base = 0 */
static int      obase = 10;	/* default output base = 0 */

#ifndef    BC_LIB_FILE
#define    BC_LIB_FILE    "/usr/lib/bclib.b"
#endif
static char    *lib_name = BC_LIB_FILE;



int get_scale()
{
	return scale;
}

int set_scale(int n)
{
	if (n < 0) {
		return -1;
	}
	return (scale = n) > BC_SCALE_MAX ? BC_SCALE_MAX : n;
}

int get_ibase()
{
	return ibase;
}

int set_ibase(int n)
{
	if (n <= 0 || n >= BC_IBASE_MAX)
		return ibase = 10;
	return ibase = n;
}

int get_obase()
{
	return obase;
}

int set_obase(int n)
{
	if (n <= 1 || n >= BC_OBASE_MAX)
		return obase = 10;
	return obase = n;
}

static int      exit_code = 0;

void 
program_exit()
{
	exit(exit_code);
}

void fatal()
{
	exit_code = -1;
	program_exit();
}

void warning()
{
	exit_code++;
}


void 
no_mem(char *s)
{
	if (s != NULL)
		error(1, "fatal error %s: no memory\n", s);
	exit_code = -1;
	program_exit();
}

int error(int ecode, char *fmt,...)
{
	va_list         va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	if (ecode)
		fatal();
	else
		warning();
	return 0;
}

int bc_printf(char *fmt,...)
{
	va_list         va;
	va_start(va, fmt);
	return fprintf(stdout, fmt, va);
}

int print_num(numb_t * p, int obase, int eol)
{
	int             t;
	int             i = 0;
	static char     outbuf[LINE_MAX];
	if ((t = numtos(p, outbuf, sizeof outbuf - 2, obase)) < 0) {
		error(0, "internal: could not convert number\n", p);
		fputs("********.***", stdout);
		if (eol)
			putchar('\n');
		fflush(stdout);
		return 12;
	}
	if (eol && t < sizeof outbuf - 2) {
		outbuf[t++] = '\n';
	}
	while (1) {
		if (t - i > BC_OUT_LINE_MAX) {
			fwrite(outbuf + i, 1, BC_OUT_LINE_MAX, stdout);
			fputs("\\\n", stdout);
			i += BC_OUT_LINE_MAX;
		} else {
			fwrite(outbuf + i, 1, t - i, stdout);
			i += t;
			break;
		}
	}
	fflush(stdout);
	return t;
}





int main(int argc, char **argv)
{
	int             i;
	extern int      init_eval(void);
	extern int      do_bc(void);
	extern int      pushf(const char *fname);

	init_eval();
	while ((i = getopt(argc, argv, "clt")) != EOF) {
		switch (i) {
		case 'l':
			if (pushf(lib_name)) {
				error(0, "cannot load library (%s):%s\n",
				      lib_name, strerror(errno));
			}
			break;
		case 'c':
			list = 1;
			break;
		case 't':
			trace = 1;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}
	while (optind < argc) {
		if (pushf(argv[optind++])) {
			error(0, "cannot load file (%s):%s\n", argv[optind - 1], strerror(errno));
		}
	}
	pushf("-");
	while (do_bc() != EOF);
	program_exit();
   return exit_code;
}
