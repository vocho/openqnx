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
 * April 1980 by  D. T. Dodge
 */

#define VERSION	'1'

#define OK	0
#define NOTHING	-2
#define ERROR	-3
#define ERROR0 -3
#define ERROR1 -4
#define ERROR2 -5
#define ERROR3 -6
#define ERROR4 -7
#define ERROR5 -8
#define ERROR6 -9
#define ERROR7 -10
#define ERROR8 -11
#define ERROR9 -12
#define ERROR10 -13
#define ERROR11 -14
#define ERROR12 -15
#define ERROR13 -16
#define ERROR14 -17
#define ERROR15	-18

#define NOT_TRUE_OR_FALSE	2

#define MACRO_LEVELS	8
#define LEARN_BUF_SIZE	400
#define LINE_LENGTH 512
#define PAT_SIZE 140
#define FNAME_SIZE 64
#define HASH_TAB_SIZE	16
#define NUM_OPTIONS		13
#define SAVE_THRESHOLD	20

#define TEXT	0
#define CMD		1

#define INACTIVE	0
#define ACTIVE		1

#define FORWARD 0
#define BACKWARD 1

#define NOGLOB 0
#define GLOB 1

#define GLOB_FLAG	0x01
#define DIRTY_FLAG	0x02
#define CONT_FLAG	0x04
#define OVER_FLAG	0x08
#define PARA_FLAG	0x10
#define STOP_FLAG	0x20
#define NEW_FLAG	0x40

#define NOSAVE	0
#define SAVE	1

#define SINGLE		0
#define MULTIPLE	1

#define FALSE	0
#define TRUE	1

#define CMD_CHAR			0xff
#define RECALL_CHAR			0xfe
#define INPUT_CHAR			0xfd
#define RECALL2_CHAR		0xfc
#define CR_					'\015'
#define LF_					'\012'
#define RS_					'\036'

#define STATUS_AREA		0
#define CMD_AREA		1
#define TEXT_AREA		2

#define SCREEN_OFFSET	2

#define NSCREEN_SIZES	6

#define MAX_RECALL_LINES	15

extern int exec_cmd(int ,int );
extern int append(int ,char );
extern int prnt(int ,int );
extern int delete(int ,int ,char );
extern int undelete(int );
extern int kopy(int );
extern int move(int );
extern int _read(char *,int ,char );
extern int _write(char *,int ,int ,char ,char );
extern int substitute(char *,char ,int );
extern int getsubst_str(char *);
extern char *subst_txt(char *,char *,char *,char *);
extern char *subst_same(char *,char *,char *);
extern int join(int );
extern int exec_file(char *,int ,int );
extern int yut(void);
extern struct __iobuf *ed_open(char *,char *,char );
extern void ed_close(void);
extern void get_fattrs(struct __iobuf *);
extern char *esc_line(char *);
extern char *esc_char(char *,char *);
extern void expand(char );
extern int hex(char );
extern int exec_sys_cmd(unsigned int ,unsigned int );
extern void msg_input(void);
extern void disp_stats(void);
extern int col_to_index(char *,int );
extern int index_to_col(char *,int );
extern char *cmd_input(char *);
extern void change_state(char );
extern int exec_glob(char ,char );
extern void update_screen(void);
extern void move_cursor(signed char ,signed char ,signed char ,signed char *,signed char *,signed char );
extern void disp_screen_image(int );
extern void hatch_cmd_line(void);
extern void disp_line_image(int ,char *,unsigned int ,char );
extern void put_screen(unsigned int ,unsigned int ,unsigned int ,unsigned int );
extern void locate_cursor(char ,char );
extern int get_term_char(void);
extern void device_setup(void);
extern void brk1(void);
extern void clear_screen1(int );
extern void setdown(void);
extern void zap_func(int );
extern void put_stat(int ,char *,int );
extern void put_option(int ,char );
extern int zap_fill(int ,int );
extern void justify(char *,int ,int );
extern int zap_center(int );
extern void dummy(void);
extern int learn(void);
extern int getladdr(void);
extern int get_laddr_expr(int *);
extern int getrange(void);
extern int nextln(int );
extern int prevln(int );
extern int patscan(char );
extern unsigned int mgetchar(void);
extern struct macro_entry *lookup(char );
extern int install(char ,char *,char );
extern int get(void);
extern int main(int ,char **);
extern void ed(int ,char **);
extern int exec_line(int ,int );
extern void quit(int );
extern void rip_out(int);
extern int zap_restore(int );
extern int option(void);
extern int match(char *,char *,char ,int );
extern char *amatch(char *,char *,char *);
extern int match_char(char **,char *,char *);
extern int member_of(char *,char );
extern int patsize(char *);
extern int getpat(void);
extern int makepat(char );
extern int getccl(char **,char **);
extern void insert_clos(char *,char *);
extern int comp(char ,char );
extern void breakable(void );
extern void unbreakable(void );
extern int fput(char *,int ,struct __iobuf *);
extern int fgetline(struct __iobuf *,char *,int );
extern void forward(struct __iobuf *);
extern unsigned int translate_key(unsigned int );
extern void report(char *,int );
extern void dump(void);
extern struct line *getptr(int );
extern int addline(char *,char ,char );
extern void relink(struct line *,struct line *);
extern char *store_line(char *,char *,int );
extern int initbuffer(void);
extern void init_externs(void);
extern int len(char *);
extern int getfn(void);
extern void set_fn(char *,char *);
extern int getint(void);
extern void putln(struct __iobuf *,char *,char );
extern void dtoc(char *,int );
extern void puterr(int );
extern void putmsg(char *);
extern void mark_line(int );
extern void prnt_screen(char *,char );
extern void purge(struct line *,int );
extern void *ccalloc(int );
extern int imin(int ,int );
extern int imax(int ,int );
extern void clear_term_screen(void);
extern void put_term_char(unsigned int ,unsigned int ,char ,unsigned int );
extern void put_term_line(int ,int ,char *,int ,unsigned int );
extern int num_nonblank_chars(char *,int );
extern int num_unmatched_chars(int ,int ,char *,int ,unsigned int );
extern int map_term_line(char *,char *,int );
extern void init_terminal(void);
extern void del_term_line(int ,int );
extern void ins_term_line(int ,int );
extern int translate(int );
extern int until(int ,int ,int );
extern int branch(void);
extern int get_cond(void);
extern int view(int );
extern int zap(void);
extern int zap_char(int );
extern int zchar_change(int ,int ,char *,int ,int );
extern int zchar_delete(int ,int ,int );
extern int zchar_erase(int ,int ,int );
extern int zchar_save(int ,int ,int );
extern int replace(int ,char *,char *);
extern int horizontal(int ,int ,char *);
extern void save_char(char );
extern int zap_line(int );
extern char *getfname(void);
extern void set_memory_mapped( void );
extern void windows_size( void );
int default_macros();
