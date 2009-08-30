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





#include "cron.h"
#include <syslog.h>
#include <limits.h>

static int listed(char *file, char *user);
static int not_authorized(char *user);
static char * parse_ctab(char * line, unsigned long * tim);

/* called by sys dependant routines */
void message(int stream, char *fmt, ...);
long do_jobs();
void ctab_load(int signo);

/* sys-dependant routines */
/* int main(int *, char **); */
void cleanup(int signo);
void trigger(cron_job * job);

#define range_to_bits(a,b,t) (~((~(t)0) << ((b) - (a) + 1)) << (a))
#define value_to_bit(a,t)    (((t)1) << (a))

/* Offsets of the time fields (struct tm names). */
#define MIN_LO 0
#define HOUR   1 
#define MDAY   2 
#define MON    3
#define WDAY   4 
#define MIN_HI 5 /* Not a time for the purposes of iteration */
#define NUM_TIMES 5

const int limits[NUM_TIMES] = { 59, 23, 31, 12, 6 };

/* Names for the different kinds of message() */
#define LOG   1        /* Only with -v           */
#define FATAL 2        /* Fatal                  */
#define FATAL_FORKED 3 /* Fatal without shutdown */
#define EVENT 4        /* Non-fatal              */

extern char * crondir; /* Set by the sys dependant main routine */

/*  Allows cron_jobs to be reloaded by a signal handler */
static cron_job * cron_jobs;

/*  Extra parameters to the message() function, set by the main()
 *  function when parsing options.
 */
int is_logging;
char * argv0;

/* output a message prefixed with argv[0] */
void message(int stream, char *fmt, ...) {
    va_list ap;
	char buf[TAB_BUFFER_SIZE];
	
    if(stream != LOG || is_logging) {
	    va_start(ap, fmt);
		vsprintf( buf, fmt, ap);
   		// print to syslog and stderr
		syslog( LOG_INFO, buf);
	    va_end(ap);
    }
    if(stream == FATAL)
	cleanup(0); /* Calls message with a LOG */
    if(stream == FATAL_FORKED)
	exit(-1);
}

char * cronstrtime() 
{
	static char buf[20];
	time_t tim;

	time(&tim);
	strftime(buf, sizeof buf, "%b %d %T", localtime(&tim));

	return buf;
}

/*  Loads one line of the crontab into the provided crtime array.
 *  The cron_time structure is assumed to be zeroed out.
 *  The expected format is in the crontab man page.
 *  If the operation is succesful, a pointer to the command portion 
 *  is returned.  Otherwise NULL is returned.  Bulletproof.
 */
static char * parse_ctab(char * line, unsigned long * crtime) {
	int n, a, b, tmp, i = 0;
	char *field;

	memset(crtime, 0x0, sizeof (*crtime) * TIMES_ELEMENTS);
	if((field = strtok(line, " \t")) == NULL)
		return NULL;

	do { /* Iterates through the first 5 of 6 times elements. */
		if(strcmp(field, "*") == 0) {
			crtime[i] = ~((unsigned long)0);
			if(i == MIN_LO)
				crtime[MIN_HI] = ~((unsigned long)0);
		} else for(;; ++field) { /* comma delimited list */
			if(sscanf(field, "%d%n", &a, &n) <= 0) {
				/* It's non-numeric and isn't a '*' */
				return NULL;

			} else if(a > limits[i] || a < (i == MON || i == MDAY)) {
					return NULL;
			/* Range */
			} else if(*(field += n) == '-') {
				if(sscanf(field, "-%d%n", &b, &n) <= 0)
					return NULL;

				if(b > limits[i] || b < (i == MON || i == MDAY))
					return NULL;

				if(a > b) { /* Required for the range_to_bits */
					tmp = a;
					a = b;
					b = tmp;
				}

				if(a < 32) { /* Silly on a 64 bit machine */
					tmp = b < 32 ? b : 31;
					if(a == 0 && tmp == 31) /* n << 32 seems to produce -1 */
						crtime[i] = ~(long unsigned)0;
					else
						crtime[i] |= range_to_bits(a, tmp, unsigned long);
				}
				if(b >= 32) { /* Only field larger than 32 is MIN */
					tmp = a > 32 ? a - 32 : 0;
					crtime[MIN_HI] |= range_to_bits(tmp, b-32, unsigned long);
				}

				field += n;
			} else { /* Single value */
				if(a >= 32)
					crtime[MIN_HI] |= value_to_bit(a-32, unsigned long);
				else
					crtime[i] |= value_to_bit(a, unsigned long);
			}
			if(*field != ',') { /* Loop through a list */
				if(*field != '\0')
					return NULL; /* Illegal element delimiter */
				break;
			}
		}
	} while(++i < NUM_TIMES && (field = strtok(NULL, " \t")));

	return i == NUM_TIMES ? strtok(0, "") : NULL;
}

