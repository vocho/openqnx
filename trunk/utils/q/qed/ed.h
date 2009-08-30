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





/*
 * This file contains the information necessary for a subtask
 * created by the full screen editor to communicate with the
 * full screen editor via a simple message interface.
 *
 * To force the editor to drop into communication with your task
 * it is necessary to force it ready and inform it of your task
 * id. This may be accomplished by seting a user exception of
 * your task id on it.
 *
 *		set_exception(Ed_tid, 0, My_tid)
 *
 * This will unblock the editor which will then do a specific receive
 * on My_tid. Upon your death the editor will be unblocked by the system
 * and continue.
 */

/*
 * The four msg_code values to be sent to the editor.
 * The editor will reply with a return status in the msg_code field.
 * A value of zero indicates success. A value of -3 lines up with
 * the "OUT OF MEMORY" error message in appendix A of the editor
 * manual. "Unknown Command" would be a -4 and so on.
 * The command in EXEC_COMMAND is a character string made up of
 * low level editor commands (No macros are expanded).
 * For example to insert a line
 *		"i text of line to insert"
 * and to change line 27.
 *		"27c text of line to change"
 * and to write the buffer out to file "next_text"
 *		"w new_text"
 * or to simply change its name
 *		"f new_name"
 *
 * Finially, you may even force the editor to redisplay its screen
 * by sending it a DISPLAY_SCREEN message.
 */

#define EXEC_COMMAND	0	/* Execute command in msg_data			*/
#define READ_CURLINE	1	/* Return current line in msg_data		*/
#define READ_ABSLINE	2	/* Return line specified in msg_line	*/
#define READ_DATA		3	/* Return data in msg_data with the     */
							/* structure indicated in ed_data		*/
#define DISPLAY_SCREEN	4	/* Force the editor to display it screen*/


struct message {
	char msg_code;
	unsigned msg_line;		/* Only used by READ_ABSLINE			*/
	char msg_data[256];
	} ;



/*
 * This overlays msg_data (above) when a READ_DATA command is send.
 */

struct ed_data {
	int nladdrs;			/* Number of line address before '!'	*/
	int laddr1;				/* First line address					*/
	int laddr2;				/* Second line address					*/
							/* ALL commands should try and operate	*/
							/* only within laddr1 and laddr2 if they*/
							/* are specified						*/
	
	int curln;				/* Current line number					*/
	int curin;
	int curcol;
	int cmdin;
	int lastln;				/* Last line number						*/
	int center_line;		/* Center line (ie: vc<number>)			*/
	int nmarks;				/* Number of tagged lines				*/
	int marker1;			/* First tagged line					*/
	int marker2;			/* Second tagged line					*/
	int left_margin;		/* Margin column numbers				*/
	int right_margin;
	
	int screen_row;			/* Line number of top of screen			*/
	int screen_col;			/* Column number of left edge of screen	*/
	char cursor_row;		/* Relative (0 -> 22) and				*/
	char cursor_col;		/* (0 -> 79) co-ordinates of cursor		*/
	char cmd_cursor_col;
	
	char cc_reg;			/* Condition code register				*/
	dev_type;				/* Device type (monochrome or colour	*/
	
	char opt_a;				/* Editor options						*/
	char opt_b;
	char state;
	char opt_d;
	char opt_f;
	char opt_i;
	char opt_j;
	char opt_l;
	char opt_m;
	char opt_n;
	char opt_s;
	char opt_t;
	char opt_w;
	
	char curfile[FNAME_SIZE + 1];	/* Current filename				*/
	} ;							
