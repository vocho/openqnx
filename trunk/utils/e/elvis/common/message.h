/* message.h */
/* Copyright 1995 by Steve Kirkendall */

/* This data type is used to denote the importance and class of a message. */
typedef enum
{
	MSG_STATUS,	/* e.g., "Reading file..." */
	MSG_INFO,	/* e.g., "Read file, 20 lines, 187 characters" */
	MSG_WARNING,	/* e.g., "More files to edit" */
	MSG_ERROR,	/* e.g., "Unknown command" */
	MSG_FATAL	/* e.g., "Error writing to session file" */
} MSGIMP;


BEGIN_EXTERNC
extern void msglog P_((char *filename));
extern void msg P_((MSGIMP imp, char *terse, ...));
extern void msgflush P_((void));
extern CHAR *msgtranslate P_((char *word));
extern ELVBOOL msghide P_((ELVBOOL hide));
END_EXTERNC
