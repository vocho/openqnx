/* xevent.h */

extern ELVBOOL x_repeating;
extern ELVBOOL x_didcmd;

void x_ev_repeat P_((XEvent *event, long timeout));
XEvent *x_ev_wait P_((void));
void x_ev_process P_((XEvent *event));
ELVBOOL x_ev_poll P_((ELVBOOL reset));
