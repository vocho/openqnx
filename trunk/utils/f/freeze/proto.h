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





//#line "bitio.c" 7
extern short GetByte(void);
//#line "bitio.c" 36
extern short GetNBits(u_short n);
//#line "bitio.c" 71
extern void Putcode(u_short l,u_short c);
//#line "decode.c" 7
extern void melt(void);
//#line "default.c" 18
extern int defopen(char *fname);
//#line "default.c" 39
extern char *defread(char *pattern);
//#line "encode.c" 9
extern void freeze(void);
//#line "freeze.c" 43
extern void Usage(void);
//#line "freeze.c" 159
extern int main(int argc,char **argv);
//#line "freeze.c" 596
extern void prratio(struct __iobuf *stream,long num,long den);
//#line "freeze.c" 621
extern char *freeze_rindex(char *s,int c);
//#line "freeze.c" 631
extern void writeerr(void);
//#line "freeze.c" 640
extern void copystat(char *ifname,char *ofname);
//#line "freeze.c" 719
extern int foreground(void);
//#line "freeze.c" 736
extern void onintr(int);
//#line "freeze.c" 747
extern void oops(int);
//#line "freeze.c" 754
extern void version(void);
//#line "freeze.c" 789
extern void tune_table(char *type);
//#line "huf.c" 53
extern void StartHuff(void);
//#line "huf.c" 90
extern void reconst(void);
//#line "huf.c" 140
extern void update(u_short c);
//#line "huf.c" 176
extern void EncodeChar(u_short c);
//#line "huf.c" 206
extern void EncodePosition(u_short c);
//#line "huf.c" 219
extern void EncodeEnd(void);
//#line "huf.c" 229
extern short DecodeChar(void);
//#line "huf.c" 262
extern short DecodePosition(void);
//#line "huf.c" 362
extern void init(unsigned char *table);
//#line "huf.c" 393
extern void write_header(void);
//#line "huf.c" 409
extern int read_header(void);
//#line "huf.c" 450
extern void corrupt_message(void);
//#line "lz.c" 62
extern void InitTree(void);
//#line "lz.c" 76
extern void Get_Next_Match(u_short r);
