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


static char		Buff[ LINE_BUFF_SIZE ];

void	PrintMarkups( char EoLChar )
{
	int		 i;
	char	*bol,
			*eol,
			*eob;
	int		 DescLen;
	char	*fmt = "/* MIG4NTO %16.16s";


	if ( FuncCount )
	{	/*
		** Annotate the markup file
		*/
		ExitStatus = 1;

		for ( i = 0; i < FuncCount; i++ )
		{	fprintf( yyout, fmt, (*Funcs)[ i ]->key );

			fseek( MigTabfp, ((MIGFUNC_STRUCT *)((*Funcs)[ i ]->data))->DescPos, SEEK_SET );
			DescLen = ((MIGFUNC_STRUCT *)((*Funcs)[ i ]->data))->DescLen;
			fread( Buff, sizeof (char), DescLen, MigTabfp );

			bol = Buff;
			eol = Buff;
			eob = Buff + DescLen;
			
			while ( eol < eob )
			{	if ( bol != Buff )
					fprintf( yyout, fmt, "" );
				eol = bol + RIGHT_MARGIN;	/* Maximum length of description */
				if ( eol < eob )
					while ( *eol != ' ' )
						eol--;
				else
					eol = eob;

				fprintf( yyout,  " %.*s */%c", eol-bol, bol, EoLChar );

				bol = eol+1;
			}
		}
	}

	/*
	** Print the line of code
	*/
	fprintf( yyout, "%s", LineBuffer );
}


/* End of File */
