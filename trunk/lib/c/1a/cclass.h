/*
Copyright 1992, 1993, 1994, 1997 Henry Spencer.  All rights reserved.
This software is not subject to any license of the American Telephone
and Telegraph Company or of the Regents of the University of California.

Permission is granted to anyone to use this software for any purpose on
any computer system, and to alter it and redistribute it, subject
to the following restrictions:

1. The author is not responsible for the consequences of use of this
   software, no matter how awful, even if they arise from flaws in it.

2. The origin of this software must not be misrepresented, either by
   explicit claim or by omission.  Since few users ever read sources,
   credits must appear in the documentation.

3. Altered versions must be plainly marked as such, and must not be
   misrepresented as being the original software.  Since few users
   ever read sources, credits must appear in the documentation.

4. This notice may not be removed or altered.
*/
/* character-class table */
static const struct cclass {
	char *name;
	char *chars;
	char *multis;
} cclasses[] = {
	{ "alnum",	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",				"" },
	{ "alpha",	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 					"" },
	{ "blank",	" \t",		"" },
	{ "cntrl",	"\007\b\t\n\v\f\r\1\2\3\4\5\6\16\17\20\21\22\23\24\25\26\27\30\31\32\33\34\35\36\37\177",	"" },
	{ "digit",	"0123456789",	"" },
	{ "graph",	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~", 					"" },
	{ "lower",	"abcdefghijklmnopqrstuvwxyz", 					"" },
	{ "print",	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ ", 					"" },
	{ "punct",	"!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~", 					"" },
	{ "space",	"\t\n\v\f\r ",	"" },
	{ "upper",	"ABCDEFGHIJKLMNOPQRSTUVWXYZ", 					"" },
	{ "xdigit",	"0123456789ABCDEFabcdef", 					"" },
	{ NULL,		0,		"" }
};

/* __SRCVERSION("cclass.h $Rev: 153052 $"); */
