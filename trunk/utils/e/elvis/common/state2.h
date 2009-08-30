/* state2.h */
/* Copyright 1995 by Steve Kirkendall */



typedef enum
{
	CURSOR_NONE,	/* no visible cursor (may be handy inside gui module) */
	CURSOR_INSERT,	/* next character will be inserted */
	CURSOR_REPLACE,	/* next character will replace a existing text */
	CURSOR_COMMAND,	/* next character will be part of a visual command */
	CURSOR_QUOTE	/* next character should be quoted; will replace '^' */
} ELVCURSOR;

struct state_s
{
   struct state_s *pop;				/* next state on stack */
   struct state_s *acton;			/* state that enter() acts on */
   RESULT	(*enter) P_((WINDOW win));	/* perform line processing */
   RESULT	(*perform) P_((WINDOW win));	/* execute command */
   RESULT	(*parse) P_((_CHAR_ key, void *info)); /* parse a keystroke */
   ELVCURSOR	(*shape) P_((WINDOW win));	/* decide on cursor shape */
   MARK		cursor;				/* the cursor & buffer to use */
   MARK		top, bottom;			/* extent being edited */
   void		*info;				/* extra info, for parser */
   ELVISSTATE	flags;				/* flags of current state */
   char		*modename;			/* mode name, for "showmode" */
   CHAR		morekey;			/* special key for [More] */
   CHAR		prompt;				/* prompt char, or '\0' */
   MAPFLAGS	mapflags;			/* keystroke mapping state */
   long		wantcol;			/* stratum's desired column */
};

#define ELVIS_USERMAP	0x0001	/* in the middle of a user's map */
#define ELVIS_KEYMAP	0x0002	/* in the middle of a function key map */
#define ELVIS_BOTTOM	0x0010	/* open mode; use bottom row only */
#define ELVIS_REGION	0x0020	/* editing a region; clean up if cursor moves */
#define ELVIS_POP	0x0100	/* pop after completing current keystroke */
#define ELVIS_ONCE	0x0200	/* pop after completing next keystroke */
#define ELVIS_1LINE	0x0400	/* pop after completing current line */
#define ELVIS_ALERT	0x0800	/* pop to this state if alerted */
#define ELVIS_FREE	0x1000	/* free(info) when freeing state */
#define ELVIS_MORE	0x2000	/* call perform() after popping */

BEGIN_EXTERNC
extern void statepush P_((WINDOW win, ELVISSTATE flags));
extern void statestratum P_((WINDOW win, CHAR *bufname, _CHAR_ prompt, RESULT (*enter)(WINDOW win)));
extern void statepop P_((WINDOW win));
extern RESULT statekey P_((_CHAR_ key));
END_EXTERNC

/* This macro returns the buffer that keystrokes act on. */
#define statecmdbuf(win)	(markbuffer((win)->state->cursor))

/* This macro returns the cursor that ex commands and search commands should
 * act on.  This is generally the stratum under the keystroke's stratum.
 */
#define statedatacursor(win)	((win)->state->acton->cursor)

/* These macros return the top & bottom of the range affected by a command. */
#define statedatatop(win)	((win)->state->acton->top)
#define statedatabottom(win)	((win)->state->acton->bottom)

/* This macro returns the buffer that ex commands and search commands should
 * act on.  This is generally the stratum under the keystroke's stratum.
 */
#define statedatabuf(win)	(markbuffer(statedatacursor(win)))
