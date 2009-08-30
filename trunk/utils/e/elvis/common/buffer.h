/* buffer.h */
/* Copyright 1995 by Steve Kirkendall */


struct undo_s
{
	struct undo_s	*next;		/* pointer to the next-older version of this buffer */
	long		changes;	/* change counter, used for restarting */
	long		changepos;	/* where to move cursor if undone */
	long		buflines;	/* number of lines in this version of the buffer */
	long		bufchars;	/* number of characters in this version */
	struct umark_s	*marklist;	/* list of marks that were in this buffer */
	BLKNO		bufinfo;	/* bufinfo BLK of previous version */
#ifdef DEBUG_ALLOC
	struct undo_s	*link1, *link2;	/* some other allocated undo versions */
	struct buffer_s	*buf;		/* the buffer that this undoes */
	char		undoredo;	/* 'u' for undo, 'r' for redo version */
#endif
};


typedef struct buffer_s
{
	struct buffer_s	*next;
	struct mark_s	*marks;		/* linked list of marks pointing to this buffer */
	struct undo_s	*undo;		/* linked list of undo versions of this buffer */
	struct undo_s	*redo;		/* linked list of undo versions of this buffer */
	struct undo_s	*undolnptr;	/* element of undo list which is line-undo version */
	BLKNO		bufinfo;	/* blkno of the bufinfo block for this buffer */
	long		changes;	/* change counter, used for restarting */
	long		changepos;	/* position of the most recent change to this buffer */
	long		undoline;	/* line number of "undolnptr" version */
	long		docursor;	/* cursor position when "willdo" was set */
	long		ntagdefs;	/* number of items in tagdef array */
	struct tedef_s	*tagdef;	/* tag definitions in this buf */
#ifdef FEATURE_AUTOCMD
	ELVBOOL		eachedit;	/* "Edit" event for each change (else only when willdo is set) */
#endif
	ELVBOOL		willdo;		/* save an "undo" version before next bufreplace()? */
	long		willevent;	/* value of eventcount when willdo was set */
#ifdef FEATURE_FOLD
	struct fold_s	*fold;		/* list of active FOLDs */
	struct fold_s	*unfold;	/* list of inactive FOLDs */
#endif
#ifdef FEATURE_REGION
	struct region_s	*regions;	/* list of regions in this buffer */
#endif
	OPTVAL		filename;	/* string: name of the file for this buffer */
	OPTVAL		bufname;	/* string: name of buffer */
	OPTVAL		bufid;		/* number: unique number for user buffer, or 0 */
	OPTVAL		buflines;	/* number: of lines, from txtbuf->lines */
	OPTVAL		bufchars;	/* number: of bytes, from txtbuf->bytes */
	OPTVAL		retain;		/* boolean: keep buffer after writing? */
	OPTVAL		modified;	/* boolean: buffer modified since last write? */
	OPTVAL		edited;		/* boolean: bufname changed since last write? */
	OPTVAL		newfile;	/* boolean: buffer for non-existent file? */
	OPTVAL		readonly;	/* boolean: no write perms on file? */
	OPTVAL		autoindent;	/* boolean: auto-indent? */
	OPTVAL		inputtab;	/* one of Tab/Spaces/Filename: <Tab> key in input mode */
	OPTVAL		autotab;	/* boolean: use tabs characters shifting? */
	OPTVAL		tabstop;	/* tab: width of a tab */
	OPTVAL		cc;		/* string: program run by :cc command */
	OPTVAL		equalprg;	/* string: program run by = command */
	OPTVAL		keywordprg;	/* string: program run by shift-K command */
	OPTVAL		make;		/* string: program run by :make command */
	OPTVAL		paragraphs;	/* string: list of nroff paragraph codes */
	OPTVAL		sections;	/* string: list of nroff section codes */
	OPTVAL		shiftwidth;	/* tab: shiftwidth used by << and >> */
	OPTVAL		undolevels;	/* number: number of undo versions to maintain */
	OPTVAL		textwidth;	/* number: word wrap position (replaced wrapmargin) */
	OPTVAL		internal;	/* boolean: is this a special-purpose buffer? */
	OPTVAL		bufdisplay;	/* string: the default display mode */
	OPTVAL		initialsyntax;	/* start in "syntax" mode regardless of bufdisplay? */
	OPTVAL		errlines;	/* number: #lines when errlist created */
	OPTVAL		readeol;	/* one of unix/dos/mac/text/binary: file read mode */
	OPTVAL		locked;		/* boolean: prevent changes to buffer? */
	OPTVAL		partiallastline;/* boolean: file has no real last newline */
	OPTVAL		putstyle;	/* one of {character line rectangle} */
	OPTVAL		timestamp;	/* timestamp of file, as a string */
	OPTVAL		guidewidth;	/* widths of columns where guides are drawn */
	OPTVAL		hlobject;	/* list of text objects to possibly highlight */
	OPTVAL		spell;		/* should tags be considered words for spellchecking? */
	OPTVAL		lisp;		/* lisp mode */
	OPTVAL		mapmode;	/* which set of key maps to use */
	OPTVAL		smartargs;	/* display arguments when typing func */
	OPTVAL		userprotocol;	/* user-defined protocol */
	OPTVAL		bb;		/* generic option */
} *BUFFER;

