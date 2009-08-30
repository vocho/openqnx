/* operator.h */
/* Copyright 1995 by Steve Kirkendall */



BEGIN_EXTERNC
extern RESULT oper P_((WINDOW win, VIINFO *vinf, MARK from, MARK to));
extern RESULT opfilter P_((MARK from, MARK to, CHAR *prog));
END_EXTERNC
