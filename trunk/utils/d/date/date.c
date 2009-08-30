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




#ifndef __QNXNTO__
#define CLOCK_SUPPORTED 
#endif

#ifdef __USAGE
%-date
%C - display or set the date and time (POSIX)

%C	[-n node] [-u] [-s seconds] [+<format>]
	[-n node] [-t]
	[-u] <date>
Options:
   -n node      Read current time from the specified node. (Default: local node)
   -s seconds   Display the string equivalent of this date, supplied as
                seconds since the start of the epoch. This value is used
                instead of the system time value for the number of seconds. 
   -t           Display the time as a simple long integer.
   -u           Perform operations using Coordinated Universal Time (UTC)
                instead of local time.
Where:
   +<format> is a format string containing text and field descriptors
   which are in the form %<char>. e.g. "+%I O'Clock %p"

   If any operands other than a +format are specified, date will asssume
   the operands are a <date> that you want to set the QNX date and time
   to. Only root will be allowed to change the date.

   The following formats may be used for a <date> when setting the date/time:

	[[[[CC]YY]MM]DD]hhmm[.SS]                        (touch utility format)
	MMDDhhmm[YY]                                     (System V format (*) )
	day [month [year [hour [min [sec]]]]]] [am|pm]   (QNX 2.15 format)

   (*) Years >99 (1999) cannot be specified using the Sys V format (where
       year follows the date/time), because of a parsing conflict with
       the YYMMDDhhmm form of the POSIX touch utility format. If the last
       pair of digits is <59 it may refer to minutes, not years.

   Date will set only the software QNX clock. To transfer the software
   time to the battery backed up hardware clock and vice-versa, use the
   rtc utility.

   See the printed manual for more detail on <date> and +<format> strings.
%-clock
%C	- display date and time on console (QNX)

%C	[-u] [-s] [-f color] [-b color] [+<format>]
Options:
 -f color    Set foreground color.
 -b color    Set background color.
 -s          Sleep 1 second between display updates.
 -u          Use Coordinated Universal Time (UTC).
Where:
   Foreground colors are:
      black, blue , green , red , cyan , magenta , brown , white
      dgray, bblue, bgreen, bred, bcyan, bmagenta, yellow, bwhite

   Background colors are:
      black, blue, green, red, cyan, magenta, brown, white

   The default foreground is white. The default background is black.

   +<format> is a format string containing text and field descriptors
   which are in the form %<char>. e.g. "+%I O'Clock %p"
   See the manual for more detail on the format string.
Caveat:
   Some machines have problems doing DMA (e.g. floppy I/O) while
   video memory is being accessed. If you are running into problems
   with this the -s option may be used to make video updates less
   frequent. When -s is not used, clock will update the screen once
   per 'tick' (typically 100 times per second). Be aware that other
   video updates may cause a problem (windows, console writes) and
   that you are taking a chance with your data if you use such
   hardware.
#endif

#ifdef __USAGENTO
%C - display or set the date and time (POSIX)

%C	[-uv] [-s seconds] [+<format>]
	[-uv] [-S seconds] <date>
	[-t]
Options:
   -S seconds   Attempt to initiate a slow clock adjustment if this can
                be accomplished within the specified number of seconds
                (maximum) without exceeding double the clock speed or
                going below half the normal clock speed for the duration
                of the adjustment.
   -s seconds   Display the string equivalent of this date, supplied as
                seconds since the start of the epoch. This value is used
                instead of the system time value for the number of seconds. 
   -t           Display the time as a simple long integer.
   -u           Perform operations using Coordinated Universal Time (UTC)
                instead of local time.
   -v           Be verbose. If multiple -v options are specified along 
                with -S, date will not terminate until the slow adjustment
                period has elapsed.

