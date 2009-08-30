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




#define DEFINE 257
#define IF 258
#define WHILE 259
#define FOR 260
#define BREAK 261
#define QUIT 262
#define AUTO 263
#define RETURN 264
#define CONTINUE 265
#define LEX_ERROR 266
#define PUSHL 267
#define PUSHR 268
#define DISCARD 269
#define SAVEV 270
#define RESTV 271
#define POPV 272
#define JMP 273
#define JMPZ 274
#define JMPNZ 275
#define CALL 276
#define PRINT 277
#define SAVEVAR 278
#define RESTVAR 279
#define SAVEARRAY 280
#define RESTARRAY 281
#define INDEX 282
#define PUSHVAR 283
#define PUSHCONST 284
#define PUSHARRAY 285
#define PUSHSPECIAL 286
#define POPVAR 287
#define POPARRAY 288
#define REPLACE 289
#define CLRSTK 290
#define CONST 291
#define IDENT 292
#define PRINT_LIST 293
#define IBASE 294
#define OBASE 295
#define SCALE 296
#define SQRT 297
#define LENGTH 298
#define EQ 299
#define GE 300
#define LE 301
#define NE 302
#define PLUSPLUS 303
#define MINUSMINUS 304
#define ASG_PLUS 305
#define ASG_MINUS 306
#define ASG_STAR 307
#define ASG_DIV 308
#define ASG_MOD 309
#define ASG_EXP 310
#define UMINUS 311
#define ASSIGN 312
#define LOR 313
#define LAND 314
typedef union   {
     int    i_val;
     char   *s_val;
} YYSTYPE;
extern YYSTYPE yylval;
