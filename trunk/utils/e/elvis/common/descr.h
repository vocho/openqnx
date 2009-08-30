/* descr.h */

/* Copyright 2000 by Steve Kirkendall.  Redistributable under the terms of
 * the Perl "Clarified Artistic" license.
 */

#ifdef FEATURE_CACHEDESC
void *descr_recall P_((WINDOW win, char *dfile));
#endif
ELVBOOL descr_open P_((WINDOW win, char *dfile));
CHAR **descr_line P_((void));
void descr_close P_((void *descr));
CHAR *descr_known P_((char *filename, char *dfile));
#ifdef DISPLAY_SYNTAX
ELVBOOL descr_cancall P_((CHAR *filename));
#endif
