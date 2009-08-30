/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */




#ifndef _CONSOLE_H_INCLUDED
#define _CONSOLE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
console_numcons(struct _console_ctrl *cc);
console_getfont(struct _console_ctrl *cc, int font, struct _console_font *buf);
console_setfont(struct _console_ctrl *cc, int font, struct _console_font *buf);
console_refreshfont(struct _console_ctrl *cc, int font);
console_larger(struct _console_ctrl *cc, short rows, short cols, short *nrows, short *ncols);
console_smaller(struct _console_ctrl *cc, short rows, short cols, short *nrows, short *ncols);
console_prefix_scan(char *node);
console_prefix_index(char *cons);
char *console_prefix_name(int index);

#ifdef __cplusplus
};
#endif
#endif
