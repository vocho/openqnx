/* tagsrch.h */

BEGIN_EXTERNC

void tsreset P_((void));
void tsparse P_((char *text));
void tsadjust P_((TAG *tag, _char_ oper));
void tsfile P_((char *filename, long maxlength));

END_EXTERNC
