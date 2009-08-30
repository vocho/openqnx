/* fold.h */
/* Copyright 2000 by Steve Kirkendall */

#ifdef FEATURE_FOLD

typedef struct fold_s
{
	struct fold_s *next;
	MARK	from;	/* start of first line */
	MARK	to;	/* end of last line, inclusive */
	CHAR	*name;	/* displayed name of the fold */
} *FOLD;

/* These are used as the "flags" parameter of foldbyrange().  You can OR these
 * together in any combination.
 */
#define FOLD_NOEXTRA	0x00	/* just exact & overlapping folds */
#define FOLD_INSIDE	0x01	/* also folds wholly inside the range */
#define FOLD_OUTSIDE	0x02	/* also folds which wholly include the range */
#define FOLD_NESTED	0x04	/* also folds nested inside other folds */
#define FOLD_TOGGLE	0x10	/* action: unfold/refold the found FOLDs */
#define FOLD_DESTROY	0x20	/* action: delete the found FOLDs */
#define FOLD_TEST	0x40	/* action: do nothing, just detect the FOLDs */

extern FOLD foldalloc P_((MARK from, MARK to, CHAR *name));
extern void foldadd P_((FOLD fold, ELVBOOL infold));
extern RESULT foldbyname P_((BUFFER buf, CHAR *name, ELVBOOL infold));
extern RESULT foldbyrange P_((MARK from, MARK to, ELVBOOL infold, int flags));
extern FOLD foldmark P_((MARK mark, ELVBOOL infold));
extern void foldedit P_((MARK from, MARK to, MARK dest));

#endif /* defined(FEATURE_FOLD) */
