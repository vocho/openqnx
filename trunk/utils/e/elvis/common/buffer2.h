/* buffer2.h */
/* Copyright 1995 by Steve Kirkendall */

/* Ideally, these function would have been declared in buffer.h, but since
 * their arguments are MARKs, they had to be declared after the MARK data type
 * is declared in mark.h, but mark.h can only be included after buffer.h...
 * so these functions' declarations had to go into their own separate file.
 */
BEGIN_EXTERNC
extern ELVBOOL	bufread P_((MARK mark, char *rname));
extern ELVBOOL	bufwrite P_((MARK from, MARK to, char *wname, ELVBOOL force));
extern void	bufwilldo P_((MARK cursor, ELVBOOL will));
extern long	bufundo P_((MARK cursor, long back));
extern void	bufreplace P_((MARK from, MARK to, CHAR *newp, long newlen));
extern void	bufpaste P_((MARK dst, MARK from, MARK to));
extern CHAR	*bufmemory P_((MARK from, MARK to));
END_EXTERNC

/* This is used to store information needed to restore MARKs for an "undo" */
struct umark_s
{
	MARK	mark;	/* pointer to POSSIBLY FREED mark */
	long	offset;	/* offset of that mark, before a change */
};

