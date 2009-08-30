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
**   I N C L U D E S
*/
#include	"mig.h"

#define	VERSION			"1.04"

#ifdef __USAGE
%C -[mqrt] -o dir file [...] [>report]
Options:
 -m             Point out where the migration library can be used where
                possible (default is to tell how to convert to the QNX
                Neutrino way only and ignore the use of the migration
                library).
 -o dir         Directory to output marked up source.
 -q             Keep quite about progress (no msgs to stderr).
 -r             Produce report only (no writing of marked up source).
 -t table_file  File containing the migration table (defaults to
                /etc/mig4nto.tab).
         
 file [...] File(s) to analyze.
 report     File to put report into.
#endif

/*
**   G L O B A L   V A R I A B L E S
*/
int						 Debug;						/* Debug flag */
int						 Quiet;						/* Quiet operation */
int						 Strict = TRUE;				/* Strict checking */
int						 ReportOnly;				/* Produce report only */
char					*(*Files)[];				/* File names to process */
int						 FileCount;					/* Number of files processed */
int						 FileCurrent;				/* Current file to be processed */
char					 MarkedUpName[ MAX_DIR ];	/* Output directory/file name */
char					*FNode = MarkedUpName;		/* Pointer to node area of dir */
char					 LineBuffer[ LINE_BUFF_SIZE ];
unsigned				 LineCount;					/* Current line number in file */
AVL_ROOT_STRUCT			*FunctionTree;				/* Tree containing the mig funcs */
AVL_ENTRY_STRUCT		*(*Funcs)[];				/* Functions on this line */
int						 FuncCount;					/* Count of funcs this line */
char					*MigTabName = "/etc/mig4nto.tab";
FILE					*MigTabfp;					/* Mig table file pointer */
int						 ExitStatus;				/* main() exit status */

char		 			*OutofMemory = "MIG4NTO:  Out of memory\n";


/*
**   L O C A L   V A R I A B L E S
*/
static int		  FileDepth;				/* Current alloc'd depth */
static int		  FuncDepth;				/* Current alloc'd depth */


/***********************************************************/
static int		Initialize( void )
{
	/*
	** Load the Migration Function Tree
	*/
	if ( LoadFunctionTree() != 0 )			/* mig4nto.tab load */
		return -1;

	/*
	** Setup file for lexical analysis and marked up output
	*/
	if ( yywrap() != 0 )
		return -1;


	return 0;
}


/***********************************************************/
void			Process( void )
{
	AVL_ENTRY_STRUCT		*FuncHit;

	while ( yylex() != 0 )
		if ( (FuncHit = LookupAvl( yytext, FunctionTree )) )
		{	if ( ++FuncCount > FuncDepth )
			{	/*
				** Get a few more function pointers
				*/
				FuncDepth += MIN_BLOCK_FUNCPTRS;

				if ( (Funcs = realloc( Funcs, FuncDepth * sizeof (AVL_ENTRY_STRUCT *) )) == NULL )
				{	fprintf( stderr, OutofMemory );
					exit( -1 );
				}
			}
			(*Funcs)[ FuncCount - 1 ] = FuncHit;
		}
}

/***********************************************************/
void			CleanUp( void )
{
	FreeAvlTree( FunctionTree, TRUE );		/* Free the AVL (including data) */
}


/***********************************************************/
int				main( int argc, char **argv )
{
	int		 c;
	int		 AbnormalExit	= FALSE;
	FILE	*fp;

    while ((c = getopt(argc, argv, "Dqmrt:o:")) != -1)
    {
        switch (c)
        {
        case 'D': Debug			= TRUE; break;
        case 'm': Strict		= FALSE; break;
        case 'q': Quiet			= TRUE; break;
        case 'r': ReportOnly	= TRUE; break;
        case 't': MigTabName    = optarg; break;
        case 'o':
            strcpy( MarkedUpName, optarg );
            FNode = &MarkedUpName[ strlen( MarkedUpName ) ];
            if ( *(FNode - 1) != '/' )
                *FNode++ = '/';
            break;
        case '?':
            /* getopt() already reported this error */
            exit( EXIT_FAILURE );
        }
    }
    
    for (; argv[optind]; optind++)
    {
    	if ( ++FileCount > FileDepth )
        {	/*
            ** Get a few more char pointers for filenames
            */
            FileDepth += MIN_BLOCK_FILENAME;

            if ( (Files = realloc( Files, FileDepth * sizeof (char *) )) == NULL )
            {	fprintf( stderr, OutofMemory );
                exit( -1 );
            }
        }

        (*Files)[ FileCount - 1 ] = argv[ optind ];
    }
    
	if ( ! ReportOnly ) {
		if ( ! *MarkedUpName )
		{	fprintf( stderr, "MIG4NTO:  No output directory\n" );
			AbnormalExit = TRUE;
		}
		else if ( (fp = fopen( MarkedUpName, "r" )) == NULL )
		{	fprintf( stderr, "MIG4NTO:  Can't access directory \"%s\"\n", MarkedUpName );
			AbnormalExit = TRUE;
		}
		else
			fclose( fp );
	}

	if ( ! FileCount )
	{	fprintf( stderr, "MIG4NTO:  No files specified\n" );
		AbnormalExit = TRUE;
	}

	if ( access( MigTabName, F_OK ) != 0 )
	{	fprintf( stderr, "MIG4NTO:  \"%s\" file does not exist\n", MigTabName );
		AbnormalExit = TRUE;
	}

	if ( ! AbnormalExit )
		if ( (MigTabfp = fopen( MigTabName, "r" )) == NULL )
		{	fprintf( stderr, "MIG4NTO:  The table is currently being modified.  Please try later.\n" );
			AbnormalExit = TRUE;
		}

	if ( ! AbnormalExit )
	{	if ( ! Quiet )
			fprintf( stderr, "MIG4NTO  -  Migration Aid Utility (%s)\n", __DATE__ );
		if ( Initialize() == 0 )
		{	Process();
			CleanUp();
		}
		else
			AbnormalExit = TRUE;
	}

	if ( Files )
		free( Files );

	if ( Funcs )
		free( Funcs );

	if ( MigTabfp )
		fclose( MigTabfp );


	return AbnormalExit ? 2 : ExitStatus;
}


/* End of File */
