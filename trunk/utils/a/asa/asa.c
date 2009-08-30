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



#ifdef __USAGE		/* asa.c */
%C - filter to translate line printer control sequences to newlines/form feeds (POSIX)

%C	[filename...]
#endif

/*
 *-----------------------------------------------------------------------------
 * Filename:	asa.c
 *
 * Author:		Jeffrey G. George
 *
 * Change		 By			  Date			Reason for Change
 * 0			JGG			21-Feb-90		First Release
 *-----------------------------------------------------------------------------
 * System :	Posix Utility Set
 *
 * Usage:	asa [filename....]
 *
 * Description:
 *			The asa utility writes its input to standard output, mapping 
 *			carriage control characters from the test files to line printer
 *			control sequences in an implementation-defined manner.
 *
 *			All characters in olumn position 1 are removed from the input,
 *			and the following actions are performed:
 *
 *			<space>		The rest of the line in output without change
 *			0			A <newline> is output, then the rest of the input line
 *			1			One or more iomplementation-defined characters that
 *						causes an advance to the next page is output, followed
 *						by teh rest of the input line.
 *			+			The <newline> of the previous line is replaced with one
 *						or more implementation defined characters that causes
 *						printing to return to the column position 1, followed
 *						by the rest of the input line.
 *
 * Note:	See POSIX 1003.2 Draft 9 Section C.1
 *			For ANSI compilation see sections annotated with FOR_ANSI
 *
 * Args:
 *			filename...					Name of input file(s)
 *
 * Include declarations:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Define declarations:
 */
#define BUFFER_SIZE 2048
#define FORM_FEED 0x0C
#define CARRIAGE_RETURN 0x0D

/*
 * Global Declarations
 */
char	buffer[ BUFFER_SIZE ];

void process_file( FILE * );

/*
 *- ---------------------------------------------------------------------------
 */

int main( int argc, char *argv[] )
{
	int i;
	FILE *cur_file;

	for( i = 1; i < argc; i++ ) {
		if ( strcmp( argv[ i ], "-") )
			cur_file = fopen( argv[i], "r" );
		else
			cur_file = stdin;
		if (!cur_file)
			fprintf(stderr,"asa: unable to open %s\n", argv[i]);	
		else{
			process_file( cur_file );
			if (cur_file != stdin)
				fclose( cur_file );
			}
		}
	return EXIT_SUCCESS;
	}



void process_file( fptr )
FILE *fptr;
{
	int i, last_char;
	last_char = 0;
	while( !feof( fptr) ){
		if (fgets( buffer, BUFFER_SIZE, fptr)){
			i = strlen( buffer)-1;
			switch( buffer[0] ){
				case '0':	*buffer='\n';	break;
				case '1':	*buffer=FORM_FEED;	break;
				case '+':	if (last_char=='\n')
								last_char = 0;
							*buffer=CARRIAGE_RETURN;
							break;
				default: 	break;
				}
			if (last_char)
				fputc( last_char, stdout );
			last_char = buffer[ i ];
			buffer[i]=0;
			fputs( buffer, stdout);
			}
		}
	}
