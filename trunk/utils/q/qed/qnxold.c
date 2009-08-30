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





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/qnxterm.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

#if defined(__QNX__) && !defined(__QNXNTO__)
#undef SIGTSTP		/* QNX4 doesn't support this correctly */
#endif

int		Break_count;

void
breakable() {
	sigset_t	set;

	if ( Break_count ) {
		Break_count--;
		sigemptyset( &set );
		sigaddset( &set, SIGINT );
#ifdef SIGTSTP
		sigaddset( &set, SIGTSTP );
#endif
		sigprocmask( SIG_UNBLOCK, &set, (void *) 0 );
		}
	}

void
unbreakable() {
	sigset_t	set;

	if ( ! Break_count++ ) {
		sigemptyset( &set );
		sigaddset( &set, SIGINT );
#ifdef SIGTSTP
		sigaddset( &set, SIGTSTP );
#endif
		sigprocmask( SIG_BLOCK, &set, (void *) 0 );
		}
	}

int fput( char *buf, int n, FILE *fp ) {
	return( fwrite( buf, 1, n, fp ) );
	}

int fgetline( FILE *fp, char *buf, int maxlen ) {
	char *p = buf;
	int c, len = 0;

	while( len <= maxlen-1  &&  (c = getc( fp )) != EOF ) {
		*p++ = (char) c;
		++len;
		if ( c == CR_  ||  c == LF_  ||  c == RS_ ) break;
		}
	*p++ = '\0';
	return( len );
/*	return( fgets( buf, maxlen, fp ) ? strlen( buf ) : 0 );	*/
	}

void
forward( FILE *fp ) {
	fseek( fp, 0L, 2 );
	}

