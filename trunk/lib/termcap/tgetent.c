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




#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#ifdef __QNXNTO__
  #include <spawn.h>
  #include <termios.h>
#else
  #include <sys/qnx_glob.h>
#endif

static char *tent;		/* pointer to termcap buffer */

/* Build in the most common terminals */
char *ansi_termcap =
"ansi|generic ansi standard terminal:"
"	:am:xo:"
"	:co#80:li#33:"
"	:bt=\\E[Z:bl=^g:cr=\\r:ct=\\E[2g:cl=\\E[H\\E[J:ce=\\E[K:cd=\\E[J:"
"	:ch=\\E[%p1%{1}%+%dG:cm=\\E[%i%d;%dH:ho=\\E[H:le=\\b:nd=\\E[C:"
"	:up=\\E[A:dc=\\E[P:dl=\\E[M:mb=\\E[5m:md=\\E[1m:mk=\\E[8m:mr=\\E[7m:"
"	:so=\\E[7m:us=\\E[4m:me=\\E[0m:se=\\E[m:ue=\\E[m:ic=\\E[@:al=\\E[L:"
"	:kb=\\b:kd=\\E[B:kh=\\E[H:kl=\\E[D:kr=\\E[C:ku=\\E[A:DL=\\E[%dM:"
"	:DO=\\E[%dB:IC=\\E[%d@:AL=\\E[%dL:LE=\\E[%dD:RI=\\E[%dC:UP=\\E[%dA:"
"	:rp=%.\\E[%p2%{1}%-%db:cv=\\E[%p1%{1}%+%dd:"
"	:sa=\\E[%?%p1%t7;%;%?%p2%t4;%;%?%p3%t7;%;%?%p4%t5;%;%?%p6%t1;%;m:"
"	:st=\\EH:ta=\\t:"
"	:kN=\\E[U:kP=\\E[V:";

#ifndef __QNXNTO__
char *qnx_termcap =
"qnx|qnx4|qnx console:"
"	:km:mi:ms:xt:YA:YC:"
"	:co#80:it#4:li#33:Co#8:pa#64:NC#3:"
"	:bl=^g:cr=\\r:cl=\\EH\\EJ:ce=\\EK:cd=\\EJ:cm=\\EY%+ %+ :do=\\l:"
"	:ho=\\EH:vi=\\Ey0:le=\\b:ve=\\Ey1:nd=\\EC:up=\\EA:vs=\\Ey2:dc=\\Ef:"
"	:dl=\\EF:mb=\\E{:md=\\E<:ti=\\Ei:mr=\\E(:so=\\E(:us=\\E[:"
"	:me=\\E}\\E]\\E>\\E):te=\\Eh\\ER:se=\\E):ue=\\E]:ic=\\Ee:al=\\EE:"
"	:ka=\\377\\344:kC=\\377\\341:kt=\\377\\237:kD=\\377\\254:kL=\\377\\274:"
"	:kd=\\377\\251:kM=\\377\\313:kE=\\377\\310:kS=\\377\\314:"
"	:kh=\\377\\240:"
"	:kI=\\377\\253:kA=\\377\\273:kl=\\377\\244:kN=\\377\\252:kP=\\377\\242:"
"	:kr=\\377\\246:kF=\\377\\261:kR=\\377\\271:kT=\\377\\342:ku=\\377\\241:"
"	:rp=\\Eg%+ %.:r1=\\ER:sf=\\l:sr=\\EI:"
"	:sa=%?%p1%t\\E<%;%p2%t\\E[%;%p3%t\\E(%;%p4%t\\E{%;%p6%t\\E<%;:"
"	:ta=\\t:"
"	:ac=l\\332m\\300k\\277j\\331q\\304x\\263u\\264t\\303n\\305v\\301w\\302O\\333a\\261o\\337s\\334:"
"	:kB=\\377\\200:"
"	:F1=\\377\\256:F2=\\377\\257:F3=\\377\\213:"
"	:F4=\\377\\214:F5=\\377\\215:F6=\\377\\216:F7=\\377\\217:F8=\\377\\220:"
"	:F9=\\377\\221:FA=\\377\\222:" \
"	:op=\\ER:Sf=\\E@%p1%Pf%gb%gf%d%d:Sb=\\E@%p1%Pb%gb%gf%d%d:"
"	:Zc=\\E!%d:";
#else
char *qansi_termcap =
"qansi|qansi-m|qansi-8859m|QNX ANSI:am:G0:co#80:it#8:li#25:ti=\\E[?7h:"
"	:AL=\\E[%dL:DC=\\E[%dP:DL=\\E[%dM:DO=\\E[%dB:IC=\\E[%d@:LE=\\E[%dD:"
"	:RA=\\E[?7l:RI=\\E[%dC:SA=\\E[?7h:SF=\\E[%dS:SR=\\E[%dT:UP=\\E[%dA:"
"	:ae=^O:al=\\E[L:as=^N:bt=\\E[Z:cb=\\E[K\\E[X:cd=\\E[J:ce=\\E[K:ch=\\E[%i%dG:"
"	:cl=\\E[2J\\E[H:cm=\\E[%i%d;%dH:cs=\\E[%i%d;%dr:ct=\\E[2g:dc=\\E[P:dl=\E[M:\"o=\\E[B:"
"	:ec=\\E[%dX:ho=\\E[H:ic=\\E[@:is=\\E[?7h\\E[0;10;39;49m:"
"	:k1=\\EOP:k2=\\EOQ:k3=\\EOR:k4=\\EOS:k5=\\EOT:k6=\\EOU:k7=\\EOV:k8=\\EOW:k9=\\EOX:"
"	:k;=\\EOY:kB=\\E[Z:kC=\\ENa:kD=\\E[P:kF=\\E[a:kI=\\E[@:kN=\\E[U:kP=\\E[V:kR=\\E[b:kT=\\ENb:"
"	:ka=\\ENd:kb=\\177:kd=\\E[B:kh=\\E[H:kl=\\E[D:kr=\\E[C:ku=\\E[A:"
"	:le=\\E[D:ll=\\E[99H:mb=\\E[5m:md=\\E[1m:me=\\E[m^O:mh=\\E[2m:mk=\\E[9m:mr=\\E[7m:"
"	:nd=\\E[C:nw=\\EE:op=\\E[39;49m:rp=%.\\E[%p2%{1}%-%db:"
"	:se=\\E[27m:sf=\\E[S:so=\\E[7m:sr=\\E[T:st=\\EH:ue=\\E[24m:up=\\E[A:us=\\E[4m:"
"	:ac=``aaffggjjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||}}~~:"
"	:vb=\\E[?5h\\E[?5l:ve=\\E[?25h\\E[?12l:vi=\\E[?25l:"
"	:ws=\\E[5m:we=\\E[m:bo=\\E[1m:be=\\E[m:";
#endif

