/*
 * qnx_term_load() finds the size of the device on stderr.
 * It also sets up the graphics chars for qnx_term_fputs().
 *
 * qnx_term_fputs() outputs all chars between 0x20 and 0x7F.
 * It will map line drawing chars to the current console.
 * All characters it can't map will be displayed as "?"
 */
#ifndef _STDIO_H_INCLUDED
	#include <stdio.h>
#endif

#define _QNX_T_LINES	0x0001		/* load the line_drawing chars */

#ifdef __cplusplus
extern "C" {
#endif
int qnx_term_load(int flags, int *cols, int *rows);
int qnx_term_fputs(char *buf, FILE *fp);

#ifdef __cplusplus
};
#endif
