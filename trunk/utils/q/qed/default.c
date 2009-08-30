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





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

char *macros[] ={
"T \\a7 \\ffoc?b2f\\0a\\0a\\ffoc+\\0a\n",
"T \\af \\ffoc?b2f\\0a\\0a\\ffoc+\\0a\n",
"T \\a1 \\ff.-1|zcl\\0a\n",
"T \\a4 \\ff%zch-1\\0a\n",
"T \\a6 \\ff%zch+1\\0a\n",
"T \\a9 \\ff.+1|zcl\\0a\n",
"T \\b1 \\ff.-4|zcl\\0a\n",
"T \\b9 \\ff.+4|zcl\\0a\n",
"T \\b6 \\ffoe+om+zch+0b2t\\0a\\ffoa+/[^ 	]/;b2\\0a\\ffoa-.-1;/@(.)[^	 ]*[\\09 ]*/\\0a\\ffoe-\\0a\n",
"T \\b4 \\ffoe+om+zch-1b1tzch+0b2tzch$b3\\0a\\ffoa-zch?[ 	]*[^ 	][^ 	]*?b1fzch/^[ 	]/b2\\0a\\ffoa+zch0?^.?zch$\\0a\\ffoe-\\0a\n",
"T \\02 \\ffvl+4zchl\\0a\n",
"T \\05 \\ffvl-4zchl\\0a\n",
"T \\a0 \\ff1zch1\\0a\n",
"T \\a8 \\ff$zch1\\0a\n",
"T \\b0 \\ff&zch1\\0a\n",
"T \\b8 \\ff&+22|zch1\\0a\n",
"T \\b8 \\ffu1tzv0\\0a\\ffb1f&+~-1|;b2\\0a\\ff&+22|\\0a\n",
"T \\a2 \\ffu1tzv0\\0a\\ffb1f@-~|;b2\\0a\\ff@-23|\\0a\n",
"T \\aa \\ffu1tzv0\\0a\\ffb1f@+~|;b2\\0a\\ff@+23|\\0a\n",
"T \\b2 \\ffvs-1\\0a\n",
"T \\ba \\ffvs1\\0a\n",
"T \\a5 \\ffvc\\0a\n",
"T \\ac \\ff%zcs%zcd\\0a\n",
"T \\bc \\ffzcp\\0a\n",
"T \\7f \\ff%zch-1%zcd\\0a\n",
"T \\17 \\ffzcer\\0a\n",
"T \\18 \\ff%zch1%zce$\\0a\n",
"T \\19 \\ff%zce$\\0a\n",
"T \\ab \\ffoi~\\0a\n",
"T \\bb \\ffoe+oi+%zcroe-\\0a\n",
"T \\80 \\ff%zch1\\0a\n",
"T \\9f \\ff%zch$\\0a\n",
"T \\81 \\ffon?b3t\\0a\\ffa\\0a\\ffb4\\0a\\ffon-zch1\\0a\\ffb2t\\0a\\ffd.+1|;b1f.-2|\\0a\n",
"T \\82 \\ffon?b3t\\0a\\ffi\\0a\\ffb4\\0a\\ffon-zch1\\0a\\ffb2t\\0a\\ffd\\0a\n",
"T \\95 \\ffad\\0a\n",
"T \\96 \\ffid\\0a\n",
"T \\83 \\ffd\\0a\n",
"T \\84 \\ffzq#b1f#zlfb2\\0a\\ffoe+ow-.,/^[ 	]*$/|zlfoe-\\0a\n",
"T \\98 \\ffzq#b1f#zlcb2\\0a\\ff.zlc\\0a\n",
"T \\85 \\ffoe+oa+oi-om+zch/@(1) /b1toi+\\0a\\ffzcc\\0a\\ffoe-\\0a\n",
"T \\86 \\ffzch$zch-1zch/ /b1toe+om+s/$/ /oe-\\0a\\ffj\\0a\n",
"T \\99 \\ffoe+oi+zcc\\0a\\ffoe-\\0a\n",
"T \\9a \\ffzch$j\\0a\n",
"T \\87 \\ffzlt\\0a\n",
"t \\88 \\ffol?b9ty\"d-delete k-kopy m-move s-save p-print\"dkmsspp \"\\0a\\ff#db17\\0a\\ff#k.b16\\0a\\ff#m.b15\\0a\\ff#w @\\0a\\ffb13\\0a\\ff#w $lpt\\0a\\ffb11\\0a\\ffn\\0a\\fc\n",
"T \\fc \\ffy\"d-delete e-erase k-kopy m-move/erase M-move/delete s-save p-print\"dekmMsp \"\\0a\\ff#zldb8\\0a\\ff#zleb7\\0a\\ffzp#zlszlrb6\\0a\\ffzp#zlszlu#zlezlrb5\\0a\\ffzp#zlszlu#zldzlrb4\\0a\\ffzp#zlwb3\\0a\\ffzp#zlw\"$lpt\"b2\\0a\\ffn\\0a\n",
"\"T \\88 \\ffol?b5ty\"d-delete k-kopy m-move\"dkm \"\\0a\\ff#db11\\0a\\ff#k.b10\\0a\\ff#m.b9\\0a\\ffn\\0a\\ffy\"d-delete e-erase k-kopy m-move/erase M-move/delete\"dekmM \"\\0a\\ff#zldb6\\0a\\ff#zleb5\\0a\\ffzp#zlszlrb4\\0a\\ffzp#zlszlu#zlezlrb3\\0a\\ffzp#zlszlu#zldzlrb2\\0a\\ffn\\0a\n",
"T \\92 \\ffy\"l-line paste c-column paste\"llc \"\\0a\\ff.r @\\0a\\ffb2\\0a\\ffzpzlR\\0a\n",
"T \\9b \\ffzlu\\0a\n",
"T \\9c \\ffy\"Enter k-kopy, m-move\"km \"\\0a\\ff#k.-1b3\\0a\\ff#m.-1b2\\0a\\ffn\\0a\n",
"T \\89 \\fe\\ffoc+\\0a\\0a\n",
"T \\8A \\fe\\ff0zch$oc+\\0a\n",
"T \\04 \\fcn\\ff0zch$oc+\\0a\n",
"T \\15 \\fcp\\ff0zch$oc+\\0a\n",
"T \\8b \\ffoc-vl.\\0a\n",
"T \\8c \\ffoc-vr.\\0a\n",
"T \\8d \\ffoc-vl-1zchl\\0a\n",
"T \\8e \\ffoc-vl+1zchl\\0a\n",
"T \\8f \\ffoc-vr-1zchr\\0a\n",
"T \\90 \\ffoc-vr+1zchr\\0a\n",
"T \\93 \\ffzchl\\0a\n",
"T \\94 \\ffzchr\\0a\n",
"T \\b7 \\ffzmPlease use Ctrl - to learn\\0a\n",
"T \\b3 \\ffoe+oc+0zch20zmEnter key to learn\\0a\\ffl \\fd\\0a\\ffoe-vs0\\0a\n",
"T \\e1 \\ffoa~\\0a\n",
"T \\e2 \\ffob~\\0a\n",
"T \\e4 \\ffod~\\0a\n",
"T \\e6 \\ffof~\\0a\n",
"T \\ea \\ffoj~\\0a\n",
"T \\ec \\ffol~\\0a\n",
"T \\ed \\ffom~\\0a\n",
"T \\ef \\ffzlo\\0a\n",
"T \\f0 \\ffzlp\\0a\n",
"T \\e3 \\ffzlj\\0a\n",
"T \\f2 \\ffor~\\0a\n",
"T \\f3 \\ffos~\\0a\n",
"T \\f4 \\ffot~\\0a\n",
"T \\f7 \\ffow~\\0a\n",
"T \\fa \\ffvz\\0a\n",
"T \\1A \\ff!sh\\0a\n",
"\"The following commented out macro may be used as an aid in defining macros.\n",
"\"Read the section on MACROS in the ED manual.\n",
"\"t \\ed \\ffoc?b7f*da\\0a\\ffon-om+0zk1\\0a\\ffu1 s/\\\\\\\\\\\\\\\\/\\\\80/\\0a\\ffu1 s/\\\\\\\\0a/\\\\0a/\\0a\\ffu1 *s/\\\\80/\\\\\\\\\\\\\\\\/\\0a\\ff0zce255 1zch1b5\\0a\\ffom+\\0a\\ffu1 *s/$/\\\\\\\\0a/\\0a\\ffu40 1j\\0a\\ff1zk0zch1oc+\\0a\n",
"vc3\n",
"vr60\n",
"va1 6 1\n",
"va2 0 6\n",
"va3 7 1\n",
"T \\0f \\11\n",
NULL
} ;

int default_macros() {
	int status;
	char **s = macros;

	status = OK;
	while(*s  &&  status == OK) {
		strcpy(lp = lbuff, *s++);
		status = exec_line(0, 0);
		}

	lp = "\n";	/* to force a fetch of a new line */
	return(status);
	}