Where:
   +<format> is a format string containing text and field descriptors
   which are in the form %<char>. e.g. "+%I O'Clock %p"

   If any operands other than a +format are specified, date will asssume
   the operands are a <date> that you want to set the Neutrino date and time
   to. Only root will be allowed to change the date.

   The following formats may be used for a <date> when setting the date/time:

	[[[[CC]YY]MM]DD]hhmm[.SS]                        (touch utility format)
	MMDDhhmm[YY]                                     (System V format (*) )
	day [month [year [hour [min [sec]]]]]] [am|pm]   (QNX 2.15 format)

   (*) Years >99 (1999) cannot be specified using the Sys V format (where
       year follows the date/time), because of a parsing conflict with
       the YYMMDDhhmm form of the POSIX touch utility format. If the last
       pair of digits is <59 it may refer to minutes, not years.

   Date will set only the software QNX clock. To transfer the software
   time to the battery backed up hardware clock and vice-versa, use the
   rtc utility.

   See the utility documentation for more detail on <date> and +<format>
   strings.
#endif

/* this code is clock and date rolled into one */
/* clock should be a link to date */
#include <lib/compat.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <util/fish_time.h>
#include <util/stdutil.h>

#ifdef __QNXNTO__
#include <sys/neutrino.h>
#include <utmp.h>
#elif 0
#include <sys/timers.h>
#include <sys/sched.h>
#include <sys/osinfo.h>
#include <sys/kernel.h>
#include <sys/vc.h>
#endif

#include <sys/types.h>
#include <util/defns.h>

#define DATE (1)
#define CLOCK (0)

/* note - do not chnage these willy-nilly - look at how and
   where they are used in the code, there may be need for changes.
   In particular, you cannot merely change the BASE_CENTURY to 2000.
*/
#define BEGIN_EPOCH  (1970)

// mktime() tm_year field is in years since 1900.
#define MKTIME_BASE_CENTURY (1900L) 

struct attribute {
	char name[3];
	unsigned char value;
} ;
	
/*
---------------------------------------------------------- prototypes -----
*/

#ifdef CLOCK_SUPPORTED
void display_clock(char *buf);
#endif

void abarf(char *str);
int ctod(char *s);
int comp(char *s1, char *s2);
#ifdef CLOCK_SUPPORTED
unsigned parse_attribute(char *string);
void set_foreground(char *, unsigned *);
void set_background(char *, unsigned *);
#endif

/*
--------------------------------------------------------- globals --------
*/

#ifdef CLOCK_SUPPORTED
static struct attribute attr_tab[] =
{
	{"bla",0},
	{"blu",1},
	{"gre",2},
	{"red",4},
	{"cya",3},
	{"mag",5},
	{"bro",6},
	{"whi",7},

	{"dgr",8},
	{"bbl",9},
	{"bgr",10},
	{"bcy",11},
	{"bre",12},
	{"bma",13},
	{"yel",14},
	{"bwh",15},
	{"",0}
} ;

unsigned attribute = 7;
#endif

int cmd;
char **msgv;

char buffer[256];

char *months[] = {"---", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
				"Aug", "Sep", "Oct", "Nov", "Dec", "---", "---", "---"};

