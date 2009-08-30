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
 *  sys/termio.h
 *

 */
#ifndef __TERMIO_H_INCLUDED
#define __TERMIO_H_INCLUDED

#ifndef _TERMIOS_H_INCLUDED
#include <termios.h>

#define TANDEM      0x00000001          /* send stopc on out q full */
#define CBREAK      0x00000002          /* half-cooked mode */
#define LCASE       0x00000004          /* simulate lower case */
#define CRMOD       0x00000010          /* map \r to \r\n on output ONLCR & ICRNL */
#define RAW         0x00000020          /* no i/o processing */
#define ODDP        0x00000040          /* get/send odd parity */
#define EVENP       0x00000080          /* get/send even parity */
#define ANYP        (ODDP|EVENP)        /* get any parity/send none */

#define NLDELAY     0x00000100          /* from termios.h NLDLY - \n delay */
#define CRDELAY     0x00000600          /* from termios.h CRDLY - \r delay */
#define TABDELAY    0x00001800          /* from termios.h TABDLY - horizontal tab delay */
#define XTABS           0x1800          /* from termios.h TAB3 - expand tabs on output */
#define BSDELAY     0x00002000          /* from termios.h BSDLY - \b delay */
#define VTDELAY     0x00004000          /* from termios.h VTDLY - vertical tab delay */
#define ALLDELAY    (NLDELAY|TABDELAY|CRDELAY|VTDELAY|BSDELAY)

#undef  B0
#define B0               0
#undef  B50
#define _TIO_B50          1
#define B50              _TIO_B50 
#undef  B75
#define _TIO_B75          2
#define B75              _TIO_B75 
#undef  B110
#define _TIO_B110         3
#define B110             _TIO_B110
#undef  B134
#define _TIO_B134         4
#define B134             _TIO_B134
#undef  B150
#define _TIO_B150         5
#define B150             _TIO_B150
#undef  B200
#define _TIO_B200         6
#define B200             _TIO_B200 
#undef  B300
#define _TIO_B300         7
#define B300             _TIO_B300 
#undef  B600
#define _TIO_B600         8
#define B600             _TIO_B600 
#undef  B1200
#define _TIO_B1200        9
#define B1200            _TIO_B1200 
#undef  B1800
#define _TIO_B1800       10
#define B1800           _TIO_B1800
#undef  B2400
#define _TIO_B2400       11
#define B2400           _TIO_B2400
#undef  B4800
#define _TIO_B4800       12
#define B4800           _TIO_B4800
#undef  B9600
#define _TIO_B9600       13
#define B9600           _TIO_B9600
#undef  B19200
#define _TIO_B19200      14
#define B19200          _TIO_B19200
#undef  B38400
#define _TIO_B38400      15
#define B38400          _TIO_B38400


#undef  EXTA
#define _TIO_EXTA        14
#define EXTA            _TIO_EXTA
#undef  EXTB
#define _TIO_EXTB        15
#define EXTB            _TIO_EXTB

#undef  CBAUD
#define _TIO_CBAUD       15              /* baud rate mask */
#define CBAUD           _TIO_CBAUD       /* baud rate mask */

/*
 * Control characters
 */
#undef  NCC
#define NCC             18               /* number of control characters */

#else
#error termio/termios incompatibility
#endif

#ifndef __IOCTL_H_INCLUDED
#include <sys/ioctl.h>
#endif

/*
 * TCGETA/TCSETA structure
 */

#include <_pack64.h>

struct termio {
     unsigned short     c_iflag;        /* input modes */
     unsigned short     c_oflag;        /* output modes */
     unsigned short     c_cflag;        /* control modes */
     unsigned short     c_lflag;        /* line discipline modes */
     char               c_line;         /* line discipline */
     char               c_padding;
     unsigned short     c_cc[NCC];      /* control chars */
};

#include <_packpop.h>

#endif

/* __SRCVERSION("termio.h $Rev: 153052 $"); */
