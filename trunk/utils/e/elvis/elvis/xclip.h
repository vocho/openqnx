/* xclip.h */

void	x_clipevent P_((XEvent *event));
ELVBOOL x_clipopen P_((ELVBOOL forwrite));
int	x_clipwrite P_((CHAR *text, int len));
int	x_clipread P_((CHAR *text, int len));
void	x_clipclose P_((void));
