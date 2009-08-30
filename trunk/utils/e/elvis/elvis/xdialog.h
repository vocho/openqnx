/* xdialog.h */

typedef enum {
	FT_DEFAULT,	/* dummy value, for fields with no explicit type */
	FT_ONEOF,	/* one of a preset list; includes boolean */
	FT_NUMBER,	/* numeric field */
	FT_STRING,	/* string field */
	FT_FILE,	/* string field where <Tab> does filename completion */
	FT_LOCKED	/* non-editable field */
} X_FIELDTYPE;
typedef struct
{
	CHAR	*label;	/* displayed name of the option */
	char	*name;	/* actual name of the option */
	CHAR	*value;	/* option's current value */
	char	*limit;	/* legal values, for FT_ONEOF or FT_NUMBER */
	X_FIELDTYPE ft;	/* input type */
	unsigned lwidth;/* width of the label */
	unsigned twidth;/* width of the text field, if there is one */
} X_FIELD;
typedef struct button_s
{
	struct button_s *next;
	int	x, y;	/* position of the button */
	int	w, h;	/* size of the button */
	int	textx;	/* horizontal offset of text (lbearing) */
	int	texty;	/* vertical offset of text (ascent) */
	_char_	shape;	/* button shape -- usually 'b' for "button" */
	int	key;	/* keystroke to simulate if mouse released on button */
	char	*label;	/* label of the button */
	int	lablen;	/* length of label */
	int	state;	/* height of the button -- usually 2 */
} X_BUTTON;
typedef struct dialog_s
{
	struct dialog_s	*next;	/* another, unrelated dialog in list */
	X11WIN	*xw;	/* window where command should be run */
	char	*name;	/* name of the dialog, from toolbar button */
	char	*desc;	/* one-line description of what "submit" does */
	char	*excmd;	/* the command to execute if "submit" pressed */
	char	*spec;	/* descriptions of the fields */
	X_FIELD	*field;	/* details about each field */
	int	nfields;/* number of fields */
	X_BUTTON *button;/* list of buttons */

	ELVBOOL	pinned;	/* is this dialog pinned (persistent) ? */
	int	current;/* currently highlighted field -- -1 to start */
	int	cursor;	/* cursor position within the current field */
	int	shift;	/* shift amount of the current text field */
	X_BUTTON *click;/* button being clicked */

	unsigned w,h;	/* overall width & height of dialog */
	int	 x0,y0;	/* offsets to corner of first field's input */
	int	 rowh;	/* height of each option row */
	int	 cellw;	/* width of an input cell */
	int	 base;	/* where to draw the label positions (ascent) */
	X_BUTTON *submit;/* the submit button -- also in "button" list, above */
	X_BUTTON *cancel;/* the cancel button -- also in "button" list, above */

	Window	win;	/* X11 window of the dialog */
	GC	gc;	/* X11 graphic context */
} dialog_t;


extern void x_dl_add P_((X11WIN *xw, char *name, char *desc, char *excmd, char *spec));
extern void x_dl_delete P_((dialog_t *dia));
extern void x_dl_destroy P_((X11WIN *xw));
extern void x_dl_event P_((Window w, XEvent *event));
extern void x_dl_docolor P_((X11WIN *xw));
