/* autocmd.h */

#ifdef FEATURE_AUTOCMD

/* The following symbolic constants may be passed to auperform() to trigger
 * auto-commands.  The values here correspond to indicies into the nametbl[]
 * array in autocmd.c -- if you change one, then you should change the other
 * to keep them in sync.
 */
typedef enum {
	AU_ALL_EVENTS = 0,
	AU_NONOPTION_EVENTS,
	AU_BUFCREATE,
	AU_BUFDELETE,
	AU_BUFENTER,
	AU_BUFFILEPOST,
	AU_BUFFILEPRE,
	AU_BUFHIDDEN,
	AU_BUFLEAVE,
	AU_BUFNEWFILE,
	AU_BUFREAD,		/* same as AU_BUFREADPOST */
	AU_BUFREADPOST,
	AU_BUFREADPRE,
	AU_BUFUNLOAD,
	AU_BUFWRITE,		/* same as AU_BUFWRITEPRE */
	AU_BUFWRITEPOST,
	AU_BUFWRITEPRE,
	AU_FILEAPPENDPOST,
	AU_FILEAPPENDPRE,
	AU_FILECHANGEDSHELL,
	AU_FILEREADPOST,
	AU_FILEREADPRE,
	AU_FILEWRITEPOST,
	AU_FILEWRITEPRE,
	AU_FILTERREADPOST,
	AU_FILTERREADPRE,
	AU_FILTERWRITEPOST,
	AU_FILTERWRITEPRE,
	AU_STDINREADPOST,
	AU_STDINREADPRE,
	AU_ALIASENTER,
	AU_ALIASLEAVE,
	AU_BGCHANGED,
	AU_CURSORHOLD,
	AU_DISPLAYENTER,
	AU_DISPLAYLEAVE,
	AU_DISPMAPENTER,
	AU_DISPMAPLEAVE,
	AU_EDIT,
	AU_FILEENCODING,
	AU_FILETYPE,
	AU_FOCUSGAINED,
	AU_FOCUSLOST,
	AU_GUIENTER,
	AU_OPTCHANGED,
	AU_OPTSET,
	AU_SCRIPTENTER,
	AU_SCRIPTLEAVE,
	AU_SYNTAX,
	AU_TERMCHANGED,
	AU_USER,
	AU_VIMENTER,
	AU_VIMLEAVE,
	AU_VIMLEAVEPRE,
	AU_WINENTER,
	AU_WINLEAVE,
	AU_USER01,
	AU_USER02,
	AU_USER03,
	AU_USER04,
	AU_USER05,
	AU_USER06,
	AU_USER07,
	AU_USER08,
	AU_USER09,
	AU_USER10,
	AU_USER11,
	AU_USER12,
	AU_USER13,
	AU_USER14,
	AU_USER15,
	AU_USER16,
	AU_USER17,
	AU_USER18,
	AU_USER19,
	AU_USER20,
	AU_USER21,
	AU_USER22,
	AU_USER23,
	AU_USER24,
	AU_USER25,
	AU_USER26,
	AU_USER27,
	AU_USER28,
	AU_USER29,
	AU_USER30,
	AU_QTY_EVENTS,		/* number of events in nametbl[] */
	AU_NO_EVENT		/* not a valid event code */
} auevent_t;

extern MARK	autop, aubottom;
extern ELVBOOL	aubusy;

extern RESULT ex_auevent P_((EXINFO *xinf));
extern RESULT ex_augroup P_((EXINFO *xinf));
extern RESULT ex_autocmd P_((EXINFO *xinf));
extern RESULT ex_doautocmd P_((EXINFO *xinf));
extern RESULT auperform P_((WINDOW win, ELVBOOL bang, CHAR *groupname, auevent_t event, CHAR *filename));
extern void audispmap P_((void));
extern CHAR *auname P_((CHAR *name));

# ifdef FEATURE_MKEXRC
extern void ausave P_((BUFFER custom));
# endif

# ifdef FEATURE_COMPLETE
extern CHAR *aucomplete P_((WINDOW win, MARK from, MARK to));
# endif


#endif /* not FEATURE_AUTOCMD */
