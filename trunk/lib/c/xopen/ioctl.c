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




#include <sys/ioctl.h>
#include <sys/sockio.h>

//#include <termios.h>
/*
 In turn includes <termios.h> if termios.h not
 already included (conflict in the baud rates)
 Since we don't use the baud rate in this file
 we should be ok ... =;-(
*/
//#include <sys/termio.h>		
/*
 In turn includes <sys/termio.h> which includes
 the files as stated above, resolving conflicts?
*/
#include <sgtty.h>

#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <devctl.h>				

#include <sys/iomsg.h>			/* For devctl messages */
#include <sys/dcmd_chr.h>		/* For char devctls */

#include <sys/dcmd_ip.h>		/* For ip devctls */
#include <net/if.h>				/* For orentry and friends */


#define SETNEWNUM(cmd, num)  (cmd = (((cmd) & ~(0xffff)) | ((num) & 0xffff)))

#define BAUD_SPEED_MASK 15

static void ltchars2termios(struct ltchars *ltchars, struct termios *termios);
static void termios2ltchars(struct termios *termios, struct ltchars *ltchars);
static void tchars2termios(struct tchars *tchars, struct termios *termios);
static void termios2tchars(struct termios *termios, struct tchars *tchars);
static void sgttyb2termios(struct sgttyb *sgttyb, struct termios *termios);
static void termios2sgttyb(struct termios *termios, struct sgttyb *sgttyb);
static void termio2termios(struct termio *termio, struct termios *termios);
static void termios2termio(struct termios *termios, struct termio *termio);
static void modem2serctl(int *modem, int *serctl);
#ifdef NOT_USED
static void linestatus2modem(int *linestatus, int *modem);
#endif

static __inline int __fcntl2ioctl(int status)
{
	if (status == -1 && errno == ENOSYS) {
		errno = ENOTTY;
	}
	return(status);
}