unsigned msize[] ={ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

struct tm tim;

bool set_date = FALSE, UTC = FALSE, sleep_mode = FALSE;

char *cmdname;

int hour_specified=0;


#if defined(__QNXNTO__) || defined (__CYGWIN__)
int verbose=0;

long        rate = 20*30;  /* slow adjust default is to do it over 30 sec (600 tick) interval */
long        minsec = 1;   /* minimum number of seconds over which any
                              required slow adjustment will be spread */
long        maxsec = 60*5; /* maximum _duration_ of the slow-adjustment
                              time period i.e. system will be back to
                              'normal' after this. */
#endif

void abarf(char *str)
{
	fprintf(stderr,"%s: Invalid %s\n",cmdname,str);
	exit(-1);
}

int main (int argc, char *argv[])
{
	time_t lsec=0L;
	int display_seconds_only=0;
	unsigned status=0;
	char am_pm, flg24;
	bool errflg = FALSE;
	bool dashs = FALSE;
	int rc, c, num_params;
#define DATE_BASE_OPTS "us:t"
#ifndef __QNXNTO__
	#define DATE_EXTRA_OPTS "n:"
	nid_t nid = 0;
#else
	#define DATE_EXTRA_OPTS "S:v";
	struct timespec new, cur;
	struct _clockadjust adj;
#endif
	char *opts;

	cmdname = argv[0];

#ifdef CLOCK_SUPPORTED
	#define CLOCK_OPTS "n:usb:c:f:"

	if (strcmp("clock",basename(cmdname)) && strcmp("?clock",basename(cmdname))) {
		cmd = DATE;
		opts = DATE_BASE_OPTS DATE_EXTRA_OPTS;
	} else {
		cmd = CLOCK;
		opts = CLOCK_OPTS;
		if (geteuid()) {
			errflg++;
		    fprintf(stderr,"%s: Sorry - must be root to run clock.\n",argv[0]);
		}
	}
#else
	cmd = DATE;
    opts = DATE_BASE_OPTS DATE_EXTRA_OPTS;
#endif

	while ((c=getopt(argc,argv,opts)) != -1) {
		switch(c) {
#ifdef CLOCK_SUPPORTED
			case 'c':	attribute = parse_attribute(optarg); break;
			case 'f':	set_foreground(optarg,&attribute); break;
 			case 'b':	set_background(optarg,&attribute); break;
#endif

#ifndef __QNXNTO__
			case 'n':	nid = strtonid(optarg, NULL); break;
#endif
			case 'u':	UTC = TRUE; 
						putenv("TZ=UTC0");
						break;
			case 't':   display_seconds_only++; break;
#ifdef __QNXNTO__
            case 'S': maxsec = atol(optarg); break; 
			case 'v':   verbose++; break;
#endif
			case 's':	if (cmd == DATE) {
							errno=0;
							dashs=TRUE;
							lsec = strtol(optarg,NULL,0);
							if (errno) {
								fprintf(stderr,"%s: bad seconds value '%s'\n",cmdname,optarg);
								exit (EXIT_FAILURE);
							}
						} else {
							sleep_mode=TRUE;
						}
						break;

			case '?':	errflg = TRUE; break;
		}
	}

	num_params = argc - optind;

	if (num_params && *(argv[optind])!='+')
	{
#ifndef __QNXNTO__
		if  (cmd==CLOCK) errflg = TRUE;
		else {
#endif
			set_date = TRUE;
			if (num_params > 7) {
				errflg = TRUE;
				fprintf(stderr,"%s: too many operands\n",cmdname);
			}
#ifndef __QNXNTO__
		}
#endif
	} else if (num_params>1) {
		errflg = TRUE;
		fprintf(stderr,"%s: too many operands\n",cmdname);
	}
	
	if(errflg) exit(EXIT_FAILURE);

	if (!dashs) {
#ifndef __QNXNTO__
		if (nid == 0)
			lsec = fish_time(NULL);
		else {
			struct timespec spec;
			pid_t vid = qnx_vc_attach(nid, PROC_PID, 0, 0);
			if (vid == -1) {
				fprintf(stderr,"%s: %s: node %ld\n", cmdname, strerror(errno), nid);
				exit (EXIT_FAILURE);
			}
			qnx_getclock(vid, CLOCK_REALTIME, &spec);
			lsec = spec.tv_sec;
		}
#else
		lsec = fish_time(NULL);
#endif

		if (display_seconds_only) {
			printf("%ld\n",(unsigned long)lsec);
			exit(EXIT_SUCCESS);
		}
	} else if (set_date) {
		fprintf(stderr,"%s: the -s option cannot be used to set the date\n",
			cmdname);
		exit (EXIT_FAILURE);
	}

	if (set_date)
	{
		argc -= (optind-1);
		argv += (optind-1);
	} else {
		struct tm *bkdn;
#ifndef __QNXNTO__
		volatile struct _timesel __far *timeptr;
		long lnsec;
#endif

#ifndef __QNXNTO__
		if (cmd==CLOCK) {
			timeptr=fish_qnx_time();
			lnsec = timeptr->nsec;
			setprio(0,1);
			fclose(stdin);
			fclose(stderr);
			fclose(stdout);
		}
#endif

#ifdef CLOCK_SUPPORTED
		do {
			if (cmd==CLOCK) {
				/* if in sleep mode, then this is unnecessary */
				if (!sleep_mode) {
					/* wait for a change in nsec since last time */
					while (timeptr->nsec==lnsec);
				}

				/* get the time */
				lsec = fish_time(NULL);
			}
#endif

			bkdn = UTC?gmtime(&lsec):localtime(&lsec);
			if (bkdn) {
				if (num_params) strftime(buffer,256,argv[optind]+1,bkdn);
				else strftime(buffer,256,"%a %b %d %T %Z %Y",bkdn);
#ifdef CLOCK_SUPPORTED
				if (cmd == CLOCK) {
					display_clock(buffer);
					if (sleep_mode) sleep(1);
				}
				else
#endif
					printf("%s\n",buffer);

#ifdef __QNXNTO__
				if (verbose) {
					/* if verbose, if there are any outstanding ClockAdjustments
			           in progress, report this fact */
					clock_getres(CLOCK_REALTIME,&cur);
					ClockAdjust(CLOCK_REALTIME,NULL,&adj);
			
					if (adj.tick_count) {
						printf("date: time undergoing adjustment (%6.6ld usec/sec over %ld.%2.2ld sec)\n",
			                  ((1000000000L/cur.tv_nsec)*adj.tick_nsec_inc)/1000L,
			                  adj.tick_count/(1000000000L/cur.tv_nsec),
			                  100L*(adj.tick_count%(1000000000L/cur.tv_nsec))/(1000000000L/cur.tv_nsec)
							);
					}
				}
#endif
			}
#ifdef CLOCK_SUPPORTED
		} while (cmd == CLOCK);
#endif
		exit(0);
	}
	
	#ifdef DIAG
		printf("argc = %d\n",argc);
		for (c=0;c<argc;c++) printf("argv[%d] = %s\n",c,argv[c]);
		printf("num_params = %d\n",num_params);
	#endif

	{
		struct tm *ptr;
		ptr = localtime(&lsec);
		memcpy(&tim, ptr, sizeof(struct tm));
	}

	am_pm = 0;
	flg24 = 1;
	if(tim.tm_hour > 11) am_pm = 1;

	if ((num_params>1)||(strlen(argv[1])<3)) {

		if(*argv[argc-1] == 'a'  ||  *argv[argc-1] == 'p') {
			--argc;
			am_pm = (char)(*argv[argc] == 'p');
			flg24 = 0;
		}

		switch(argc) {
			case 7:	tim.tm_sec = ctod(argv[6]);
					if(tim.tm_sec > 59) abarf("second");
                    /* fall thru */
			case 6:	tim.tm_min = ctod(argv[5]);
					if(tim.tm_min > 59) abarf("minute");
                    /* fall thru */
			case 5:	tim.tm_hour = ctod(argv[4]);
					hour_specified++;
					if(tim.tm_hour > 23)	abarf("hour");
			 		if(am_pm  &&  !flg24  &&  tim.tm_hour < 12) tim.tm_hour += 12;
			 		if(!am_pm && !flg24 && tim.tm_hour == 12) tim.tm_hour = 0;
					if(tim.tm_hour > 11) am_pm = 1;
                    /* fall thru */
			case 4:{
				int year = ctod(argv[3]);
				/* 2 digits year handled specially */
				if (strlen(argv[3]) == 2)
					if (year >= 70)
						year += 1900;
					else
						year += 2000;
				if (year<BEGIN_EPOCH) abarf("year"); /* in case of a 4 digits date < BEGIN_EPOCH */
				tim.tm_year = year - MKTIME_BASE_CENTURY;
                    /* fall thru */
			}
			case 3:	if(argv[2][0] >= '0'  &&  argv[2][0] <= '9') tim.tm_mon = ctod(argv[2]) - 1;
					else
					{  /* try decoding string */
						for(tim.tm_mon = 0; tim.tm_mon < 12; ++tim.tm_mon)
							if(comp(months[tim.tm_mon + 1], argv[2])) break;
					}
					if(tim.tm_mon>11) abarf("month");
					/* fall thru */
			case 2:	if(tim.tm_mon==1 && (tim.tm_year%4)==0) msize[2]=29; /* leap yrs */
					tim.tm_mday=ctod(argv[1]);
					if(tim.tm_mday>msize[tim.tm_mon+1] || tim.tm_mday==0) abarf("day");
				break;
	
		}
	} else {
		int year = tim.tm_year+1900; /* defaults to current year */
		int year_digits = 4;
		
		/* third or fourth synopsis, only one operand */
		if ( (strlen(argv[1])==10)&&(ctod(argv[1]+8)>59)) {
			/* sys V MMDDhhmmYY If last two digits (as an int) are >59,
               it must be a year specified at the end */
			year        = ctod(argv[1]+8);		argv[1][8] = 0; year_digits = 2;
			tim.tm_min	= ctod(argv[1]+6);		argv[1][6] = 0;
			tim.tm_hour	= ctod(argv[1]+4);		argv[1][4] = 0; hour_specified++;
			tim.tm_mday	= ctod(argv[1]+2);		argv[1][2] = 0;
			tim.tm_mon	= ctod(argv[1]+0)-1;    argv[1][0] = 0;
		} else {
			/* Sun, POSIX touch format   */
			/* [[[[CC]YY]MM]DD]hhmm[.SS] */
			/* must be at least four bytes */
			char *ptr;

			if (strlen(argv[1])<4) abarf("date specification\n");

            if ((ptr = strchr(argv[1],'.'))) {
				tim.tm_sec = ctod(ptr+1); *ptr = 0;
			} else ptr=argv[1]+strlen(argv[1]);

			if (strlen(argv[1])%2) abarf("date specification (odd # digits)\n");

			if (ptr!=argv[1]) { tim.tm_min  = ctod(ptr-=2);		*ptr=0; }
			if (ptr!=argv[1]) { tim.tm_hour = ctod(ptr-=2);		*ptr=0; hour_specified++; }
			if (ptr!=argv[1]) { tim.tm_mday = ctod(ptr-=2);		*ptr=0; }
			if (ptr!=argv[1]) { tim.tm_mon  = ctod(ptr-=2)-1;	*ptr=0; }
			if (ptr!=argv[1]) { year        = ctod(ptr-=2);		*ptr=0; year_digits = 2; }
			if (ptr!=argv[1]) { year       += ctod(ptr-=2)*100;	*ptr=0; year_digits = 4; }
		}
		if(tim.tm_mon==1 && (tim.tm_year%4)==0) msize[2]=29; /* leap yrs */
		if(tim.tm_hour > 11) am_pm = 1;

		/* check for in-range numbers */
		if(tim.tm_sec > 59)     abarf("second");
		if(tim.tm_min > 59)     abarf("minute");
		if(tim.tm_hour > 23)	abarf("hour");

		if (year_digits == 2)
			if (year >= 70)
				year += 1900;
			else
				year += 2000;
		if (year<BEGIN_EPOCH) abarf("year"); /* in case of a 4 digits date < BEGIN_EPOCH */
        tim.tm_year = year - MKTIME_BASE_CENTURY;

		if(tim.tm_mon>11) abarf("month");
		if(tim.tm_mday>msize[tim.tm_mon+1] || tim.tm_mday==0) abarf("day");
	}
								

	#ifdef DIAG
		printf("DIAG: %s\n",asctime(&tim));
		printf("DIAG: timezone = %ld\n",timezone);
	#endif
	
	/* set isdst to -1 to force mktime to try & figure out whether dst is
       in effect or not. Otherwise we would be inheriting the dst state from
       the current time, which could be quite different from the time that
       is being set */

	if (hour_specified) {
		tim.tm_isdst = -1;
	}

	lsec = mktime(&tim);
	if (lsec == -1)
		abarf("date");
	
	if (UTC)
	{
		#ifdef DIAG
			printf("DIAG: lsec = %ld\n",lsec);
			printf("DIAG: timezone = %ld\n",timezone);
		#endif
		/*
		lsec -= (timezone - (daylight?3600:0));
		*/
		lsec -= timezone;
	}

	/* handle slow adjustment, if new time is 'close' to current OS time */
	rc=-1;
#ifdef __QNXNTO__
	{
		time_t sec;
		long   usec;

		new.tv_sec = lsec;
		new.tv_nsec = 0L;

		/* QNX4 would be:
		*		getclock(TIMEOFDAY, &cur);
		*		if((sec = new.tv_sec - cur.tv_sec) >= -maxsec  &&  sec <= maxsec) {
		*			usec  = new.tv_sec*1000000 + new.tv_nsec/1000;
		*			usec -= cur.tv_sec*1000000 + cur.tv_nsec/1000;
		*			rc = qnx_adj_time(usec, rate, NULL, NULL);
		*			}
		*/

        /* Neutrino version */

		clock_gettime(CLOCK_REALTIME,&cur);

		if (verbose>1) printf("date: cur.tv_sec=%ld, cur.tv_nsec=%ld\n",(unsigned long)cur.tv_sec,cur.tv_nsec);

		sec = new.tv_sec - cur.tv_sec;

		/* we will not speed up the clock by >100%, nor slow it
           down by >50%. The maximum number of seconds to make up the
           difference is 'maxsec' */

		if( sec<maxsec && sec>-(maxsec/2) ) {

			if (verbose) printf("date: attempting slow adjust (delta = %d sec.)\n",sec);
			usec  = new.tv_sec*1000000 + new.tv_nsec/1000;
			usec -= cur.tv_sec*1000000 + cur.tv_nsec/1000;

			clock_getres(CLOCK_REALTIME,&cur);
			
			if (verbose>1) printf("date: clock resolution = %ld\n",cur.tv_nsec);

			/* we need to pick a rate which will give us a time interval
               between minsec and maxsec seconds for the entire operation */
			{
				long remainder, tickspersecx10, minticks, maxticks, minrate, maxrate;

				tickspersecx10=1000000000L/cur.tv_nsec;
				remainder=1000000000L%cur.tv_nsec;
				tickspersecx10=10*tickspersecx10+(10*remainder)/cur.tv_nsec;

				if (verbose>1) printf("tickspersec = %ld.%ld\n",tickspersecx10/10,tickspersecx10%10);

				minticks=(minsec*tickspersecx10)/10L;
				maxticks=(maxsec*tickspersecx10)/10L;

				if (verbose>1) printf("minticks=%ld (minsec=%ld)\n",minticks,minsec);
				if (verbose>1) printf("maxticks=%ld (maxsec=%ld)\n",maxticks,maxsec);

				maxrate = usec/minticks;	/* in usec */
				minrate = usec/maxticks;    /* in usec */

				if (verbose>1) printf("total usec=%ld\n",usec);
				if (verbose>1) printf("maxrate=%ld (usec), minrate=%ld (usec)\n",maxrate,minrate);

				/* cap rate at +ticksize, -0.5 ticksize */
				if (maxrate*1000L>cur.tv_nsec) maxrate=(cur.tv_nsec+999L)/1000L;
				if (maxrate*2000L<-cur.tv_nsec) maxrate=-(cur.tv_nsec+999L)/2000L;

				/* min rate is already capped by sec<maxsec sec*2>-maxsec */

				if (verbose>1) printf("capped maxrate=%ld (usec), minrate=%ld (usec)\n",maxrate,minrate);

				/* weight towards minrate end */
				adj.tick_nsec_inc = 1000*(minrate*2+maxrate)/3;
               // adj.tick_count = (maxticks*2+minticks)/3;
     adj.tick_count = usec/(adj.tick_nsec_inc/1000);
			}
		
			if (verbose) {
				printf("date: Adjusting clock %ld000 nsec\n",usec);
				printf("date: (%9.9ld nsec per tick over %ld ticks)\n",adj.tick_nsec_inc,adj.tick_count);
			}
			/* Should we use a different symbol for when we try and slow adjust the date? */
			logwtmp("|", "date", "");
			rc= ClockAdjust(CLOCK_REALTIME,&adj,NULL);
			if (verbose>1) {
				clock_getres(CLOCK_REALTIME,&cur);
				printf("date: clock resolution = %ld\n",cur.tv_nsec);
				do {				
					sleep(1);
					ClockAdjust(CLOCK_REALTIME,NULL,&adj);
					clock_gettime(CLOCK_REALTIME,&cur);
					printf("date: tick_count=%ld sec=%ld nsec=%ld\n",adj.tick_count,
                                  (unsigned long)cur.tv_sec, cur.tv_nsec);
				} while (adj.tick_count!=0);
            }
			logwtmp("{", "date", "");
		}
	}
#endif

	/* if not using slow adjust, or slow adjust failed, set time */
	if (rc==-1) {
		struct timespec temp;

		temp.tv_sec = lsec;
		temp.tv_nsec = 0;
#ifdef __QNXNTO__
		logwtmp("|", "date", "");
		status=clock_settime(CLOCK_REALTIME,&temp);
#else 
		status=setclock(TIMEOFDAY,&temp);
#endif
		if (status==-1) {
			perror("Can't set system time");
		}
#ifdef __QNXNTO__
		logwtmp("{", "date", "");
#endif
	}


#ifndef __QNXNTO__
	#ifdef DIAG
		printf("calling fish_time\n");
	#endif
	lsec = fish_time(NULL);
	#ifdef DIAG
		printf("which returned %ld\n",lsec);
	#endif
#else
	lsec = time(NULL);
#endif

	{
		struct tm *bkdn;

		bkdn = UTC?gmtime(&lsec):localtime(&lsec);
		if (bkdn)
		{
			#ifdef DIAG
				printf("DIAG: %s\n",asctime(bkdn));
				fflush(stdout);
			#endif
			strftime(buffer,256,"%a %b %d %T %Z %Y",bkdn);
			printf("%s\n",buffer);
		}
	}

	return(status);
}

int ctod(char *s)
{
	int i = 0;
	char *p = s;

	while (*p>='0' && *p<='9') i = i*10 + *p++ - '0';

	if(*p != '\0')
		abarf("time. Numbers may only contain the digits 0-9");

	return(i);
}

int comp(char *s1, char *s2)
{
	register char *p1 = s1;
	register char *p2 = s2;

	while(*p1) if((*p1++ | ' ') != (*p2++ | ' ')) return(0);
	return(1);
}

#ifndef __QNXNTO__
void set_background(char *string, unsigned *attribute)
{
	int i;

	for (i=0;attr_tab[i].name[0];i++) if (!strncmp(attr_tab[i].name,string,3)) break;
    if ((attr_tab[i].name[0]) && (attr_tab[i].value<8))
	{
		*attribute = *attribute & 0x0F;
		*attribute |= attr_tab[i].value<<4;
	} else {
		printf("%s:Not a valid background attribute ('%s')\n",cmdname,string);
		exit(EXIT_FAILURE);
	} 
}

void set_foreground(char *string, unsigned *attribute)
{
	int i;

	for (i=0;attr_tab[i].name[0];i++) if (!strncmp(attr_tab[i].name,string,3)) break;
	if (attr_tab[i].name[0])
	{
		*attribute = *attribute & 0x70;
		*attribute |= attr_tab[i].value;
	} else {
		printf("%s: Not a valid attribute ('%s')\n",cmdname,string);
		exit(EXIT_FAILURE);
	} 
}

unsigned parse_attribute(char *string)
{
	unsigned attribute = 7;
	char *tmp;
	bool weird_case = FALSE;

	while (*string)
	{
		if (!strncmp("fg:",string,3) || !strncmp("fg=",string,3) || weird_case )
		{
			if (!weird_case) string+=3;
			else weird_case = FALSE;

			while (isspace(*string)) string++;
			for (tmp=string;!isspace(*tmp) && *tmp;tmp++) ;
			if (isspace(*tmp)) 
			{
				*tmp = 0;
				tmp++;
			}
	
			set_foreground(string,&attribute);

			string = tmp;
		}
		else if (!strncmp("bg:",string,3) || !strncmp("bg=",string,3))
		{
			string+=3;
			while (isspace(*string)) string++;
			for (tmp=string;!isspace(*tmp) && *tmp;tmp++) ;
			if (isspace(*tmp)) 
			{
				*tmp = 0;
				tmp++;
			}
			set_background(string,&attribute);
			string = tmp;
		} else {
			weird_case = TRUE;
		}
	}

	return (attribute);
}

void display_clock(char *buf)	
{
	short unsigned int width;
	short int len, i;
	unsigned short __far *ptr;

	/* display to both monochrome and colour memory areas */
	width = *((short unsigned far *)MK_FP(0x0040, 0x004A));
	if(width < 20 || width > 256)
		width = 80;

	len = strlen(buf);

	for (i=0;buf[i];i++)	
	{
		ptr = MK_FP(0x28, (i + width - len) * 2);
		*ptr = ((unsigned)buf[i]) | (attribute << 8);
	}
	for (i=0;buf[i];i++)	
	{
		ptr = MK_FP(0x30, (i + width - len) * 2);
		*ptr = ((unsigned)buf[i]) | (attribute << 8);
	}
}
#endif

