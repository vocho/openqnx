/* vicmd.h */
/* Copyright 1995 by Steve Kirkendall */


BEGIN_EXTERNC
extern RESULT v_ascii	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_at	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_delchar	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_ex	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_expose	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_input	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_notex	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_notop	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_number	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_paste	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_quit	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_setmark	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_tag	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_undo	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_visible	P_((WINDOW win, VIINFO *vinf));
extern RESULT v_window	P_((WINDOW win, VIINFO *vinf));
END_EXTERNC
