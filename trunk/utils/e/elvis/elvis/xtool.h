/* xtool.h */

#define MAXTOOLS	50

typedef struct
{
	Window		win;		/* toolbar subwindow */
	int		x, y;		/* position of toolbar within window */
	unsigned int	w, h;		/* overall size of the toolbar */
	ELVBOOL		recolored;	/* have colors been changed lately? */
	struct
	{
		int	x, y;		/* position of a button within toolbar*/
		int	bevel;		/* bevel height of a button */
	}		state[MAXTOOLS];/* info about each button */
} X_TOOLBAR;

CHAR *x_tb_dump P_((char *label));
void x_tb_predict P_((X11WIN *xw, unsigned w, unsigned h));
void x_tb_create P_((X11WIN *xw, int x, int y));
void x_tb_destroy P_((X11WIN *xw));
void x_tb_draw P_((X11WIN *xw, ELVBOOL fromscratch));
void x_tb_event P_((X11WIN *xw, XEvent *event));
ELVBOOL x_tb_config P_((ELVBOOL gap, char *label, _char_ op, char *value));
void x_tb_recolor P_((X11WIN *xw));