/* search for capability within terminal entry */
static char *getcap(const char *cp) {
    char *tp;

    /* assert we did tgetent(), and that cp is a two-character capability */
    if ((tp = tent) == 0 || cp == 0 || cp[0] == 0 || cp[1] == 0)
	return 0;
    for (;;) {
	while (*tp && *tp++ != ':');
	if (*tp == 0) break;
	/* always do this one byte by one byte. this is the safest 
	 * and work for all platforms 
	 */
	if (tp[0] == cp[0] && tp[1] == cp[1])
	    return tp;
    }
    return 0;			/* capability not found */
}

/*
 * tgetent - get the termcap entry for terminal name, and put it
 * in bp (which must be an array of 1024 chars). Returns 1 if
 * termcap entry found, 0 if not found, and -1 if file not found.
 */
int tgetent(char *bp, const char *term) {
    char *tc, *cp;
    char tn[16];
    FILE *fp;

    if (term == 0) return 0;

    /*
     * If $TERMCAP does not begin with / it is assumed to
     * already contain the terminal capability description
     *
     * What happens if this contains a tc reference?
     */
    tent = bp;	/* point to termcap buffer */
    if (tc = getenv("TERMCAP")) {
	if (*tc != '/' && (cp = getenv("TERM")) && strcmp(term, cp) == 0) {
	    (void) strcpy(bp, tc);
	    return 1;
	}
    }
#ifndef __QNXNTO__
    else if((cp = getenv("TERM")) &&  strstr(cp, "ansi")) {
	    (void) strcpy(bp, ansi_termcap);
	    return 1;
	}
    else if(cp  &&  strstr(cp, "qnx")) {
	    (void) strcpy(bp, qnx_termcap);
	    return 1;
	}
#else
    else if((cp = getenv("TERM")) &&  strstr(cp, "qansi")) {
	    (void) strcpy(bp, qansi_termcap);
	    return 1;
	}
    else if(cp  &&  strstr(cp, "ansi")) {
	    (void) strcpy(bp, ansi_termcap);
	    return 1;
	}
#endif
    else
	tc = "/etc/termcap";

    strcpy(tn, term);
    /* open the termcap database */
    if (fp = fopen(tc, "r")) {
	int c, last = 0;

	for (bp = (tc = tent) - 1; (c = fgetc(fp)) != EOF; *++bp = last = c)
	    if (c == '\n')
		if (last == '\\') {
		    /* skip leading whitespace after escaped newline */
		    while ((c = fgetc(fp)) != EOF && (c == ' ' || c == '\t'));
		    --bp;
		} else {
		    /* nul terminate completed terminal description */
		    *++bp = 0, cp = tc;
		    do {    /* check if any terminal names match */
			for (c = 0; tn[c] && tn[c] == *cp; cp++, c++);
			if (tn[c] == 0 && (*cp == '|' || *cp == ':')) {
			    /* return if no terminal continuation */
			    if ((cp = getcap("tc")) == 0 || cp[2] != '=') {
				fclose(fp);
				return 1;
			    }
			    tc = cp, cp += 3; /* skip tc= */
			    /* copy new terminal name into tn */
			    for (c = 0; cp[c] != ':' && (tn[c] = cp[c]); c++);
			    tn[c] = 0;
			    rewind(fp);
			    break;  /* terminal continuation */
			}
			/* skip to next terminal name */
			while (*cp && *cp != '|' && *cp != ':') cp++;
		    } while (*cp++ == '|');
		    c = fgetc(fp);
		    bp = tc - 1;
		}
	fclose(fp);
    }

    {
	pid_t kid;
	int fd[2];
	int n;

	if (pipe(fd) == 0) {
	    static char *argv[] = { "infocmp", "-C", 0, 0 };
	    argv[2] = (char *) term;
	    fcntl(fd[0], F_SETFD, FD_CLOEXEC);
	    fcntl(fd[1], F_SETFD, FD_CLOEXEC);
	{
#ifdef __QNXNTO__
	    struct inheritance inherit;
	    memset(&inherit, 0, sizeof(inherit));
	    kid = spawnp(argv[0], 3, fd, &inherit, argv, environ);
#else
	    qnx_spawn_options.iov[1] = fd[1];
	    kid = spawnvp(P_NOWAIT, argv[0], argv);
#endif
	}
	    if (kid != -1  &&  waitpid(kid, &n, 0) != -1  &&  n == 0) {
		n = read(fd[0], tent, 1024);
		close(fd[0]), close(fd[1]);
		for (bp = tent + n; bp >= tent && *--bp != ':';);
		*++bp = '\0';
		return 1;
	    }
	    close(fd[0]), close(fd[1]);
	}
    }

    tent = 0;		/* no valid termcap */
    return 0;
}

