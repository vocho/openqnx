/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <stdarg.h>
#include <string.h>
#include <kernel/nto.h>
#include "kdintl.h"


static void		(*outchar)(struct syspage_entry *, char);
static unsigned	paddr_size;

int 
kvsnprintf(char *buf, unsigned buflen, const char *fmt, va_list ap) {
	unsigned					len;

	for(buf--, fmt--, len = 0; len < buflen && (*++buf = *++fmt); len++) {
		if(*buf == '%') {
			register char				*str = 0;
			char						fill = ' ';
			char						leadchr = 0;
			int							i;
			int							width = 0;
			int							left = 0;
			int							ignore = 0;
			unsigned					strwidth = 0;
			unsigned					radix = 0;
			uint64_t					value = 0;
			char						temp[35];

			do {
				switch(*++fmt) {
				case '\0':
					fmt--;
				case '%':
					str = buf;
					break;

				case 'd':
					i = va_arg(ap, int);
					radix = 10;
					if(i < 0) {
						leadchr = '-';
						value = -i;
					} else {
						value = i;
					}
					break;
	
				case 'u':
					radix = 10;
					value = va_arg(ap, unsigned);
					break;

				case 'b':
					radix = 2;
					value = va_arg(ap, unsigned);
					break;

				case 'o':
					radix = 8;
					value = va_arg(ap, unsigned);
					break;
	
				case 'X':
				case 'x':
					radix = 16;
					value = va_arg(ap, unsigned);
					break;
	
				case 'p':
					radix = 16;
					value = (uintptr_t)va_arg(ap, void *);
					break;

				case 'P':	
					radix = 16;
					fill = '0';
					if(width == 0) width = paddr_size * 2;
					if(paddr_size == sizeof(paddr32_t)) {
						value = va_arg(ap, paddr32_t);
					} else {
						value = va_arg(ap, paddr64_t);
					}
					break;
					
				case 'c':
					str = temp;
					str[0] = va_arg(ap, int);
					str[1] = '\0';
					strwidth = 1;
					break;
	
				case 's':
					str = va_arg(ap, char *);
					if(!str) {
						str = "(NULL)";
					}
					break;

				case '-':
					left = 1;
					break;
	
				case '0':
					fill = *fmt;
					break;

				case '*':
					width = va_arg(ap, int);
					if(width < 0) {
						width = -width;
						left = 1;
					}
					ignore = 1;
					break;
	
				default:
					if(!ignore) {
						if(*fmt >= '0' && *fmt <= '9') {
							while(*fmt >= '0' && *fmt <= '9') {
								width = (width * 10) + *fmt++ - '0';
							}
							fmt--;
							ignore = 1;
						}
					}
					break;
				}
				if(radix) {
					unsigned char		shift = ((*fmt == 'X') ? 'A' : 'a') - '9' - 1;

					str = &temp[sizeof temp - 1];
					*str-- = '\0';
					do {
						*str = value % radix + '0';
						if(*str > '9') {
							*str += shift;
						}
						str--;
						value /= radix;
					} while(value);
					if(leadchr) {
						*str = leadchr;
					} else {
						str++;
					}
					strwidth = &temp[sizeof temp - 1] - str;
				}
			} while(!str);
			if(str != buf) {
				len--;
				buf--;
				if(width) {
					if(!strwidth) {
						strwidth = strlen(str);
					}
					if(width < strwidth) {
						width = strwidth;
					}
					if(!left) {
						while(++len < buflen && width > strwidth) {
							*++buf = fill;
							strwidth++;
						}
						len--;
					}
				}
				for(str--; ++len < buflen && (*++buf = *++str););
				if(*buf == '\0') {
					buf--;
				}
				len--;
				while(++len < buflen && width > strwidth) {
					*++buf = fill;
					len++;
					strwidth++;
				}
				len--;
			}
		}
	}
	return len;
}


void
out_text(const char *buffer, unsigned len) {
	unsigned	i;

	for(i = 0; i < len; ++i) {
		if(buffer[i] == '\n') outchar(_syspage_ptr, '\r');
		outchar(_syspage_ptr, buffer[i]);
	}
}


int
kprintf(const char *fmt, ... ) {
	char		buffer[100];
	va_list		ap;
	unsigned	len;

	va_start(ap, fmt);
	len = kvsnprintf(buffer, sizeof buffer, fmt, ap);
	out_text(buffer, len);
	va_end(ap);
	return len;
}


int
ksprintf(char *buffer, const char *fmt, ... ) {
	va_list				ap;
	int					len;

	va_start(ap, fmt);
	len = kvsnprintf(buffer, 1000, fmt, ap);
	va_end(ap);
	return(len);
}


void
kprintf_setup(void (*oc)(struct syspage_entry *, char), unsigned ps) {
	outchar = oc;
	paddr_size = ps;
}
