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



#if !defined(__MINGW32__) && !defined(__CYGWIN__)
#include <lib/compat.h>

char *getenv(const char *);
char	*optarg;
int optind = 0;

int opterr = 1;
int optopt = 'z';

#ifndef TRUE
#define	TRUE (!0)
#endif

#ifndef FALSE
#define	FALSE (!1)
#endif

#define PSTRICT_ENVAR	"POSIX_STRICT"  /* Envar to indicate Strictly POSIX env */

#define PARGS_SEP	'*'	    /* argument seperator		    */
static int posix_opts =	-1;	    /* Strict Posix flag		    */

#define IS_OPTION( c ) ( c == '-' )

#define HUH		'?'
#define NO_MORE 	(-1)
#define HAS_ARG 	':'

/* just for now - fudge it up */
#define TXT(s)		    s
#define T_INVALID_OPTION    "Invalid option ('%c')\n"
#define T_EXTENDED_OPTIONS  "'%c' is an QNX extended option. This cannot be used when\nQNX is set up for strict POSIX conformance.\n"
#define T_MISSING_ARG	    "Missing argument for option '%c'\n"

int getopt(argc,argv,optstring)
int	argc;
char	*argv[];
char	*optstring;
{

    static	char	*next_char_to_process;
    static	int	last_optind;
    char		*cptr;
    int 		rc;
    char		*err_msg = (char *)0;

    /*
     *	Check if we are only parse POSIX defined options
     */

	if (optind == 0) {
		optind = 1;			/* set to point at first argument */
#if 0
		if(next_char_to_process)
			next_char_to_process=NULL;
		if(last_optind)
			last_optind=0;
#else
		last_optind = 2;
#endif
	}
	if (optind < last_optind) {
		last_optind = optind-1;	/* set to cause 'next_char_to_process to reset */
		posix_opts = getenv( PSTRICT_ENVAR ) != (char *)0;
	}

    if (optind>=argc) {
	/* ran out of arguments to process */
		return(NO_MORE);

    }

    if (last_optind != optind)
    {
		/* optind has been changed - reset next_char_to_process */
		next_char_to_process = argv[optind];
		last_optind = optind;

		/* if this argv doesn't start with -, we've reached end of options */
		if (!IS_OPTION(*next_char_to_process))
		{
		    return(NO_MORE);
		}
		next_char_to_process++;

		/* if this argv is just a -, we have also reached the end of options */
		if (!*next_char_to_process)
		{
		    /* don't increment optind - let the app see this as an option */
		    return(NO_MORE);
		}
    }

    /* check for user-specified end-of-options marker */
    if (IS_OPTION(*next_char_to_process))
    {
		/* hit end of options ('--' option) */
		optind++;
		return(NO_MORE);
    }

    /* check for match in optstring. Skip HAS_ARG characters */
    for (cptr=optstring;*cptr;++cptr)
    {
		if ( *cptr == HAS_ARG  )
		{
		    continue;
		}
		else if ((*cptr==PARGS_SEP) && (*(cptr+1)==PARGS_SEP ))
		{
		    /* we are into the extensions now */
		    if (posix_opts)
		    {
				err_msg = T_EXTENDED_OPTIONS;
				cptr="";    /* make cptr ptr to NULL */
				break;	    /* abort scan for match in optstring */
			} else {
				cptr++;
				continue;
		    }
		}

		if ( *next_char_to_process == *cptr )
		    break;			    /* found our match !	*/
	}

    /* increment optind now. If we didn't want to increment it (we
       aren't finished with the current one yet) we will decrement
       it later. There are more cases where it is incremented than
       where it remains the same. */

	optind++;

    /* Check that char just tested is a valid agument. Increment cptr now,
       since we will check a little bit further on for the HAS_ARG
       character indicating that we should expect an argument. Set
       rc to *cptr since in most cases this is what rc will end up
       being. Where we discover that we are in an error condition
       we will overwrite this value of rc. */

	if (rc = (int)*cptr++)
	{
		/* If there are more chars to come in current argv */
		if (*++next_char_to_process)
		{
	    /* if this option is supposed to take an argument, it
	       should be at the end of this argv.
	       Note that cptr has already been incremented for us. */
		    if (*cptr == HAS_ARG)
		    {
				/* this char must be the option argument */
				optarg = next_char_to_process;
		    } else {
				/* there are more chars to come in current argv -
			   this is the one case where optind remains the
			   same - decrement it since we had incremented it
			   at the top */
				optind--;
		    }
		} else {
		/* if there are no more chars to come in current argv */
		    if (*cptr == HAS_ARG)
		    {
				if (optind<argc)
				{
					/* this is cool - we were at end of current arg and
					last option requires an argument. We will fetch
					from next argv */
				    optarg = argv[optind];
					optind++;
				} else {
				    /* not cool. we ran out of argv arguments. Return
				       HUH because we are missing an argument */
				    optopt = rc;
				    rc = HUH;
				    err_msg = T_MISSING_ARG;
				}
			}
		}
	} else {
	/* HUH? - this character isn't a valid option */
		optind++;
		optopt = (int) *next_char_to_process;
		rc = HUH;
		if (!err_msg)
			err_msg = T_INVALID_OPTION;
		cptr = next_char_to_process +1;
	}

#if 0
	/* print error message to stderr */
	if ((opterr) && (rc==HUH))
	{
		fprintf(stderr,TXT(err_msg),*--cptr);
	}
#endif
	return(rc);
}

#endif
