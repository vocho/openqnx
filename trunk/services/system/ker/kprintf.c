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

#include <stdarg.h>
#include "externs.h"

void
strscrn_display(const char *text) {
	scrn_display(text, strlen(text));
}

void
scrn_display(const char *text, int len) {
	void (*outchar)(struct syspage_entry *, char) = calloutptr->debug[0].display_char;
	unsigned status = InterruptStatus();
	InterruptDisable();

	SPINLOCK(&debug_slock);
	while(len != 0) {
		if(*text == '\n') outchar(_syspage_ptr, '\r');
		outchar(_syspage_ptr, *text);
		++text;
		--len;
	}
	SPINUNLOCK(&debug_slock);
	if (( status )) { 
		InterruptEnable();
	}
}

int
scrn_poll_key(void) {
	int		ch;
	unsigned status = InterruptStatus();
	InterruptDisable();

	SPINLOCK(&debug_slock);
	ch = calloutptr->debug[0].poll_key(_syspage_ptr);
	SPINUNLOCK(&debug_slock);
	if (( status )) {
		InterruptEnable();
	}
	return(ch);
}


void
dbug_display(const char *text, int len) {
	const char *cp;

	if(len == 0) len = strlen(text);

	// Watcom says we must break into multiple calls at \n boundries.
	for(cp = text ; len ; ++cp, --len) {
		if(*cp == '\n') {
			DebugKDOutput(text, (cp - text) + 1);
			text = cp + 1;
		}
	}
	if(cp - text) {
		DebugKDOutput(text, cp - text);
	}
}


int kvsnprintf(char *buf, size_t buflen, const char *fmt, va_list ap) {
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
			paddr_t						value;
			char						temp[35];

			do {
				value = 0;
				switch(*++fmt) {
				case '\0':
					fmt--;
					/* fall through */
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
	
				case 'P':
					radix = 16;
					value = va_arg(ap, paddr_t);
//					fill = '0';
//					width = sizeof(paddr_t)*2;
					break;

				case 'p':
					width = sizeof(unsigned) * 2;	/* two nibbles per byte */
					fill = '0';
					/* Fall Through */
				case 'X':
				case 'x':
					radix = 16;
					value = va_arg(ap, unsigned);
					break;
					
				case 'c':
					str = temp;
					str[0] = (char)va_arg(ap, int);
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
					unsigned shift = (((*fmt == 'X') ? 'A' : 'a') - '9') - 1;
					unsigned c;

					str = &temp[sizeof temp - 1];
					*str-- = '\0';
					do {
						c = value % radix + '0';
						if(c > '9') c += shift;
						*str-- = (char)c;
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
				for(str--; ++len < buflen && (*++buf = *++str);) {
					/* nothing to do */
				}
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


int
kdprintf(const char *fmt, ... ) {
	char				buffer[100];
	va_list				ap;
	unsigned			len;

	va_start(ap, fmt);
	len = kvsnprintf(buffer, sizeof buffer, fmt, ap);
	dbug_display(buffer, len);
	va_end(ap);
	return(len);
}


int
kprintf(const char *fmt, ... ) {
	char				buffer[100];
	va_list				ap;
	unsigned			len;

	va_start(ap, fmt);
	len = kvsnprintf(buffer, sizeof buffer, fmt, ap);
	scrn_display(buffer, len);
	va_end(ap);
	return(len);
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

int ksnprintf(char *buffer, size_t len, const char *fmt, ...) {
	va_list				ap;

	va_start(ap, fmt);
	len = kvsnprintf(buffer, len, fmt, ap);
	va_end(ap);
	return(len);
}

__SRCVERSION("kprintf.c $Rev: 174894 $");
