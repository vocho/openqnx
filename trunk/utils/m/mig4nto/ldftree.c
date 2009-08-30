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

static int		InsertIntoTree( long FPos )
{
	char				*FName;
	char				*FDesc;
	AVL_ENTRY_STRUCT	*Node;


	if ( (FName = strtok( &LineBuffer[2], " \t\n\x0a\x1e" )) )
		if ( (FDesc = strtok( NULL, "\n\x0a\x1e" )) )
		{	if ( InsertAvl( FName, NULL, FunctionTree, &Node ) )
			{	Node->key  = strdup( FName );
				Node->data = calloc( 1, sizeof( MIGFUNC_STRUCT ) );
				if ( Node->key && Node->data )
				{	((MIGFUNC_STRUCT *)(Node->data))->DescPos = FPos + (FDesc - LineBuffer);
					((MIGFUNC_STRUCT *)(Node->data))->DescLen = strlen( FDesc );
				}
				else
				{	fprintf( stderr, OutofMemory );
					return -1;
				}
			}
		}


	return 0;
}


int				LoadFunctionTree()
{
	char	*p;
	int		 c;
	int		 i;
	long	 FPos;


	if ( (FunctionTree = AllocAvlTree( (AVL_COMP_FN) strcmp )) == NULL )
	{	fprintf( stderr, OutofMemory );
		return -1;
	}

	if ( ! Quiet )
		fprintf( stderr, "Loading \"%s\"...", MigTabName );

	while( 1 )
	{	if ( (FPos = ftell( MigTabfp )) == -1 )
		{	fprintf( stderr, "MIG4NTO:  Unable to read \"%s\"\n", MigTabName );
			return -1;
		}

		/*
		** Get character by character to allow different line separators
		*/
		i = 0;
		p = LineBuffer;

		do
		{	if ( (c = fgetc( MigTabfp )) == EOF )
				break;

			if ( i++ <= LINE_BUFF_SIZE )
				*p++ = c;
		}
		while ( c != '\n' && c != LF );

		if ( c == EOF )
			break;

		*p = '\0';

		/*
		** Ignore Comments
		*/
		if ( LineBuffer[ 0 ] == '"' )
			continue;

		/*
		** Ignore Blank Lines
		*/
		c = LineBuffer[ strspn( LineBuffer, " \t" ) ];
		if ( c == '\n' || c == LF )
			continue;

		if ( Strict && LineBuffer[0] != 'M' )
        {	/* strict or not a migration function */
			if ( InsertIntoTree( FPos ) != 0 )
				return -1;
		}
		else if ( !Strict && LineBuffer[0] == 'M' )
        {   /* not strict and is a migration function */
			if ( InsertIntoTree( FPos ) != 0 )
				return -1;
        }
	}

	if ( ! Quiet )
	{	fprintf( stderr, "Complete." );
		if ( Debug )
			fprintf( stderr, " (%u entries)\n", FunctionTree->entries );
		else
			fprintf( stderr, "\n" );
	}


	return 0;
}


/* End of File */