#define o_filename(buf)		((buf)->filename.value.string)
#define o_bufname(buf)		((buf)->bufname.value.string)
#define o_bufid(buf)		((buf)->bufid.value.number)
#define o_buflines(buf)		((buf)->buflines.value.number)
#define o_bufchars(buf)		((buf)->bufchars.value.number)
#define o_retain(buf)		((buf)->retain.value.boolean)
#define o_preservable(buf)	((buf)->preservable.value.boolean)
#define o_modified(buf)		((buf)->modified.value.boolean)
#define o_edited(buf)		((buf)->edited.value.boolean)
#define o_newfile(buf)		((buf)->newfile.value.boolean)
#define o_readonly(buf)		((buf)->readonly.value.boolean)
#define o_autoindent(buf)	((buf)->autoindent.value.boolean)
#define o_inputtab(buf)		((buf)->inputtab.value.character)
#define o_autotab(buf)		((buf)->autotab.value.boolean)
#define o_tabstop(buf)		((buf)->tabstop.value.tab)
#define o_cc(buf)		((buf)->cc.value.string)
#define o_equalprg(buf)		((buf)->equalprg.value.string)
#define o_keywordprg(buf)	((buf)->keywordprg.value.string)
#define o_make(buf)		((buf)->make.value.string)
#define o_paragraphs(buf)	((buf)->paragraphs.value.string)
#define o_sections(buf)		((buf)->sections.value.string)
#define o_shiftwidth(buf)	((buf)->shiftwidth.value.tab)
#define o_undolevels(buf)	((buf)->undolevels.value.number)
#define o_textwidth(buf)	((buf)->textwidth.value.number)
#define o_internal(buf)		((buf)->internal.value.boolean)
#define o_bufdisplay(buf)	((buf)->bufdisplay.value.string)
#define o_initialsyntax(buf)	((buf)->initialsyntax.value.boolean)
#define o_errlines(buf)		((buf)->errlines.value.number)
#define o_readeol(buf)		((buf)->readeol.value.character)
#define o_locked(buf)		((buf)->locked.value.boolean)
#define o_partiallastline(buf)	((buf)->partiallastline.value.boolean)
#define o_putstyle(buf)		((buf)->putstyle.value.character)
#define o_timestamp(buf)	((buf)->timestamp.value.string)
#define o_guidewidth(buf)	((buf)->guidewidth.value.tab)
#define o_hlobject(buf)		((buf)->hlobject.value.string)
#define o_spell(buf)		((buf)->spell.value.boolean)
#define o_lisp(buf)		((buf)->lisp.value.boolean)
#define o_mapmode(buf)		((buf)->mapmode.value.string)
#define o_smartargs(buf)	((buf)->smartargs.value.boolean)
#define o_userprotocol(buf)	((buf)->userprotocol.value.boolean)
#define o_bb(buf)		((buf)->userprotocol.value.string)
#define BUFOPTQTY		((sizeof(struct buffer_s) - (int)bufoptvals((BUFFER)0)) / sizeof(OPTVAL))

#define bufbufinfo(buffer)	((buffer)->bufinfo)
#define bufoptvals(buffer)	(&(buffer)->filename)
#define bufmarks(buffer)	((buffer)->marks)
#define bufsetmarks(buffer, mark) ((buffer)->marks = (mark))
#define buflist(start)		((start) ? (start)->next : elvis_buffers)

extern BUFFER bufdefault;
extern BUFFER elvis_buffers;
extern BUFFER bufdefopts;
extern MSGIMP bufmsgtype;
BEGIN_EXTERNC
extern void bufinit P_((void));
extern BUFFER bufalloc P_((CHAR *name, _BLKNO_ bufinfo, ELVBOOL internal));
extern CHAR *buffilenumber P_((CHAR **refp));
extern BUFFER buffind P_((CHAR *name));
extern BUFFER bufload P_((CHAR *bufname, char *filename, ELVBOOL reload));
extern BUFFER bufpath P_((CHAR *path, char *filename, CHAR *bufname));
extern ELVBOOL bufunload P_((BUFFER buf, ELVBOOL force, ELVBOOL save));
extern ELVBOOL bufsave P_((BUFFER buf, ELVBOOL force, ELVBOOL mustwr));
extern void bufoptions P_((BUFFER buffer));
extern void buffree P_((BUFFER buffer));
extern void buftitle P_((BUFFER buffer, CHAR *title));
END_EXTERNC