/* look for user in file, names are whitespace delimited tokens */
static int listed(char *file, char *user) {
    char line[51]; /* 1 + the scan code value below */
    FILE * fp;

    if ((fp = fopen(file, "r")) == 0)
	return -1;  /* file does not exist */

    while (fscanf(fp, "%50s", line) != EOF) {
		if (strcmp(user, line) == 0) {
			fclose(fp);
			return 1;
		}
    }
    fclose(fp);
    return 0;

}

/* determine if crontab for user will be run */
static int not_authorized(char *user) {
    switch (listed("cron.allow", user)) {
	case -1:	/* no cron.allow, authorize if not denied */
		return listed("cron.deny", user) == 1;
	case  0:	/* not found, authorize if not denied */
		return listed("cron.deny", user) == 1;
	case  1:	/* allowed, authorize */
		return 0;
    }
    return 1;
}

/* Load the crontabs */
void ctab_load(int signo) {
    DIR *dirp;
    cron_job * job_tmp, * job_index = cron_jobs;

    if(signo)
		message(LOG, "crontab update" );

    /* Throw out the old cron table entries */
    while(job_index != NULL) {
		assert(job_index->command != NULL);
		free(job_index->command);
		job_tmp = job_index->next;
		free(job_index);
		job_index = job_tmp;
    }
    cron_jobs = NULL;

    /* read all crontabs */
    if (dirp = opendir("crontabs/")) {
		struct dirent *dent;
		char entrybuf[TAB_BUFFER_SIZE];
		unsigned long crtime[TIMES_ELEMENTS];
		char * command;
		char * comptr;
		FILE * fp;
		int lineno;
		struct passwd * pwinfo;

		while (dent = readdir(dirp)) { /* Loop through crontabs */
			if (dent->d_name[0] == '.')
				continue;

			if(not_authorized(dent->d_name)) {
				message(LOG, "unauthorized crontab %s", dent->d_name);
				continue;
			}
			pwinfo = getpwnam(dent->d_name);
			if(pwinfo == NULL) {
				message(LOG, "skipping crontab without user: %s", dent->d_name);
				continue;
			}
			strcat(strcpy(entrybuf, "crontabs/"), dent->d_name);
			if((fp = fopen(entrybuf, "r")) == NULL) {
				message(EVENT, "%s: %s", entrybuf, strerror(errno));
				continue;
			}

			lineno = 0; /* loop through lines in crontab */
			while(fgets(entrybuf, TAB_BUFFER_SIZE, fp) != NULL) {
				++lineno;

				comptr = entrybuf; /* translate '%' and truncate '\n' */
				while(comptr = strpbrk(comptr, "\n%")) {
					if(*comptr == '%') {
						if(*(comptr - 1) == '\\')
							memmove(comptr - 1, comptr, strlen(comptr) + 1);
						else {
							*comptr = '\n';
							++comptr;
						}
					} else {
						*comptr = '\0';
						break; /* newline found, comptr non-null */
					}
				}

				if(comptr == NULL && !feof(fp)) {
					message(LOG, "line too long in crontab: %s %d", dent->d_name, lineno);
					while(fgets(entrybuf, TAB_BUFFER_SIZE, fp) != NULL) {
						if(NULL != strchr(entrybuf, '\n')) 
							break;
					}
					continue; /* Couldn't use this entry */
				}

				if(entrybuf[0] == '#' || entrybuf[0] == '\0')
					continue;

				if((command = parse_ctab(entrybuf, crtime)) == NULL) {
					message(LOG, "invalid crontab: %s line %d", dent->d_name, lineno);
					continue;
				}

				if(((job_tmp = malloc(sizeof(cron_job))) == NULL)
				 || ((job_tmp->command = strdup(command)) == NULL)) {
					message(EVENT, "%s: %s", dent->d_name, strerror(errno));
					free(job_tmp); /* may be NULL */
					break;
				}

				job_tmp->next = cron_jobs; /* Enlist job */
				cron_jobs = job_tmp; 

				memcpy(job_tmp->time, crtime, sizeof crtime);

				job_tmp->user = pwinfo->pw_uid;
			}

			fclose(fp);
		}

		closedir(dirp);
    } else {
		message(FATAL, "%s/crontabs: %s", crondir, strerror(errno));
    }
}

#define INF INT_MAX

/*  Returns INF if no bit is set at or beyond the 'time' bit in current_lo
 *  or 'time - 32' in current_hi. 
 *  Returns the 'time' for to the set bit otherwise.
 */
