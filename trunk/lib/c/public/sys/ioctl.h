/*-
 * Copyright (c) 1982, 1986, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)ioctl.h     7.19 (Berkeley) 6/26/91
 */

#ifndef __IOCTL_H_INCLUDED
#define __IOCTL_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#ifndef	__SOCKIO_H_INCLUDED
#include <sys/sockio.h>
#endif

#include <_pack64.h>

struct winsize {
	unsigned short	ws_row;
	unsigned short	ws_col; 
	unsigned short	ws_xpixel;
	unsigned short	ws_ypixel; 
};

/*
 * Pun for SUN.
 */
struct ttysize {
    unsigned short      ts_lines;
    unsigned short      ts_cols;
    unsigned short      ts_xxx;
    unsigned short      ts_yyy;
};
#include <_packpop.h>

#define TIOCGSIZE       TIOCGWINSZ
#define TIOCSSIZE       TIOCSWINSZ

#define FREAD			0x0001			/* For TIOCFLUSH */			
#define FWRITE			0x0002

#define TIOCM_DTR       0x0001            /* data terminal ready */
#define TIOCM_RTS       0x0002            /* request to send */
#define TIOCM_CTS       0x1000            /* clear to send */
#define TIOCM_DSR       0x2000            /* data set ready */
#define TIOCM_RI        0x4000            /* ring */
#define TIOCM_RNG       TIOCM_RI
#define TIOCM_CD        0x8000            /* carrier detect */
#define TIOCM_CAR       TIOCM_CD
#define TIOCM_LE        0x0100            /* line enable */
#define TIOCM_ST        0x0200            /* secondary transmit */
#define TIOCM_SR        0x0400            /* secondary receive */

/*
 * Ioctl's have the command encoded in the lower word, and the size of
 * any in or out parameters in the upper word.  The high 2 bits of the
 * upper word are used to encode the in/out status of the parameter.
 */
#define IOCPARM_MASK    0x3fff          /* parameter length, at most 14 bits */
#define IOCPARM_LEN(x)  (((x) >> 16) & IOCPARM_MASK)
#define IOCBASECMD(x)   ((x) & ~(IOCPARM_MASK << 16))
#define IOCGROUP(x)     (((x) >> 8) & 0xff)

#define IOCPARM_MAX     NBPG            /* max size of ioctl, mult. of NBPG */
#define IOC_VOID        0x00000000      /* no parameters */
#define IOC_OUT         0x40000000      /* copy out parameters */
#define IOC_IN          0x80000000      /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
#define IOC_DIRMASK     0xc0000000      /* mask for IN/OUT/VOID */

