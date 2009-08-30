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
 * Do NOT add any variables except at the end of the file.  The message
 * interface to the editor replies with all variables between
 * nladdrs and opt_e. See the file /lib/ed.h for the structure.
 */

EXT	int nladdrs;
EXT	int laddr1;
EXT	int laddr2;

EXT	int curln;
EXT	int curin;
EXT	int curcol;
EXT	int cmdin;
EXT	int lastln;
EXT	int center_line;
EXT	int nmarks;
EXT	int marker1;
EXT	int marker2;
EXT	int limit1;
EXT	int limit2;
EXT	int left_margin;
EXT	int right_margin;

EXT	int screen_row;
EXT	int screen_col;
EXT	char cursor_row;
EXT	char cursor_col;
EXT	char cmd_cursor_col;

EXT	char cc_reg;
EXT	int devno;

EXT struct opt opt;
EXT	char curfile[FNAME_SIZE + 1];
EXT	char opt_e;
EXT	struct opt opt_stack;
EXT	struct opt opt_save;

EXT	char opt_chars[16];

EXT	unsigned msg_tid;
EXT	int redisp_line;
EXT	int nsubs;
EXT	int del_index;
EXT	int del_cnt;
EXT	int ln_length;
EXT	int curln_save;
EXT	int curin_save;
EXT	int curcol_save;
EXT	int lastln_save;
EXT	char stat_str[30];
EXT	int screen_height, screen_width;

EXT	char *macro_ptr, macro_flags;
EXT	struct macro_entry *hash_tab[HASH_TAB_SIZE];
EXT	struct line *lastp;
EXT	struct line *firstp;
EXT	struct line *purge_ptr;
EXT	struct line *purge_end_ptr;
EXT	int npurge_lns;
EXT	char purge_type;

EXT	char lbuff[LINE_LENGTH + 3];
EXT	char dbuff[LINE_LENGTH + 1];
/*
 * The following three buffers will be allocated. Note that they
 * are used as a file pointer on each file r,w or e.
 * This will guarrentte that we always have space for a file pointer.
 * The three buffers must be at least as big as a file pointer which is
 * 512 + about 20.
 */

EXT	char *rbuff;	/* make LINE_LENGTH + 3 */
EXT	char *tbuff;	/* make PAT_SIZE + 2 */
EXT	char *pattern;	/* make PAT_SIZE + 2 */

EXT	char *lp;

EXT	FILE *ed_fp;
EXT	FILE *x_fp;

EXT	int nmarks_save;
EXT	int cmd_status;
EXT	int auto_save_cnt;
EXT	char msg_char;
EXT	char dirty;
EXT	int prnt_row;
EXT	char clr_flag;
EXT	char lock_cursor;
EXT	char fill_char;
EXT	char recall_char;
EXT	unsigned attributes[3];
EXT	char view_quick;
EXT	char limit_save;

EXT	int macro_level;
EXT	struct macro_stk_entry macro_stk[MACRO_LEVELS + 1];
EXT	char *learn_ptr;
EXT	char escape_char;
EXT int learn_index;

EXT	int msgrow;
EXT	int msgcol;
EXT	unsigned compose_char;
EXT	unsigned macro_disable_char;
EXT	unsigned delete_line;
EXT	unsigned num_delete_lines;

EXT	char *error_msgs[20];
EXT	char *macro_file;
EXT	char openerrflag;
EXT	char memory_mapped, windowed;
EXT	char dev_type;
EXT	char file_type;

EXT	int file_attrs;
EXT char restrict_flag;
EXT char browse_flag;
EXT char tab_len;

EXT int recall_lineno;
EXT char recall_buf[MAX_RECALL_LINES][LINE_LENGTH];