static int nexttime(int crtime, unsigned long current_hi, unsigned long current_lo, unsigned size) {
	unsigned long 	index;
	unsigned		term;

	if(crtime < 32) {
		index = 1 << crtime;
		term = size;
		if(term >= 32) term = 32;
		while(crtime < term) {
			if(current_lo & index) {
				return crtime;
			}
			++crtime;
			index <<= 1;
		}
		index = 1;
	} else {
		index = 1 << (crtime - 32);
	}
	while(crtime < size) {
		if(current_hi & index) {
			return crtime;
		}
		++crtime;
		index <<= 1;
	}

	return INF;
}

/*  Does all jobs scheduled for the current min.
 *  Returns the number of seconds to sleep until the next job or -1
 *  to sleep till 00:00:00 am.
 */
long do_jobs() {
	register cron_job * job_index;
	int nextmin, best_nextmin = 60;
	int nexthr,  best_nexthr  = 23;
	int tryhr;
	int nowhr;
	int nowmin;
	long unsigned now_time[TIMES_ELEMENTS];
	struct tm * now_tm; /* Used for formulating current_time */
	time_t gmtm;
	
	time(&gmtm); /* place time in same array as used for jobs */
	now_tm = localtime(&gmtm);
	/* make a copy of these two since '*now_tm' data gets overwritten */
	nowhr = now_tm->tm_hour; 
	nowmin = now_tm->tm_min;
	now_time[MDAY] = value_to_bit(now_tm->tm_mday, long unsigned); 
	now_time[WDAY] = value_to_bit(now_tm->tm_wday, long unsigned); 
	now_time[MON]  = value_to_bit(now_tm->tm_mon + 1, long unsigned);
	now_time[HOUR] = value_to_bit(now_tm->tm_hour, long unsigned);
	if(now_tm->tm_min < 32) {
		now_time[MIN_LO] = value_to_bit(now_tm->tm_min, long unsigned);
		now_time[MIN_HI] = 0;
	} else {
		now_time[MIN_LO] = 0;
		now_time[MIN_HI] = value_to_bit(now_tm->tm_min - 32, long unsigned);
	}

	for(job_index = cron_jobs; job_index; job_index = job_index->next) {
		/* Is the job scheduled for today? */
		if((job_index->time[WDAY] & now_time[WDAY])
		  && (job_index->time[MDAY] & now_time[MDAY])
		  && (job_index->time[MON] & now_time[MON])) {

			/* Check if this job is scheduled for this min */
			if((job_index->time[HOUR] & now_time[HOUR])
			  && ((job_index->time[MIN_LO] & now_time[MIN_LO])
			    || (job_index->time[MIN_HI] & now_time[MIN_HI]))) {
				  trigger(job_index);
			}

			/* Check if this job is the next ready */ 
			nextmin = INF;
			tryhr = nowhr;
			for(;;) {
				nexthr = nexttime(tryhr, 0, job_index->time[HOUR], 24);
				if(nexthr == INF) break;
				nextmin = nexttime((nexthr == nowhr) ? (nowmin+1) : 0, job_index->time[MIN_HI], job_index->time[MIN_LO], 60);
				if(nextmin != INF) break;
				++tryhr;
			}

			if((nexthr < best_nexthr) 
			  || ((nexthr == best_nexthr) && (nextmin < best_nextmin))) {
				best_nexthr  = nexthr;
				best_nextmin = nextmin;
			}
		} /* end if today */
	}

	time(&gmtm); /* recalculate time in case loop above took a long time */
	now_tm = localtime(&gmtm);
	
	nexthr  = best_nexthr  - now_tm->tm_hour;
	nextmin = best_nextmin - now_tm->tm_min;

	/* Correct for seconds to avoid all but recent drifting */
	return nextmin * 60 + nexthr * ((long)60 * 60) - now_tm->tm_sec; 
}

#ifdef EXTENDED_DIAGNOSTICS /* Left as a debugging aid */

void cron_ls(int signo)
{
	cron_job * index = cron_jobs;

	while(index) {
		printf("* min:");
		field_spew(index->time[MIN_LO], 0);
		field_spew(index->time[MIN_HI], 32);
		printf(" hour:");
		field_spew(index->time[HOUR], 0);
		printf(" mday:");
		field_spew(index->time[MDAY], 0);
		printf(" mon:");
		field_spew(index->time[MON], 0);
		printf(" wday:");
		field_spew(index->time[WDAY], 0);
		printf("\n* command = \"%s\"\n# \n", index->command);

		index = index->next;
	}
}

void field_spew(long unsigned sp, int plus)
{
	int offset = 0;
	int last = 0;
	unsigned long current = 1;

	while (current) {
		if(current & sp) {
			if(!last)
				printf(" %d", offset + plus);
			++last;
		} else {
			if(last > 1)
				printf("-%d", offset - 1 + plus);
			last = 0;
		}

		++offset;
		current <<= 1;
	}

	if(last > 1)
		printf("-%d", offset - 1);
}

#endif

