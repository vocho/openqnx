%{
   
/*
 * Scanner for bc, pretty straightforward, but doesn't support bc's
 * number of type "100 200 300" to be "100200300"
 */

#include <libc.h>
#include <stdlib.h>
#include "parse.h"
#include "number.h"
#include "bc.h"


int lineno=0;
void warning();
int yyerror(char *s);
int map_escape(int i);
int nextline(FILE *f, char *buf, int max_size);
char   *input_line( FILE *, char *, int );
#undef yywrap

#ifdef SMALL_BUF
#undef YY_READ_BUF_SIZE
#define YY_READ_BUF_SIZE 512
#endif

#undef yywrap

#undef  YY_INPUT
#define YY_INPUT(buf,result,max_size) result=nextline(yyin,buf,max_size)
%}

DIGIT [0-9A-F]
WHITE [ \t]
NUMCHR [0-9A-F]
INUMCHR [0-9A-Fa-f]
%%
auto       {    return yylval.i_val =AUTO;      }
break      {    return yylval.i_val =BREAK;     }
continue   {    return yylval.i_val =CONTINUE;  }
define   {    return yylval.i_val =DEFINE;    }
for        {    return yylval.i_val =FOR;       }
ibase      {    return yylval.i_val =IBASE;     }
if         {    return yylval.i_val =IF;        }
length     {    return yylval.i_val =LENGTH;    }
obase      {    return yylval.i_val =OBASE;     } 
return     {    return yylval.i_val =RETURN;    }
scale      {    return yylval.i_val =SCALE;     }
sqrt       {    return yylval.i_val =SQRT;      }
while      {    return yylval.i_val =WHILE;     }

equals     {    return yylval.i_val ='=',ASSIGN;}
equal      {    return yylval.i_val = EQ;       }
plus       {    return yylval.i_val ='+';       }
minus      {    return yylval.i_val ='-';       }
times      {    return yylval.i_val ='*';       }
divide     {    return yylval.i_val ='/';       }
remainder  {    return yylval.i_val ='%';       }
or         {    return yylval.i_val = LOR;      }
and        {    return yylval.i_val = LAND;     }
print      {    return yylval.i_val = PRINT_LIST; }

[+\-*/%^<>{}\[\]();,]  { return yylval.i_val =yytext[0]; }

"="     {    return yylval.i_val = '=',ASSIGN; }
"++"    {    return yylval.i_val =  PLUSPLUS;    }
"--"    {    return yylval.i_val =  MINUSMINUS; }
"+="    {    return yylval.i_val =  ASG_PLUS,ASSIGN; }
"-="    {    return yylval.i_val =  ASG_MINUS,ASSIGN; }
"*="    {    return yylval.i_val =  ASG_STAR,ASSIGN; }
"/="    {    return yylval.i_val =  ASG_DIV,ASSIGN; }
"%="    {    return yylval.i_val =  ASG_MOD,ASSIGN; }
"^="    {    return yylval.i_val =  ASG_EXP,ASSIGN; }
"=="    {    return yylval.i_val =  EQ; }
">="    {    return yylval.i_val =  GE; }
"<="    {    return yylval.i_val =  LE; }
"!="    {    return yylval.i_val =  NE; }
"||"    {    return yylval.i_val = LOR; }
"&&"    {    return yylval.i_val = LAND; }
\"        {    
		int c;
		int    len=0;        /* remove initial '"' character */
		    while ((c=input()) != EOF) {
		        if (c == '\\')
		            c = map_escape(c);
		        else if (c == '\"')
		            break;
		        yytext[len++] = c;
		    }
		    yytext[len] = '\0';
		    yylval.s_val =strdup(yytext);
		    return '\"';
		}

\/\*    {
		    /*    eat the comments */
		    int c;
		    int state = 0;
		    while ((c=input()) != EOF && state != 99) {
		    	if (c == '\n') lineno++;
		        switch (state) {
		        case    0:
		            if (c == '*')
		                state = 1;
		            break;
		        case    1:
		            if (c == '/')
		                state = 99;
		            else 
		                state = c == '*';
		        }
		    }
		    if (state != 99)    {
		        /* end-of-file in comment */
		        fprintf(stderr,"warning, end of file inside comment\n");
		    }
		}


[a-z]    {
          	yylval.i_val = yytext[0]-'a';
          	return IDENT;
         }

{DIGIT}({NUMCHR}|\\\n)*(\.({NUMCHR}|\\\n)*)?  { yylval.s_val = strdup(yytext);
		                                        return CONST; }
