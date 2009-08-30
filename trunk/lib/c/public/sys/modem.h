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
 *  sys/modem.h   Communications scripting functions
 *

 */
#ifndef __MODEM_H_INCLUDED
#define __MODEM_H_INCLUDED

#ifndef __TERMIOS_H_INCLUDED
 #include <termios.h>
#endif

#include <_pack64.h>

/*
 Example script
*/
/* curstate curflags newstate newflags newtimeout newquiet retvalue pattern response progress 
struct modem_script table[] ={
	{1, 0,            1, 0,               2,5, 0, NULL,            "ATZ\\r\\p0a", "Reseting modem"},
	{1, 0,            2, 0,              30,5, 0, "*ok*",          "ATDT5910934", "Dialing"},
	{2, MODEM_BAUD,   3, MODEM_LASTLINE, 15,5, 0, "*connect*",     NULL,          "Connected"},
	{3, 0,            4, 0,               8,5, 0, "*login:*",      "guest",       "Login"},
	{4, MODEM_NOECHO, 5, 0,              15,5, 0, "*password:*",   "xxxx",        "Password"},
	{5, 0,            0, 0,               0,0, 0, "*$ *",          NULL,          NULL},
	{0, 0,            0, 0,               0,0, 1, "*no carrier*",  NULL,          "No carrier"},
	{0, 0,            0, 0,               0,0, 2, "*no answer*",   NULL,          "No answer"},
	{0, 0,            0, 0,               0,0, 3, "*no dialtone*", NULL,          "No dialtone"},
	{0, 0,            0, 0,               0,0, 4, "*busy*",        NULL,          "Busy"},
	};

*/


struct modem_script {
	char	curstate;	/* This state */
	char	curflags;	/* Set to zero */
	char	newstate;	/* State to goto on a match (0 means done) */
	char	newflags;	/* Flags to use for nextstate */
	char	newtimeout;	/* Timeout to use for nextstate */
	char	newquiet;	/* Quiet time to use for nextstate */
	short	retvalue;	/* Return value on match with nextstate 0 */
	char	*pattern;	/* Input matching pattern */
	char	*response;	/* Output response to a pattern match in input */
	char	*progress;	/* A progress message passed to the io callback on state change */
	} ;

/* curflags definitions */
#define MODEM_BAUD		0x01	/* Extract baud */
#define MODEM_NOECHO	0x02	/* Don't pass responses to output function */

/* newflags definitions */
#define MODEM_ALLOWCASE	0x01	/* Allow mixed case in matches */
#define MODEM_ALLOWCTRL	0x02	/* Allow ctrl chars in matches */
#define MODEM_ALLOW8BIT	0x04	/* Allow 8 bit chars in matches */
#define MODEM_LASTLINE	0x08	/* Only return last line if possible */

/*
** Input patterns as defined by fnmatch() 
** Output responses support \ expansions as follows:
**    \r (CR), \n (NL), \xx (2 hex chars)
**    \B (500 msec break), \D (1 sec dropline), \Pxx (pause xx tenths of seconds)
*/

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

extern int modem_open(char *__device, speed_t __baud);
extern int modem_read(int __fd, char *__buf, int __bufsize, int __quiet, int __timeout, int __flags, int (*cancel)(void));
extern int modem_script(int __fd, struct modem_script *__table, speed_t *__baud,
							void (*io)(char *__progress, char *__in, char *__out),
							int (*cancel)(void));
extern int modem_write(int __fd, char *__str);

__END_DECLS

#include <_packpop.h>

#endif

/* __SRCVERSION("modem.h $Rev: 153052 $"); */