unsigned	char	old_qnxkeys[]	=	{
	/*	0400	*/	0xff,
	/*	0401	*/	0xff,
	/*	0402	*/	0xa9,
	/*	0403	*/	0xa1,
	/*	0404	*/	0xa4,
	/*	0405	*/	0xa6,
	/*	0406	*/	0xa0,
	/*	0407	*/	0xff,
	/*	0410	*/	0xff,
	/*	0411	*/	0x81,
	/*	0412	*/	0x82,
	/*	0413	*/	0x83,
	/*	0414	*/	0x84,
	/*	0415	*/	0x85,
	/*	0416	*/	0x86,
	/*	0417	*/	0x87,
	/*	0420	*/	0x88,
	/*	0421	*/	0x89,
	/*	0422	*/	0x8a,
	/*	0423	*/	0xae,
	/*	0424	*/	0xaf,
	/*	0425	*/	0x8b,
	/*	0426	*/	0x8c,
	/*	0427	*/	0x8d,
	/*	0430	*/	0x8e,
	/*	0431	*/	0x8f,
	/*	0432	*/	0x90,
	/*	0433	*/	0x91,
	/*	0434	*/	0x92,
	/*	0435	*/	0x93,
	/*	0436	*/	0x94,
	/*	0437	*/	0xdb,
	/*	0440	*/	0xdc,
	/*	0441	*/	0x95,
	/*	0442	*/	0x96,
	/*	0443	*/	0x97,
	/*	0444	*/	0x98,
	/*	0445	*/	0x99,
	/*	0446	*/	0x9a,
	/*	0447	*/	0x9b,
	/*	0450	*/	0x9c,
	/*	0451	*/	0x9d,
	/*	0452	*/	0x9e,
	/*	0453	*/	0xbe,
	/*	0454	*/	0xbf,
	/*	0455	*/	0xd1,
	/*	0456	*/	0xd2,
	/*	0457	*/	0xd3,
	/*	0460	*/	0xd4,
	/*	0461	*/	0xd5,
	/*	0462	*/	0xd6,
	/*	0463	*/	0xd7,
	/*	0464	*/	0xd8,
	/*	0465	*/	0xd9,
	/*	0466	*/	0xda,
	/*	0467	*/	0xce,
	/*	0470	*/	0xcf,
	/*	0471	*/	0xff,
	/*	0472	*/	0xff,
	/*	0473	*/	0xff,
	/*	0474	*/	0xff,
	/*	0475	*/	0xff,
	/*	0476	*/	0xff,
	/*	0477	*/	0xff,
	/*	0500	*/	0xff,
	/*	0501	*/	0xff,
	/*	0502	*/	0xff,
	/*	0503	*/	0xff,
	/*	0504	*/	0xff,
	/*	0505	*/	0xff,
	/*	0506	*/	0xff,
	/*	0507	*/	0xff,
	/*	0510	*/	0xbc,
	/*	0511	*/	0xbb,
	/*	0512	*/	0xac,
	/*	0513	*/	0xab,
	/*	0514	*/	0xcb,
	/*	0515	*/	0xe1,
	/*	0516	*/	0xcc,
	/*	0517	*/	0xc8,
	/*	0520	*/	0xb1,
	/*	0521	*/	0xb9,
	/*	0522	*/	0xaa,
	/*	0523	*/	0xa2,
	/*	0524	*/	0xe2,
	/*	0525	*/	0x9f,
	/*	0526	*/	0xe4,
	/*	0527	*/	0xd0,
	/*	0530	*/	0xff,
	/*	0531	*/	0xff,
	/*	0532	*/	0xad,
	/*	0533	*/	0xff,
	/*	0534	*/	0xff,
	/*	0535	*/	0xff,
	/*	0536	*/	0xff,
	/*	0537	*/	0xff,
	/*	0540	*/	0xff,
	/*	0541	*/	0x80,
	/*	0542	*/	0xc0,
	/*	0543	*/	0xa3,
	/*	0544	*/	0xe3,
	/*	0545	*/	0xa5,
	/*	0546	*/	0xb5,
	/*	0547	*/	0xc5,
	/*	0550	*/	0xa8,
	/*	0551	*/	0xb8,
	/*	0552	*/	0xe6,
	/*	0553	*/	0xe8,
	/*	0554	*/	0xed,
	/*	0555	*/	0xe5,
	/*	0556	*/	0xe9,
	/*	0557	*/	0xca,
	/*	0560	*/	0xef,
	/*	0561	*/	0xeb,
	/*	0562	*/	0xc2,
	/*	0563	*/	0xde,
	/*	0564	*/	0xec,
	/*	0565	*/	0xe7,
	/*	0566	*/	0xf2,
	/*	0567	*/	0xea,
	/*	0570	*/	0xf0,
	/*	0571	*/	0xf1,
	/*	0572	*/	0xee,
	/*	0573	*/	0xb3,
	/*	0574	*/	0xb7,
	/*	0575	*/	0xf3,
	/*	0576	*/	0xf4,
	/*	0577	*/	0xff,
	/*	0600	*/	0xf6,
	/*	0601	*/	0xa7,
	/*	0602	*/	0xc1,
	/*	0603	*/	0xc9,
	/*	0604	*/	0xf7,
	/*	0605	*/	0xf8,
	/*	0606	*/	0xf9,
	/*	0607	*/	0xb0,
	/*	0610	*/	0xe0,
	/*	0611	*/	0xb4,
	/*	0612	*/	0xc4,
	/*	0613	*/	0xc6,
	/*	0614	*/	0xba,
	/*	0615	*/	0xfa,
	/*	0616	*/	0xb2,
	/*	0617	*/	0xbd,
	/*	0620	*/	0xcd,
	/*	0621	*/	0xff,
	/*	0622	*/	0xb6,
	/*	0623	*/	0xff,
	/*	0624	*/	0xc7,
	/*	0625	*/	0xc3,
	/*	0626	*/	0xff,
	/*	0627	*/	0xff,
	/*	0630	*/	0xf5,
	/*	0631	*/	0xfe
	};

unsigned
translate_key( unsigned ch ) {
	return( old_qnxkeys[ ch -= 0400 ] );
	}

#if 0
static int rcount;

void
report( char *msg, int arg1 )
	{
	term_printf( 10, 0, 0, "%3d: ", rcount++ );
	term_printf( -1, -1, 0, msg, arg1 );
	term_clear( 1 );
	term_flush();
	}

static int dcount;

void
dump() {
	struct line *p1;
	int y = 10, i = 0;

	p1 = firstp;
	term_printf( y++, 0, 0, "%d", dcount++ );
	term_printf( y++, 0, 0, "rbuff: |%s|", rbuff );
	while( p1 ) {
		term_printf( y++, 0, 0, "%04x %04x %04x |%s|",
			p1, p1->next, p1->prev, p1->textp );
		term_clear( 1 );
		if ( p1 == lastp ) break;
		p1 = p1->next;
		if ( ++i > 10 ) break;
		}
	term_clear(2);
	term_flush();
	}
#endif
