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





#include	"mig.h"


static FILE		*FileIn;
static char		 EoLChar = '\n';

/*
**	This routine is called by the lexical analyzer each time an
**	EOF condition occurs.  If there are more files to analyze,
**	then yyin and yyout are set to point to the new files
**	and the return value is 0.  When all is done 1 is returned.
*/
int			yywrap()					/* END OF FILE */
{
	char	*FilePortionName;
	int		 i;
	int		 c;
	char	*p;


	if ( ! Quiet  &&  yyin != stdin )
		fprintf( stderr, "\t%u lines processed\n", --LineCount );

	LineCount = 1;

	if ( yyin != stdin )		/* Don't go through here 1st time */
		fclose( yyin );

	if ( yyout != stdout )		/* Close file being marked up */
		fclose( yyout );

	if ( FileIn )				/* Close orignal source file */
		fclose( FileIn );

	while ( FileCount-- )
	{	char	fullpathin[  FULLPATH_MAX + 1 ],
				fullpathout[ FULLPATH_MAX + 1 ];

		while ( FileCount >= 0 )
		{	
#if defined(__QNXNTO__)
            if ( realpath( (*Files)[ FileCount ], fullpathin ) == NULL )
			{	fprintf( stdout, "\nMIG4NTO:  Unable to obtain full path of %s.  Skipped\n", (*Files)[FileCount--] );
				continue;
			}
#else
            if ( qnx_fullpath( fullpathin, (*Files)[ FileCount ] ) == NULL )
			{	fprintf( stdout, "\nMIG4NTO:  Unable to obtain full path of %s.  Skipped\n", (*Files)[FileCount--] );
				continue;
			}
#endif

			if ( (yyin = fopen( (*Files)[ FileCount ], "r" )) == NULL )
			{	fprintf( stdout, "\nMIG4NTO:  Unable to Open %s.  Skipped\n", (*Files)[FileCount--] );
				continue;
			}
			else
				break;
		}

		if ( FileCount < 0 )
			return 1;

		/*
		** Strip path
		*/
		if ( ! ReportOnly )
		{	if ( (FilePortionName = strrchr( (*Files)[ FileCount ], '/' )) )
				FilePortionName++;
			else
				FilePortionName = (*Files)[ FileCount ];

			strcpy( FNode, FilePortionName );

#if defined(__QNXNTO__)
			if ( realpath( MarkedUpName, fullpathout ) != NULL )
#else
			if ( qnx_fullpath( fullpathout, MarkedUpName ) != NULL )
#endif
				if ( strcmp( fullpathin, fullpathout ) == 0 )
				{	fprintf( stdout, "\nMIG4NTO:  Input/Output File are the same %s.  Skipped\n", fullpathout );
					continue;
				}

			if ( (yyout = fopen( MarkedUpName, "w" )) == NULL )
			{	fprintf( stderr, "MIG4NTO:  Unable to Open %s\n", MarkedUpName );
				exit( -1 );
			}
		}

		/*
		** Update Status
		*/
		if ( ! Quiet )
			fprintf( stderr, "%s\n", (*Files)[ FileCount ] );

		/*
		** So we can copy to the output (the file exists)
		*/
		if ( ! ReportOnly )
		{	FileIn = fopen( (*Files)[ FileCount ], "r" );

			/*
			** Get character by character to allow either
			** QNX 2.xx or 4.x line separators
			*/
			i = 0;
			p = LineBuffer;

			do
			{	if ( (c = fgetc( FileIn )) == EOF )
					break;

				if ( i++ < LINE_BUFF_SIZE )
					*p++ = c;

			} while ( c != '\n' && c != LF );

			EoLChar = c;

			*p = '\0';
		}

		return 0;
	}

	return 1;
}


/*
**	This routine is called by the lexical analyzer each time an
**	EOL condition occurs.  It updates the marked up file and
**	continues.
*/
void		yyeol()						/* END OF LINE */
{
	int		 i;
	int		 c;
	char	*p;


	/*
	** Update the report
	*/
	for ( i = 0; i < FuncCount; i++ )
		fprintf( stdout, "%s (%u): %s\n",	(*Files)[ FileCount ],
											LineCount,
											(char *) (*Funcs)[ i ]->key );
	/*
	** Print the line preceeded by our markups
	*/
	if ( ! ReportOnly )
	{	PrintMarkups( EoLChar );

		/*
		** Get character by character to allow either
		** QNX 2.xx or 4.x line separators
		*/
		i = 0;
		p = LineBuffer;

		do
		{	if ( (c = fgetc( FileIn )) == EOF )
				break;

			if ( i++ < LINE_BUFF_SIZE )
				*p++ = c;

		} while ( c != '\n' && c != LF );

		*p = '\0';
	}

	FuncCount = 0;

	if ( Debug )
		fprintf( stderr, "%u\r", LineCount );
}


/* End of File */