/*
 * tgetflag - get the boolean flag corresponding to id. Returns -1
 * if invalid, 0 if the flag is not in termcap entry, or 1 if it is
 * present.
 */
int tgetflag(const char *id) {
    char *cp = getcap(id);

    return cp && cp[2] != '@' ? 1 : 0;
}


static int num_cols() {
	int col;

#ifdef __QNXNTO__
	if ( tcgetsize( 1, NULL, &col ) == 0 && col )
		return col;
#else
	int dev_size();
	if (dev_size(1, -1, -1, (int *) 0, &col) == 0 && col)
	    return col;
#endif

	return(0);
}


static int num_lines() {
	int lin;

#ifdef __QNXNTO__
	if ( tcgetsize( 1, &lin, NULL ) == 0 && lin )
		return lin;
#else
	int dev_size();
	if (dev_size(1, -1, -1, &lin, (int *) 0) == 0 && lin)
	    return lin;
#endif

	return(0);
}


/*
 * tgetnum - get the numeric terminal capability corresponding
 * to id. Returns the value, -1 if invalid.
 */
int tgetnum(const char *id) {
    char *cp;

    if (id[0] == 'l' && id[1] == 'i') {
	int lines;

	if (cp = getenv("LINES"))
	    return atoi(cp);
	else if (lines = num_lines())
	    return lines;
    } else if (id[0] == 'c' && id[1] == 'o') {
	int columns;

	if (cp = getenv("COLUMNS"))
	    return atoi(cp);
	else if (columns = num_cols())
	    return columns;
    }
    cp = getcap(id);
    return cp && cp[2] == '#' ? atoi(cp + 3) : -1;
}

/*
 * tgetstr - get the string capability corresponding to id and place
 * it in area (advancing area at same time). Expand escape sequences
 * etc. Returns the string, or NULL if it can't do it.
 */
char *tgetstr(const char *id, char **area) {
    char *cp = getcap(id);
    char *wsp, *ret;
    int c;

    if (cp == 0 || cp[2] != '=') return 0;

    cp += 3, ret = wsp = *area;
    while ((c = *cp++) && c != ':') {
	switch (c) {
	  default:  *wsp = c;		break;
	  case '^':
	    switch (c = *cp++) {
	      case '?': *wsp = '\177';	break;
	      default:  *wsp = c & 037;	break;
	    }				break;
	  case '\\':
	    switch (c = *cp++) {
	      default:  *wsp = c;	break;
	      case 'E':	*wsp = '\033';	break;
	      case 'a':	*wsp = '\a';	break;
	      case 'b':	*wsp = '\b';	break;
	      case 'f':	*wsp = '\f';	break;
	      case 'l':	*wsp = 0x0a;	break;
	      case 'n':	*wsp = '\n';	break;
	      case 'r':	*wsp = '\r';	break;
	      case 't':	*wsp = '\t';	break;
	      case '0': case '1': case '2': case '3':
	      case '4': case '5': case '6': case '7':
		*wsp = c - '0';
		while ((c = *cp) && '0' <= c && c <= '7')
		    *wsp = *wsp * 8 + *cp++ - '0';
		break;
	    }
	}
	wsp++;
    }
    *wsp++ = 0;
    *area = wsp;
    return ret;
}