int ioctl(int fd, int cmd, ...) {
    va_list vl;
	void *data;
	struct termios	ltermios;
#ifdef NOT_USED
	struct termios	*ptermios;
	struct termio	ltermio;
#endif
	struct termio	*ptermio;
	int				match, tempint;

	//Pull out the command and the data
	va_start(vl, cmd);
	data = va_arg(vl, void *);
	va_end(vl);

	/* 
     These calls have significantly different inputs or outputs
     that we have to manipulate (or re-direct entirely)
	*/
	switch ((unsigned)cmd) {	
	/**** These calls map to fcntl's ****/

	//Set/Clear FD_CLOEXEC state
	case FIOCLEX:	
		return(fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC));
	case FIONCLEX:
		return(fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) & ~FD_CLOEXEC));

	//These are undoced uses for fcntl
	case SIOCGPGRP:
	case FIOGETOWN:
		if ( (tempint = fcntl (fd, F_GETOWN)) != -1)
			*(int *)data = tempint;
		return(tempint);
	case SIOCSPGRP:
	case FIOSETOWN:
		return(fcntl(fd, F_SETOWN, *(int *)data));

	//Set the non-blocking state for reads on and off
	case FIONBIO:
		if (*(int *)data)
			return(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK));
		else
			return(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK));

	//Set the async state on and off
	case FIOASYNC:
		if (*(int *)data)
			return(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_ASYNC));
		else
			return(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_ASYNC));

	//Set/Clear exclusive mode
	case TIOCEXCL: {
		struct flock lflock;
		memset(&lflock, 0, sizeof(lflock));
		lflock.l_type = F_WRLCK;			//Fails if file opened read only?
		lflock.l_whence = SEEK_SET;
		lflock.l_start = 0;
		lflock.l_len = 0;
		lflock.l_pid = getpid();
		return(__fcntl2ioctl(fcntl(fd, F_SETLK, &lflock)));
	}
	case TIOCNXCL: {
		struct flock lflock;
		memset(&lflock, 0, sizeof(lflock));
		lflock.l_type = F_UNLCK;
		lflock.l_whence = SEEK_SET;
		lflock.l_start = 0;
		lflock.l_len = 0;
		lflock.l_pid = getpid();
		(void)fcntl(fd, F_GETLK, &lflock);
		return(__fcntl2ioctl(fcntl(fd, F_SETLK, &lflock)));
	}

	//Make the terminal the controlling terminal for the process
	case TIOCSCTTY:
		return(tcsetsid(fd, getpid()));
	//Dis-associate this terminal as the controlling terminal
	case TIOCNOTTY:
		return(tcsetsid(-1, getpid()));

	/**** These calls translate from one type to another using tc[s|g]etattr ****/

	//Terminal properties w/ termio
	case TCGETA: {
		ptermio = (struct termio *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		termios2termio(&ltermios, ptermio);
		return(EOK);
	}
	case TCSETA: {
		ptermio = (struct termio *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);		
		termio2termios(ptermio, &ltermios);
		return(tcsetattr(fd, TCSANOW, &ltermios));
	}
	case TCSETAW: {
		ptermio = (struct termio *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);		
		termio2termios(ptermio, &ltermios);
		return(tcsetattr(fd, TCSADRAIN, &ltermios));
	}
	case TCSETAF: {
		ptermio = (struct termio *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);		
		termio2termios(ptermio, &ltermios);
		return(tcsetattr(fd, TCSAFLUSH, &ltermios));
	}

	//Set terminal state in an sgttyb structure (incomplete)
	case TIOCGETP: {
		struct sgttyb *psgttyb = (struct sgttyb *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		termios2sgttyb(&ltermios, psgttyb);
		return(EOK);
	}	
	case TIOCSETP: {
		struct sgttyb *psgttyb = (struct sgttyb *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		sgttyb2termios(psgttyb, &ltermios);
		return(tcsetattr(fd, TCSAFLUSH, &ltermios));
	}
	case TIOCSETN: {
		struct sgttyb *psgttyb = (struct sgttyb *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		sgttyb2termios(psgttyb, &ltermios);
		return(tcsetattr(fd, TCSANOW, &ltermios));
	}

	//Terminal state in tchars structure
	case TIOCGETC: {
		struct tchars *ptchars = (struct tchars *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		termios2tchars(&ltermios, ptchars);
		return(EOK);
	}
	case TIOCSETC: {
		struct tchars *ptchars = (struct tchars *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		tchars2termios(ptchars, &ltermios);
		return(tcsetattr(fd, TCSANOW, &ltermios));
	}
	
	//Terminal state in an ltchars structure
	case TIOCGLTC: {
		struct ltchars *pltchars = (struct ltchars *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		termios2ltchars(&ltermios, pltchars);
		return(EOK);
	}
	case TIOCSLTC: {
		struct ltchars *pltchars = (struct ltchars *)data;
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		ltchars2termios(pltchars, &ltermios);
		return(tcsetattr(fd, TCSANOW, &ltermios));
	}

	//Set/Get the local flags structure
	case TIOCLGET: {
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		*((int *)data) = (int)ltermios.c_lflag;
		return(EOK);
	}
	case TIOCLSET: {
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		ltermios.c_lflag &= ~0xffff;
		ltermios.c_lflag |= *((int*)data);
		return(tcsetattr(fd, TCSANOW, &ltermios));
	}

	//Clear the break flag (implement TIOCSBRK?)
	case TIOCCBRK: {
		if (tcgetattr(fd, &ltermios) == -1)
			return(-1);
		ltermios.c_iflag &= ~(BRKINT);
		ltermios.c_iflag |= IGNBRK;
		return(tcsetattr(fd, TCSANOW, &ltermios));
	}

	//Set the HUPCL flag 
	case TIOCHPCL: {
		if (tcgetattr(fd, &ltermios) != -1) {
			ltermios.c_cflag |= HUPCL;
			return(tcsetattr(fd, TCSANOW, &ltermios));
		}
	}

	/**** These calls map to devctl's, but need serious data munging ****/

	//Inject a character into the stream
	case TIOCSTI: {	
		return(tcinject(fd, (char*)data, sizeof(char)));
	}

	//Send a break for a period of time
	case TCSBRK: {
		int *duration = (int *) data;				//Duration is measured in ms 
		return(tcsendbreak(fd, *duration));
		//tempint = (((*duration) ? *duration : 300) << 16) | _SERCTL_BRK_CHG | _SERCTL_BRK;
		//return(devctl(fd, DCMD_CHR_SERCTL, &tempint, sizeof(tempint), &ret));
	}

	/**** Modem control operations (via devctl) ****/
	//Clear the state of the modem by ~AND'ing in the int argument
	case TIOCMBIC: {
		int tmpmodem;
		if (_devctl(fd, DCMD_CHR_LINESTATUS, &tmpmodem, sizeof(tmpmodem), _DEVCTL_FLAG_NOTTY) == -1) 
			return(-1);
		tmpmodem &= ~(*((int*)data));
		tempint = 0;
		modem2serctl(&tmpmodem, &tempint);
		return(_devctl(fd, DCMD_CHR_SERCTL, &tempint, sizeof(tempint), _DEVCTL_FLAG_NOTTY)); 
	}
	//Set the state of the modem by OR'ing in the int argument
	case TIOCMBIS: {
		int tmpmodem;
		if (_devctl(fd, DCMD_CHR_LINESTATUS, &tmpmodem, sizeof(tmpmodem), _DEVCTL_FLAG_NOTTY) == -1) 
			return(-1);
		tmpmodem |= *((int*)data);		
		tempint = 0;
		modem2serctl(&tmpmodem, &tempint);
		return(_devctl(fd, DCMD_CHR_SERCTL, &tempint, sizeof(tempint), _DEVCTL_FLAG_NOTTY)); 
	}

	//Set the state of the modem lines
	case TIOCMSET: {
		tempint = 0;
		modem2serctl((int*)data, &tempint);
		return(_devctl(fd, DCMD_CHR_SERCTL, &tempint, sizeof(tempint), _DEVCTL_FLAG_NOTTY)); 
	}

	//Set/Clear DTR lines
	case TIOCCDTR: {
		int status; 
		if (_devctl(fd, DCMD_CHR_LINESTATUS, &status, sizeof(status), _DEVCTL_FLAG_NOTTY) == -1)
			return(-1);
		if (status & _LINESTATUS_SER_DTR) {
			status =  _SERCTL_DTR_CHG;
			return(_devctl(fd, DCMD_CHR_SERCTL, &status, sizeof(status), _DEVCTL_FLAG_NOTTY));
		}
		return(EOK);
	}
	case TIOCSDTR: {
		int status; 
		if (_devctl(fd, DCMD_CHR_LINESTATUS, &status, sizeof(status), _DEVCTL_FLAG_NOTTY) == -1)
			return(-1);
		if (!(status & _LINESTATUS_SER_DTR)) {
			status =  _SERCTL_DTR | _SERCTL_DTR_CHG;
			return(_devctl(fd, DCMD_CHR_SERCTL, &status, sizeof(status), _DEVCTL_FLAG_NOTTY));
		}
		return(EOK);
	}
	default:
		break;
	}

/*
 * The following block can go away when 6.4 is released
 * and <sys/sockio.h> from the networking project is
 * generally available.
 */
#ifndef NOSIOCGIFCONF
/* Define these as in the latest <sys/sockio.h> */
#define NOSIOCGIFCONF _IOWR('i', 36, struct ifconf)
#undef SIOCGIFCONF
#define SIOCGIFCONF   _IOWR('i', 38, struct ifconf)
#endif
	/* 
	 Generic handling for all but SIOCGIFCONF networking IOCTL's
	*/
	if ((unsigned)cmd == SIOCGIFCONF || (unsigned)cmd == NOSIOCGIFCONF) {
		io_devctl_t		msg;
		iov_t			wiov[2], riov[3];
		struct ifconf *ifconfp = (struct ifconf *)data;

		msg.i.type = _IO_DEVCTL;
		msg.i.combine_len = sizeof(msg.i);
		msg.i.dcmd = cmd;
		msg.i.zero = 0;
		msg.i.nbytes = ifconfp->ifc_len;

		SETIOV (wiov + 0, &msg, sizeof (msg.i));
		SETIOV (wiov + 1, data, IOCPARM_LEN((unsigned)cmd));
		SETIOV (riov + 0, &msg, sizeof (msg.o));
		SETIOV (riov + 1, ifconfp, sizeof (ifconfp->ifc_len));
		SETIOV (riov + 2, ifconfp->ifc_buf, ifconfp->ifc_len);
		return MsgSendv(fd, wiov, 2, riov, 3);
	}

	/* 
	 These calls require their command types to be translated
	*/
	switch ((unsigned)cmd) {
	case TCGETS:								//Not on NetBSD 
		SETNEWNUM(cmd, TIOCGETA);  match = 1;
		break;

	case TCSETS:								//Not on NetBSD
		SETNEWNUM(cmd, TIOCSETA);  match = 1; 
		break;

	case TCSETSW:								//Not on NetBSD
		SETNEWNUM(cmd, TIOCSETAW);  match = 1;
		break;

	case TCSETSF:								//Not on NetBSD
		SETNEWNUM(cmd, TIOCSETAF);  match = 1;
		break;

	case TIOCSETPGRP:							//Not on Sun
		SETNEWNUM(cmd, TIOCSPGRP);  match = 1;
		break;

	case TIOCGETPGRP:							//Not on Sun
		SETNEWNUM(cmd, TIOCGPGRP);  match = 1; 
		break;

	case TIOCSTOP: 
	case TIOCSTART:							
		//These functions pass in void but we need to pass an int so ...
		data = &tempint;
		tempint = (cmd == TIOCSTOP) ? TCOOFF : TIOCSTART;
		cmd = _IOW(_DCMD_CHR,TCXONC,int);		//Create new command  
	case TCXONC:								//Not on NetBSD
		//Assume incoming data already looks like:
		//data = 0= suspend output = TCOOFF, 1= restart output = TCOON
		//       2= suspend input = TCIOFF,  3= restart input = TCION
		SETNEWNUM(cmd, TCXONC);  match = 1;
		break;

	case TIOCFLUSH: 
		//Need to re-map 0 -> 2, FREAD -> 0, FWRITE -> 1
		switch (*((int*)data)) {
			case 0:      *((int*)data) = TCIOFLUSH; break;
			case FREAD:  *((int*)data) = TCIFLUSH; break;
			case FWRITE: *((int*)data) = TCOFLUSH; break;
			default: break;
		}
		/* Fall Through */
	case TCFLSH:								//Not on NetBSD 
		//Assume input data looks like:
		//data = 0 = flush in, 1 = flush output, 2 = flush both
		SETNEWNUM(cmd, TIOCFLUSH); match = 1;
		break;

	default:
		break;
	}

	//If you got this far then out you go as a generic devctl
	return(_devctl(fd, cmd, data, IOCPARM_LEN((unsigned)cmd), _DEVCTL_FLAG_NOTTY));
	/*
	 Returns different things for different commands:
	 - GETPRGRP returns pid_t 
	 - FIOREAD returns number of chars in input queue
	 - TIOCOUTQ returns number of chars in output queue
	 - 0 for all other cases
	*/
}

#if 0 /* BEGIN BLOCK COMMENT */
 All of the translation utilities rely on an
 incestuous sharing of appropriate flags and values.

 Termio structure looks like <sys/termio.h>:
 #define NCC			16
 struct termio {
     unsigned short     c_iflag;        /* input modes */
     unsigned short     c_oflag;        /* output modes */
     unsigned short     c_cflag;        /* control modes */
     unsigned short     c_lflag;        /* line discipline modes */
     char               c_line;         /* line discipline */
     char               c_padding;
     unsigned short     c_cc[NCC];      /* control chars */
 };

 Termios structure looks like <termios.h>:
 #define NCCS    40

 typedef unsigned char   cc_t;
 typedef long            speed_t;
 typedef unsigned long   tcflag_t;
 struct termios {
    tcflag_t        c_iflag;    /* Input Modes */
    tcflag_t        c_oflag;    /* Ouput modes */
    tcflag_t        c_cflag;    /* Control Modes */
    tcflag_t        c_lflag;    /* Local Modes */
    cc_t            c_cc[NCCS]; /* Control Characters */
    unsigned short  zero[3];
    unsigned short  c_status;   /* device status */
    unsigned short  c_qflag;    /* QNX Specific flags */
    unsigned short  handle;
    speed_t         c_ispeed;   /* Input Baud rate */
    speed_t         c_ospeed;   /* Output baud rate */
 };

 Getty type structures <sgtty.h>
 /*
  * TIOCGETP/TIOCSETP structure
  */
 struct sgttyb {
    char sg_ispeed; /* input speed */
    char sg_ospeed; /* output speed */
    char sg_erase;  /* erase character */
    char sg_kill;   /* kill character */
    int  sg_flags;  /* mode flags */
 };

 /*
  * TIOCGETC/TIOCSETC structure
  */
 struct tchars {
    char t_intrc;   /* interrupt */
    char sg_ospeed; /* output speed */
    char sg_erase;  /* erase character */
    char sg_kill;   /* kill character */
    int  sg_flags;  /* mode flags */
 };
 /*
  * TIOCGETC/TIOCSETC structure
  */
 struct tchars {
    char t_intrc;   /* interrupt */
    char t_quitc;   /* quit */
    char t_startc;  /* start output */
    char t_stopc;   /* stop output */
    char t_eofc;    /* end-of-file */
    char t_brkc;    /* input delimiter (like nl) */
 };

 /*
  * TIOCGLTC/TIOCSLTC structure
  */
 struct ltchars {
    char    t_suspc;    /* stop process signal */
    char    t_dsuspc;   /* delayed stop process signal */
    char    t_rprntc;   /* reprint line */
    char    t_flushc;   /* flush output (toggles) */
    char    t_werasc;   /* word erase */
    char    t_lnextc;   /* literal next character */
 };

 Modem status control variables

TIOCM_LE   Line Enable.			
TIOCM_DTR  Data Terminal Ready.	
TIOCM_RTS  Request To Send.	
TIOCM_ST   Secondary Transmit.	
TIOCM_SR   Secondary Receive.
TIOCM_CTS  Clear To Send.	
TIOCM_CAR  Carrier Detect.
TIOCM_CD   Carier Detect (synonym).
TIOCM_RNG  Ring Indication.
TIOCM_RI   Ring Indication (synonym).
TIOCM_DSR  Data Set Ready.

#endif /* END BLOCK COMMENT */

static const speed_t baud_speeds[] = {	0,		50,		75,		110,
										134,	150,	200,	300,
										600,	1200,	1800,	2400,
										4800,	9600,	19200,	38400};

#define NUM_SPEEDS (sizeof(baud_speeds)/sizeof(baud_speeds[0]))

//translate encoded speed value into baud speed. (termio -> termios)
static speed_t get_bauds(unsigned int enc){
	if(enc >= NUM_SPEEDS)
		return baud_speeds[ NUM_SPEEDS - 1 ];
	return baud_speeds[ enc ];
}

//translate baud speed to encoded speed value. (termio <- termios)
static int get_enc_speed(speed_t bspeed){
	int i;

	for(i=0;i<NUM_SPEEDS;i++)
			if(bspeed == baud_speeds[i])
				return i;
	return (NUM_SPEEDS - 1);
}

// Translate a "termio" structure into a "termios".
static void termio2termios(struct termio * termio,
					struct termios * termios) {
#define SET_LOW_BITS(x,y)       (*(unsigned short *)(&x) = (y))
        SET_LOW_BITS(termios->c_iflag, termio->c_iflag);
        SET_LOW_BITS(termios->c_oflag, termio->c_oflag);
        SET_LOW_BITS(termios->c_cflag, termio->c_cflag);
        SET_LOW_BITS(termios->c_lflag, termio->c_lflag);
#undef SET_LOW_BITS
        memcpy(termios->c_cc, termio->c_cc, NCC);
		termios->c_ispeed = get_bauds( termio->c_cflag & BAUD_SPEED_MASK );
		termios->c_ospeed = termios->c_ispeed;

		termios->c_cflag &= ~BAUD_SPEED_MASK;
}

// Translate a "termios" structure into a "termio".
static void termios2termio(struct termios * termios,
					struct termio * termio) {
        termio->c_iflag = termios->c_iflag;
        termio->c_oflag = termios->c_oflag;
        termio->c_cflag = termios->c_cflag;
        termio->c_lflag = termios->c_lflag;
        //termio->c_line  = termios->c_line;
		//Linux Line Disciplines are:
		//N_TTY 0, N_SLIP 1, N_MOUSE 2, N_PPP 3, N_STRIP 4
        memcpy(termio->c_cc, termios->c_cc, NCC);
		termio->c_cflag |= get_enc_speed(termios->c_ispeed);
}

/*
 We really need a definitive reference on how to map 
 these two structures back and forth.  Linux doesn't
 really even bother, 
*/
static void termios2sgttyb(struct termios *termios,
					struct sgttyb *sgttyb) {
	/* According to Sun manpages:
	 sg_ispeed and sg_ospeed fields describe  the  input  and
	 output  speeds  of the device, and reflect the values in the
	 c_cflag field of the termios structure.
	*/
	sgttyb->sg_ospeed = get_enc_speed( termios->c_ospeed);
	sgttyb->sg_ispeed = get_enc_speed( termios->c_ispeed);
/*
	if (termios->c_cflag & CBAUDEXT)
		sgttyb->sg_ospeed = (termios->c_cflag & CBAUD) + CBAUD + 1;
	else
		sgttyb->sg_ospeed = termios->c_cflag & CBAUD;

	if (termios_p->c_cflag & CIBAUDEXT)
		sgttyb->sg_ispeed = ((termios->c_cflag & CIBAUD) >> IBSHIFT)
							 + (CIBAUD >> IBSHIFT) + 1;
	else
		sgttyb->sg_ispeed = (termios->c_cflag & CIBAUD) >> IBSHIFT;
*/

	sgttyb->sg_erase  = termios->c_cc[VERASE];
	sgttyb->sg_kill  = termios->c_cc[VKILL];
	sgttyb->sg_flags = 0;

	if ((termios->c_iflag & IXOFF)) 
		sgttyb->sg_flags |= TANDEM;

	if ((termios->c_lflag & ICANON)) 
		sgttyb->sg_flags |= CBREAK;

	if ((termios->c_iflag & IUCLC) && (termios->c_oflag & OLCUC))
		sgttyb->sg_flags |= LCASE;

	if ((termios->c_lflag & ECHO)) 
		sgttyb->sg_flags |= ECHO;

	if (!(termios->c_oflag & OPOST)) 
		sgttyb->sg_flags |= RAW;

	if ((termios->c_iflag & ICRNL) && (termios->c_oflag & ONLCR)) 
		sgttyb->sg_flags |= CRMOD;

	if ((termios->c_cflag & PARENB)) 
		sgttyb->sg_flags |= (termios->c_cflag & PARODD) ? ODDP : EVENP;
	else 
		sgttyb->sg_flags |= ANYP;

	//Manage delay values ...
}

static void sgttyb2termios(struct sgttyb *sgttyb, 
                    struct termios *termios) {
	termios->c_ispeed = get_bauds( sgttyb->sg_ispeed );
	termios->c_ospeed = get_bauds( sgttyb->sg_ospeed );
/*
	if (sgttyb->sg_ospeed > CBAUD) {
		termios->c_cflag |= CBAUDEXT;
		sgttyb->sg_ospeed -= (CBAUD + 1);
	} else
		termios->c_cflag &= ~CBAUDEXT;
	termios->c_cflag = (termios->c_cflag & ~CBAUD) | (sgttyb->sg_ospeed & CBAUD);


	if (sgttyb->sg_ispeed == 0) {
		sgttyb->sg_ispeed = termios->c_cflag & CBAUD;
		if (termios->c_cflag & CBAUDEXT)
			sgttyb->sg_ispeed += (CBAUD + 1);
	}
	if ((sgttyb->sg_ispeed << IBSHIFT) > CIBAUD) {
		termios->c_cflag |= CIBAUDEXT;
		sgttyb->sg_ispeed -= ((CIBAUD >> IBSHIFT) + 1);
	} else
		termios->c_cflag &= ~CIBAUDEXT;
	termios->c_cflag =
		(termios->c_cflag & ~CIBAUD) | ((sgttyb->sg_ispeed << IBSHIFT) & CIBAUD);
*/
	termios->c_cc[VERASE] = sgttyb->sg_erase;
	termios->c_cc[VKILL] = sgttyb->sg_kill;

	if ((sgttyb->sg_flags & TANDEM))
		termios->c_iflag |= IXOFF;
	else
		termios->c_iflag &= ~IXOFF;

	if ((sgttyb->sg_flags & CBREAK))
		termios->c_lflag |= ICANON; 
	else 
		termios->c_lflag &= ~ICANON; 

	if ((sgttyb->sg_flags & LCASE)) {
		termios->c_iflag |= IUCLC;
		termios->c_oflag |= OLCUC;
	}
	else {
		termios->c_iflag &= ~IUCLC;
		termios->c_oflag &= ~OLCUC;
	}

	if ((sgttyb->sg_flags & ECHO))
		termios->c_lflag |= ECHO;
	else
		termios->c_lflag &= ~ECHO;

	if ((sgttyb->sg_flags & RAW))
		termios->c_oflag |= OPOST;
	else
		termios->c_oflag &= ~OPOST;

	if ((sgttyb->sg_flags & CRMOD)) {
		termios->c_iflag |= ICRNL;
		termios->c_oflag |= ONLCR;
	}
	else {
		termios->c_iflag &= ~ICRNL;
		termios->c_oflag &= ~ONLCR;
	}

	if (sgttyb->sg_flags & ODDP) { 
		termios->c_cflag |= PARENB | PARODD;
	}
	else if (sgttyb->sg_flags & EVENP) { 
		termios->c_cflag &= ~PARODD; 
		termios->c_cflag |= PARENB; 
	}
	else {
		termios->c_cflag &= ~PARENB;
	}
	
	//Manage delay values ...
}

static void termios2tchars(struct termios *termios, 
                    struct tchars *tchars) {
	tchars->t_intrc = termios->c_cc[VINTR];
	tchars->t_quitc = termios->c_cc[VQUIT];
	tchars->t_startc = termios->c_cc[VSTART];
	tchars->t_stopc = termios->c_cc[VSTOP];
	tchars->t_eofc = termios->c_cc[VEOF];
	tchars->t_brkc = termios->c_cc[VEOL];
	//Local flag mapping, no variable
}

static void tchars2termios(struct tchars *tchars, 
                    struct termios *termios)  {
	termios->c_cc[VINTR] =	tchars->t_intrc;
	termios->c_cc[VQUIT] =	tchars->t_quitc;
	termios->c_cc[VSTART] = tchars->t_startc;
	termios->c_cc[VSTOP] = tchars->t_stopc;
	termios->c_cc[VEOF] = tchars->t_eofc;
	termios->c_cc[VEOL] = tchars->t_brkc;
	//Local flag mapping, no variable
}

static void termios2ltchars(struct termios *termios, 
                    struct ltchars *ltchars) {
	ltchars->t_suspc = termios->c_cc[VSUSP];
	ltchars->t_dsuspc = termios->c_cc[VDSUSP];
	ltchars->t_rprntc = termios->c_cc[VREPRINT];
	ltchars->t_flushc = termios->c_cc[VDISCARD];
	ltchars->t_werasc = termios->c_cc[VWERASE];
	ltchars->t_lnextc = termios->c_cc[VLNEXT];
}

static void ltchars2termios(struct ltchars *ltchars, 
                    struct termios *termios)  {
	termios->c_cc[VSUSP] =	ltchars->t_suspc;
	termios->c_cc[VDSUSP] =	ltchars->t_dsuspc;
	termios->c_cc[VREPRINT] = ltchars->t_rprntc;
	termios->c_cc[VDISCARD] = ltchars->t_flushc;
	termios->c_cc[VWERASE] = ltchars->t_werasc;
	termios->c_cc[VLNEXT] = ltchars->t_lnextc;
}

/*
 These functions are not in any way complete,
 but are provided to assist w/ backward compatablilty
 We map TIOCM_* to _SERCTL_* and LINESTATUS_*
*/
#ifdef NOT_USED
static void linestatus2modem(int *linestatus, int *modem) {
	*modem = *linestatus;
}
#endif

static void modem2serctl(int *modem, int *serctl) {
	int newvalues;
	newvalues = (*modem & (TIOCM_DTR | TIOCM_RTS)) | _SERCTL_DTR_CHG | _SERCTL_RTS_CHG;
	*serctl &= ~(_SERCTL_DTR | _SERCTL_DTR_CHG | _SERCTL_RTS | _SERCTL_RTS_CHG);
	*serctl |= newvalues;
}


__SRCVERSION("ioctl.c $Rev: 170700 $");
