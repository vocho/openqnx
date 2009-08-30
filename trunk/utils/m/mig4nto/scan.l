D           [0-9]
L           [a-zA-Z_]
H           [a-fA-F0-9]
E           [Ee][+-]?{D}+
FS          (f|F|l|L)
IS          (u|U|l|L)*
W			[ \t\v\f]
NL			\n

%{
#include <stdio.h>
#include <stdlib.h>

#ifdef	C86
#	include <io.h>
#else
#	include <unistd.h>
#endif

#ifdef		yywrap
#	undef	yywrap
#endif

#include	"mig.h"

/*
**	I had to redefine the input routine because the current
**	scanner only supports 7-bit ASCII.  The read routine strips
**	off the high order bit and allows the scanner to process the
**	7 bit data.
*/
#undef		YY_INPUT
#define		YY_INPUT( buf, result, max_size )								\
       if ( ((result) = read( fileno(yyin), (buf), (max_size) )) < 0 )		\
           YY_FATAL_ERROR( "read() in flex scanner failed" );				\
	else if ( (result) > 0 )											\
	{	int	i;															\
																		\
		for ( i = 0; i < (result); i++ )								\
			if ( (buf)[i] == LF )										\
				(buf)[i]  = '\n';										\
			else if ( (unsigned char) ((buf)[i]) > 127 )				\
				(buf)[i] = ' ';											\
	}

%}

%x	preprocess string comment ccomment
                  
%%

"auto"          		|
"break"         		|
"case"          		|
"char"          		|
"const"         		|
"continue"      		|
"default"       		|
"do"            		|
"double"        		|
"else"          		|
"enum"          		|
"extern"        		|
"float"         		|
"for"           		|
"goto"          		|
"if"            		|
"int"           		|
"long"          		|
"register"      		|
"return"        		|
"short"         		|
"signed"        		|
"sizeof"        		|
"static"        		|
"struct"        		|
"switch"        		|
"typedef"       		|
"union"         		|
"unsigned"      		|
"void"          		|
"volatile"      		|
"while"         		;

'(\\.|[^\\'])+'     	;

{L}({L}|{D})*			return IDENTIFIER;

^{W}*#					BEGIN preprocess;
<preprocess>.			;
<preprocess>\\{NL}		{ yyeol(); LineCount++; }
<preprocess>{NL}		{ yyeol(); LineCount++; BEGIN 0; }

"/*"					BEGIN comment;
<comment>.				;
<comment>{NL}			{ yyeol(); LineCount++; }
<comment>"*/"			BEGIN 0;

"//"					BEGIN ccomment;
<ccomment>.				;
<ccomment>{NL}			{ yyeol(); LineCount++; BEGIN 0; }

\"						BEGIN string;
<string>\\\"			;
<string>\\{NL}			{ yyeol(); LineCount++; }
<string>[^"]			;
<string>\"				BEGIN 0;

"-}"					return QNX2_PTR_OP2;
"@"						return QNX2_PTR_OP1;

{NL}					{ yyeol(); LineCount++; }

.           			;

%%


/* End of File */