#define _IOC(inout,group,num,len) \
        (inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define _IO(g,n)        _IOC(IOC_VOID,  (g), (n), 0)
#define _IOR(g,n,t)     _IOC(IOC_OUT,   (g), (n), sizeof(t))
#define _IOW(g,n,t)     _IOC(IOC_IN,    (g), (n), sizeof(t))
#define _IOWR(g,n,t)    _IOC(IOC_INOUT, (g), (n), sizeof(t))

/*** Not in NetBSD, In Sun -- modified to match Sun ***/
#define TCGETA          _IOR('T', 1, struct termio)
#define TCSETA          _IOW('T', 2, struct termio)
#define TCSETAW         _IOW('T', 3, struct termio)
#define TCSETAF         _IOW('T', 4, struct termio)
#define TCSBRK          _IOW('T',  5, int)
#define TCXONC          _IOW('T',  6, int)
#define TCFLSH          _IOW('T',  7, int)
#define TCGETS          _IOR('T', 13, struct termios)
#define TCSETS          _IOW('T', 14, struct termios)
#define TCSETSW         _IOW('T', 15, struct termios)
#define TCSETSF         _IOW('T', 16, struct termios)
/*** END ***/

#define TIOCHPCL        _IO('t',  2)                    /* hang up on last close */
#define TIOCGETP        _IOR('t', 8, struct sgttyb)     /* get parameters -- gtty */
#define TIOCSETP        _IOW('t', 9, struct sgttyb)     /* set parameters -- stty */
#define TIOCSETN        _IOW('t',10, struct sgttyb)     /* as above, but no flushtty*/

#define TIOCSINUSE      TIOCEXCL

#define TIOCEXCL        _IO('t', 13)                    /* set exclusive use of tty */
#define TIOCNXCL        _IO('t', 14)                    /* reset exclusive use of tty */
                                                        /* 15 unused */
#define TIOCFLUSH       _IOW('t', 16, int)              /* flush buffers */
#define TIOCSETC        _IOW('t', 17, struct tchars)    /* set special characters */
#define TIOCGETC        _IOR('t', 18, struct tchars)    /* get special characters */
/*** Not in Sun, In NetBSD ***/
#define TIOCGETA        _IOR('t', 19, struct termios)   /* get termios struct */
#define TIOCSETA        _IOW('t', 20, struct termios)   /* set termios struct */
#define TIOCSETAW       _IOW('t', 21, struct termios)   /* drain output, set */
#define TIOCSETAF       _IOW('t', 22, struct termios)   /* drn out, fls in, set */
/*      TIOCGETD        _IOR('t', 26, int)           */ /* get line discipline */
/*      TIOCSETD        _IOW('t', 27, int)           */ /* set line discipline */
#define TIOCDRAIN       _IO('t',  94)                   /* wait till output drained */
/*      TIOCSIG         _IO('t',  95)                */ /* pty: generate signal */
/*      TIOCEXT         _IOW('t', 96, int)           */ /* pty: external processing */
#define TIOCSCTTY       _IO('t',  97)                   /* become controlling tty */
/*** END ***/
/*      TIOCCONS        _IOW('t', 98, int)           */ /* become virtual console */
/*      TIOCUCNTL       _IOW('t', 102, int)          */ /* pty: set/clr usr cntl mode */
#define TIOCSWINSZ      _IOW('t', 103, struct winsize)  /* set window size */
#define TIOCGWINSZ      _IOR('t', 104, struct winsize)  /* get window size */
/*      TIOCREMOTE      _IOW('t', 105, int)          */ /* remote input editing */
#define TIOCMGET        _IOR('t', 106, int)             /* get all modem bits */
#define TIOCMBIC        _IOW('t', 107, int)             /* bic modem bits */
#define TIOCMBIS        _IOW('t', 108, int)             /* bis modem bits */
#define TIOCMSET        _IOW('t', 109, int)             /* set all modem bits */
#define TIOCSTART       _IO('t',  110)                  /* start output, like ^Q */
#define TIOCSTOP        _IO('t',  111)                  /* stop output, like ^S */
#define TIOCPKT         _IOW('t', 112, int)             /* pty: set/clear packet mode */
#define         TIOCPKT_DATA            0x00            /* data packet */
#define         TIOCPKT_FLUSHREAD       0x01            /* flush packet */
#define         TIOCPKT_FLUSHWRITE      0x02            /* flush packet */
#define         TIOCPKT_STOP            0x04            /* stop output */
#define         TIOCPKT_START           0x08            /* start output */
#define         TIOCPKT_NOSTOP          0x10            /* no more ^S, ^Q */
#define         TIOCPKT_DOSTOP          0x20            /* now do ^S ^Q */
#define         TIOCPKT_IOCTL           0x40            /* state change of pty driver */
#define TIOCNOTTY       _IO('t', 113)                   /* void tty association */
#define TIOCSTI         _IOW('t', 114, char)            /* simulate terminal input */
#define TIOCOUTQ        _IOR('t', 115, int)             /* output queue size */
#define TIOCGLTC        _IOR('t', 116, struct ltchars)  /* get local special chars*/
#define TIOCSLTC        _IOW('t', 117, struct ltchars)  /* set local special chars*/
#define TIOCSPGRP       _IOW('t', 118, int)             /* set pgrp of tty */
#define TIOCGPGRP       _IOR('t', 119, int)             /* get pgrp of tty */
#define TIOCCDTR        _IO('t', 120)                   /* clear data terminal ready */
#define TIOCSDTR        _IO('t', 121)                   /* set data terminal ready */
#define TIOCCBRK        _IO('t', 122)                   /* clear break bit */
#define TIOCSBRK        _IO('t', 123)                   /* set break bit */
#define TIOCLGET        _IOR('t', 124, int)             /* get local modes */
#define TIOCLSET        _IOW('t', 125, int)             /* set entire local mode word */

/*** Not in NetBSD, In Sun ***/
#define TIOCSETPGRP     _IOW('t', 130, int)             /* set pgrp of tty (posix) */
#define TIOCGETPGRP     _IOR('t', 131, int)             /* get pgrp of tty (posix) */
/*** END ***/

#define         UIOCCMD(n)      _IO('u', n)             /* usr cntl op "n" */

#define FIOCLEX         _IO('f', 1)                     /* set close on exec on fd */
#define FIONCLEX        _IO('f', 2)                     /* remove close on exec */
#define FIOGETOWN       _IOR('f', 123, int)           /* get owner */
#define FIOSETOWN       _IOW('f', 124, int)           /* set owner */
#define FIOASYNC        _IOW('f', 125, int)           /* set/clear async i/o */
#define FIONBIO         _IOW('f', 126, int)           /* set/clear non-blocking i/o */
#define FIONREAD        _IOR('f', 127, int)           /* get # bytes to read */

__BEGIN_DECLS

int     ioctl(int __fd, int __cmd, ...);

__END_DECLS

#endif

/* __SRCVERSION("ioctl.h $Rev: 173311 $"); */