\.({NUMCHR}|\\\n)+                             { yylval.s_val = strdup(yytext);
		                                        return CONST; }
(0[xX])?({INUMCHR}|\\\n)*(\.({INUMCHR}|\\\n)*)?   { yylval.s_val = strdup(yytext);
		                                        return CONST; }
\n        { lineno++; return yylval.i_val = '\n';}
[ \t]+    {  /* skip spaces, tabs */ }
^!.+\n    {	lineno++;
		yytext[strlen(yytext)-1] = '\0';
		system(yytext+1);
	}
.       { yyerror("unrecognized character\n"); }

%%


int   exit_code = 0;

#define MAX_FILES 64

char *flist[MAX_FILES];
int  fhead=0; int ftail=0;
int  nfiles;
static int firstread=1;
static char *curfname ="";


#define get_curf() curfname
/*
 * Bc processes a list of files before stdin.
 * so we setup yyin to be the first file in the
 * list, and after the last file is exhausted,
 * set yyin to stdin.
 * After that,
 */
int yywrap()
{
	while (fhead < ftail) {
		char   *s=flist[fhead++];
		if (strcmp(s, "-") == 0) {
			yyin = stdin;
			curfname = "-stdin-";
			return 0;
		}
		if ((yyin=fopen(s,"r"))) {
			curfname = s;
			return 0;
		}
		error(0,"unable to open '%s':%s\n", s, strerror(errno));
	}
	return 1;
}

int pushf(char *fn)
{
	if (ftail < MAX_FILES) {
		flist[ftail++] = fn;
		return 0;
	}
	errno = ENOSPC;
	return 1;
}

int parse_error(s)
	char    *s;
{
	error(0,"<%s>, line %d: %s near <%s>\n",
		            get_curf(),lineno+1,s,yytext);
	return 1;
}


int yyerror(s)
	char    *s;
{
	warning();
	return 1;
}

int nextline(FILE *f, char *buf, int max_size)
{
	int	len;
	if (firstread) {
		firstread = 0;
		return 0;
	}

	if (f==0) return 0;
	if (input_line(f,buf,max_size-2) == 0) return 0;
	len = strlen(buf);
	if (buf[len-1] != '\\') {
		buf[len++] = '\n';
  	}
	buf[len] = '\0';
	return len;
}



#define add_hex(_lval,_h)    (((_lval) << 4) + (_h))
#define add_oct(_lval,_o)    (((_lval) << 3) + (_o))

#define MAX_COUNT     3

int
map_escape(c)
	int        c;
{
	int        x = c;
	int        state = 0;
	int        ival = 0;    
	int        count = 0;

	while (c != EOF) {
		switch (state) {
		case    0:
		    switch (c) {
		    case    '\\':            /* escape character */
		        state = 1;
		        c = input();
		        break;
		    default:
		        return c;
		    }
		    break;            
		case    1:
		    switch (c) {            /* check for std escape sequences */
		    case    'n':    return '\n';
		    case    't':    return '\t';
		    case    'b':    return '\b';
		    case    'r':    return '\r';
		    case    'f':    return '\f';
		    case    'a':    return '\a';
		    case    'v':    return '\v';
		    case    '\\':    return '\\';
		    case    '\?':    return '\?';
		    case    '\'':    return '\'';
		    case    '\"':    return '\"';

		    case    'x':
		    case    'X':
		        x = c;
		        state = 2;
		        count = 0;
		        c = input();
		        break;
		    default:
		        if (isdigit(c) && c < '8'){
		            count = 0;
		            state = 3;
		        } else
		            return c;
		    }
		    break;
		case    2:
		    if (isxdigit(c) && count++ < MAX_COUNT)    {
		        ival = add_hex(ival, c <= '9' ? c-'0' : toupper(c)-'A'+10);
		        c = input();
		    } else {
		        unput(c);
		        if (count == 1) {
		            return x;
		        }
		        return ival;
		    }
		    break;

		case    3:
		    if (isdigit(c) && c < '8' && count++ < MAX_COUNT) {
		        ival = add_oct(ival,c-'0');
		        c = input();
		    } else {
		        if (count == 1) {
		            return c;
		        }
		        unput(c);
		        return ival;    
		    }
		    break;
		default:
		    error(1,"error, impossible state for LIT\n");
		    exit(-1);
		}
	}
	error(0, "end of file in literal\n");
	return -1;
}
