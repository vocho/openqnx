/* options2.h */

#ifdef FEATURE_MKEXRC
BEGIN_EXTERNC
extern void optsave P_((BUFFER custom));
END_EXTERNC
#endif

#ifdef FEATURE_COMPLETE
BEGIN_EXTERNC
extern CHAR *optcomplete P_((WINDOW win, MARK m));
END_EXTERNC
#endif
