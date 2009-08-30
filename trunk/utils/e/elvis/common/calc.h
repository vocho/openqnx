/* calc.h */
/* Copyright 1995 by Steve Kirkendall */

/* This is used for storing information about subscripts */
typedef struct
{
	CHAR	*ptr;	/* start of a chunk of text */
	int	len;	/* length of the chunk */
} CHUNK;

typedef enum {CALC_DOLLAR=1, CALC_PAREN=2, CALC_MSG=3, CALC_OUTER=4, CALC_ALL=7} CALCRULE;

BEGIN_EXTERNC
extern ELVBOOL calcnumber P_((CHAR *value));
extern ELVBOOL calctrue P_((CHAR *value));
extern CHAR *calculate P_((CHAR *expr, CHAR **arg, CALCRULE rule));
#ifdef FEATURE_CALC
# ifdef FEATURE_ARRAY
extern _CHAR_ calcsubscript P_((CHAR *array, CHAR *sub, int max, CHUNK *chunks));
# endif
extern ELVBOOL calcbase10 P_((CHAR *value));
extern ELVBOOL calcsel P_((MARK from, MARK to));
extern CHAR *calcelement P_((CHAR *set, CHAR *element));
extern CHAR *calcset P_((CHAR *left, _CHAR_ oper, CHAR *right));
#endif
END_EXTERNC
