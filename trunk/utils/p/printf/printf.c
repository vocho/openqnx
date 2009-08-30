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
#ifdef __USAGE
%C - write formatted output (POSIX)

%C	format [argument...]
Where:
 format    is a printf() style format specification.
 argument  is a list of strings to be formatted by position.
#endif
*/

/*-
P1003.2/9,4.49:	printf -- Write formatted output. 

Synopsis:
	printf format [arguments...]

Description:

	printf writes its arguments to standard output according to the
	format specified as the first parameter.   The arguments are 
	character strings, are converted into the internal representation,
	specified positionally in the format string, and then output using
	the format string to convert to appropriate types.  The purpose for
	the conversion is to avoid duplicating the printf() library routine,
	it is much simpler, and less error prone, to let it do the work.

	The format-type specifications are:
		d,i,o,u,x,X,f,eE,g,G,c,s

	The length modifier (ell) is not referenced in the specification.			
	It is permitted.		
    	
		
Bugs:
	The '%c' format, when confronted with a valid decimal value, should
	print the ascii character conforming to the modulo of that value
	from the number of ascii characters....

*/
	
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include <util/stdutil.h>
#include <lib/compat.h>

/*	

	format spec : 
				| char
				| format_item
				| format_spec char
				| format_spec item
				;
	format_itmer: 	%[+- #]*0?([0-9]*|\*)(.([0-9]*|\*)?)?[doxucsefgb]

*/

char		*built_fmt;

void
error_exit(char *err) {
	fflush(stdout);
	fprintf(stderr, "\n%s\n", err);
	exit(EXIT_FAILURE);
}

char *
handle_escape(char *esc) {
	char	ch;

	switch(*esc) {
	case '\\': 	ch = '\\'; break;
	case 'a': 	ch = '\a'; break;
	case 'b': 	ch = '\b'; break;
	case 'f': 	ch = '\f'; break;
	case 'n': 	ch = '\n'; break;
	case 'r': 	ch = '\r'; break;
	case 't': 	ch = '\t'; break;
	case 'v': 	ch = '\v'; break;
	case '0':	
		ch = strtoul(esc+1, &esc, 8);
		--esc;
		break;
	case 'x':	
		ch = strtoul(esc+1, &esc, 16);
		--esc;
		break;
	case '\0':
		error_exit("invalid escape sequence");
	default:
		ch = strtoul(esc, &esc, 8);
		--esc;
		break;
	}
	putc(ch, stdout);
	return(esc + 1);
}

void
escape_string(char *src, char *dst) {
	char	ch;
	char	*start = dst;

	for( ;; ) {
		ch = *src++;
		if(ch == '\\') {
			switch(*src++) {
			case '\\': 	ch = '\\'; break;
			case 'a': 	ch = '\a'; break;
			case 'b': 	ch = '\b'; break;
			case 'f': 	ch = '\f'; break;
			case 'n': 	ch = '\n'; break;
			case 't': 	ch = '\t'; break;
			case 'v': 	ch = '\v'; break;
			case '0':
				ch = strtoul(src, &src, 8);
				break;
			case 'x':
				ch = strtoul(src, &src, 16);
				break;
			case 'c':
				*dst = '\0';
				printf(built_fmt, start);
				exit(EXIT_SUCCESS);
			default:
				error_exit("invalid escape sequence");
			}
		}
		*dst++ = ch;
		if(ch == '\0')break;
	}
}

char *
handle_arg(char *fmt, char *arg) {
	char		*dst;
	char		*new;
	size_t		len;
	unsigned	longs;
	union {
		unsigned long	_int;
		double			_double;
	}			data;

	dst = built_fmt;
	*dst++ = '%';
	len = strspn(&fmt[0], "-+#"); 			// collect flags
	len += strspn(&fmt[len], "0123456789");	// collect field width
	if(fmt[len] == '.') {					// collect precision
		++len;
		len += strspn(&fmt[len], "0123456789");	
	}
	memcpy(dst, fmt, len);
	dst += len;
	fmt += len;
	for(longs = 0; *fmt == 'l'; ++longs) {
		*dst++ = *fmt++;
	}
	*dst++ = *fmt;
	*dst = '\0';
	switch(*fmt++) {
	case 'd':
	case 'i':
	case 'o':
	case 'u':
	case 'x':
	case 'X':
	case 'c':
		//print out an integer format
		data._int = strtoul(arg, NULL, 0);
		switch(longs) {
		case 0:
			printf(built_fmt, (unsigned)data._int);
			break;
		case 1:
			printf(built_fmt, (unsigned long)data._int);
			break;
		case 2:
			//Maybe support later
		default:
			error_exit("invalid format string");
		}
		break;
		
	case 'f': 
	case 'e':
	case 'E':
	case 'g':
	case 'G':
		//print out a floating point format
		data._double = strtod(arg, NULL);
		printf(built_fmt, data._double);
		break;

	case 'b':
		dst[-1] = 's';
		new = alloca(strlen(arg)+1);
		if(new == NULL) {
			error_exit("no memory for temporary string");
		}
		escape_string(arg, new);
		arg = new;
		// Fall through
	case 's': 
		//print out a string format
		printf(built_fmt, arg);
		break;
	default:
		error_exit("invalid format string");
	}
	return(fmt);
}


int
main(int argc, char **argv) {
	char		*fmt;
	unsigned	arg_idx;

	if(argc < 2) {
		error_exit("usage: printf format strings...");
	}

	built_fmt = malloc(strlen(argv[1])+1);
	if(built_fmt == NULL) {
		error_exit("insufficient memory for temporary format string");
	}
	arg_idx = 2;
	for(fmt = argv[1]; *fmt != '\0'; ++fmt) {
		if(*fmt == '\\') {
			fmt = handle_escape(&fmt[1]) - 1;
		} else if(*fmt == '%') {
			if(fmt[1] == '%') {
				putc(*fmt, stdout);
				++fmt;
			} else {
				if(arg_idx >= argc) {
					error_exit("insufficient arguments for format string");
				}
				fmt = handle_arg(&fmt[1], argv[arg_idx++]) - 1;
			}
		} else {
			putc(*fmt, stdout);
		}
	}
	return(EXIT_SUCCESS);
}
