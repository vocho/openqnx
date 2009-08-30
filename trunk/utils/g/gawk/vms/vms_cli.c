/*
 * vms_cli.c - command line interface routines.
 *							Pat Rankin, Nov'89
 *	Routines called from vms_gawk.c for DCL parsing.
 */

#define  P(foo) ()
#include "config.h"	/* in case we want to suppress 'const' &c */
#include "vms.h"
#ifndef _STRING_H
#include <string.h>
#endif

extern U_Long CLI$PRESENT(const Dsc *);
extern U_Long CLI$GET_VALUE(const Dsc *, Dsc *, short *);
extern U_Long CLI$DCL_PARSE(const Dsc *, const void *, ...);
extern U_Long sys$cli(void *, ...);
extern U_Long sys$filescan(const Dsc *, void *, long *);
extern void  *lib$establish(U_Long (*handler)(void *, void *));
extern U_Long lib$sig_to_ret(void *, void *);	/* condition handler */

/* Cli_Present() - call CLI$PRESENT to determine whether a parameter or     */
/*		  qualifier is present on the [already parsed] command line */
U_Long
Cli_Present( const char *item )
{
    Dsc item_dsc;
    (void)lib$establish(lib$sig_to_ret);

    item_dsc.len = strlen(item_dsc.adr = (char *)item);
    return CLI$PRESENT(&item_dsc);
}

/* Cli_Get_Value() - call CLI$GET_VALUE to retreive the value of a */
/*		    parameter or qualifier from the command line   */
U_Long
Cli_Get_Value( const char *item, char *result, int size )
{
    Dsc item_dsc, res_dsc;
    U_Long sts;
    short len = 0;
    (void)lib$establish(lib$sig_to_ret);

    item_dsc.len = strlen(item_dsc.adr = (char *)item);
    res_dsc.len = size,  res_dsc.adr = result;
    sts = CLI$GET_VALUE(&item_dsc, &res_dsc, &len);
    result[len] = '\0';
    return sts;
}

/* Cli_Parse_Command() - use the $CLI system service (undocumented) to	 */
/*			retreive the actual command line (which might be */
/*			"run prog" or "mcr prog [params]") and then call */
/*			CLI$DCL_PARSE to parse it using specified tables */
U_Long
Cli_Parse_Command( const void *cmd_tables, const char *cmd_verb )
{
    struct { short len, code; void *adr; } fscn[2];
    struct { char rqtype, rqindx, rqflags, rqstat; unsigned :32;
	     Dsc rdesc; unsigned :32; unsigned :32; unsigned :32; } cmd;
    U_Long sts;
    int    ltmp;
    char   longbuf[2600];
    (void)lib$establish(lib$sig_to_ret);

    memset(&cmd, 0, sizeof cmd);
    cmd.rqtype = CLI$K_GETCMD;		/* command line minus the verb */
    sts = sys$cli(&cmd, (void *)0, (void *)0);	/* get actual command line */

    if (vmswork(sts)) {		/* ok => cli available & verb wasn't "RUN" */
	/* invoked via symbol => have command line (which might be empty) */
	/*    [might also be invoked via mcr or dcl; that's ok]		  */
	if (cmd.rqstat == CLI$K_VERB_MCR) {
	    /* need to strip image name from MCR invocation   */
	    memset(fscn, 0, sizeof fscn);
	    fscn[0].code = FSCN$_FILESPEC;	/* full file specification */
	    (void)sys$filescan(&cmd.rdesc, fscn, (long *)0);
	    cmd.rdesc.len -= fscn[0].len;	/* shrink size */
	    cmd.rdesc.adr += fscn[0].len;	/* advance ptr */
	}
	/* prepend verb and then parse the command line */
	strcat(strcpy(longbuf, cmd_verb), " "),  ltmp = strlen(longbuf);
	if (cmd.rdesc.len + ltmp > sizeof longbuf)
	    cmd.rdesc.len = sizeof longbuf - ltmp;
	strncpy(&longbuf[ltmp], cmd.rdesc.adr, cmd.rdesc.len);
	cmd.rdesc.len += ltmp,	cmd.rdesc.adr = longbuf;
	sts = CLI$DCL_PARSE( &cmd.rdesc, cmd_tables);
    }

    return sts;
}
