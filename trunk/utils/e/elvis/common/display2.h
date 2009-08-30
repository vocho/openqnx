/* display2.h */
/* Copyright 1995 by Steve Kirkendall */

/* Define DISPLAY_ANYMARKUP if any markup modes are used */
#ifndef DISPLAY_ANYMARKUP
# if defined(DISPLAY_HTML) || defined(DISPLAY_MAN) || defined(DISPLAY_TEX)
#  define DISPLAY_ANYMARKUP
# endif
#endif

/* Define DISPLAY_ANYDESCR if any markup or syntax modes are used */
#ifndef DISPLAY_ANYDESCR
# if defined(DISPLAY_SYNTAX) || defined(DISPLAY_ANYMARKUP)
#  define DISPLAY_ANYDESCR
# endif
#endif

struct dispmode_s
{
	char	*name;
	char	*desc;
	ELVBOOL	canopt;
	ELVBOOL	wordwrap;
	int	nwinopts;
	OPTDESC	*winoptd;
	int	nglobopts;
	OPTDESC	*globoptd;
	OPTVAL	*globoptv;
	DMINFO	*(*init) P_((WINDOW win));
	void	(*term) P_((DMINFO *info));
	long	(*mark2col) P_((WINDOW w, MARK mark, ELVBOOL cmd));
	MARK	(*move) P_((WINDOW w, MARK from, long linedelta, long column, ELVBOOL cmd));
	MARK	(*wordmove) P_((MARK from, long count, ELVBOOL backward, ELVBOOL whitespace));
	MARK	(*setup) P_((WINDOW w, MARK top, long cursor, MARK bottom, DMINFO *info));
	MARK	(*image) P_((WINDOW w, MARK line, DMINFO *info,
			void (*draw)(CHAR *p, long qty, _char_ font, long offset)));
	void	(*header) P_((WINDOW w, int pagenum, DMINFO *info,
			void (*draw)(CHAR *p, long qty, _char_ font, long offset)));
	void	(*indent) P_((WINDOW w, MARK line, long linedelta));
	CHAR	*(*tagatcursor) P_((WINDOW win, MARK cursor));
	MARK	(*tagload) P_((CHAR *tagname, MARK from));
	MARK	(*tagnext) P_((MARK cursor));
};

extern DISPMODE	dmnormal;
#ifdef DISPLAY_HEX
extern DISPMODE	dmhex;
#endif
#ifdef DISPLAY_HTML
extern DISPMODE	dmhtml;
#endif
#ifdef DISPLAY_MAN
extern DISPMODE	dmman;
#endif
#ifdef DISPLAY_TEX
extern DISPMODE	dmtex;
#endif
#ifdef DISPLAY_SYNTAX
extern DISPMODE dmsyntax;
#endif
extern DISPMODE	*allmodes[];

BEGIN_EXTERNC
extern void	displist P_((WINDOW win));
extern ELVBOOL	dispset P_((WINDOW win, char *newmode));
extern void	dispinit P_((ELVBOOL before));
extern void	dispoptions P_((DISPMODE *mode, DMINFO *info));
extern MARK	dispmove P_((WINDOW win, long linedelta, long wantcol));
extern long	dispmark2col P_((WINDOW win));
extern void	dispindent P_((WINDOW w, MARK line, long linedelta));

#ifdef DISPLAY_ANYMARKUP
extern void	dmmuadjust P_((MARK from, MARK to, long delta));
#endif
#ifdef DISPLAY_SYNTAX
extern CHAR	dmspreprocessor P_((WINDOW win));
extern ELVBOOL	dmskeyword P_((WINDOW win, CHAR *word));
# ifdef FEATURE_SMARTARGS
extern void	dmssmartargs P_((WINDOW win));
# endif
#endif
extern int	dmnlistchars P_((_CHAR_ ch, long offset, long col, short *tabstop, void(*draw)(CHAR *p, long qty, _char_ font, long offset)));


END_EXTERNC
