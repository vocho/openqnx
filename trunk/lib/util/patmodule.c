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
------------------------------------------------------------- includes -----
*/

#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <malloc.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef __MINGW32__
#include <lib/compat.h>
#else
#include <pwd.h>
#include <grp.h>
#include <fnmatch.h>
#endif

#include <util/defns.h>
#include <patmodule.h>
#include <util/stdutil.h>

/*
------------------------------------------------------------- text strings --
*/

#define T_INVALID_USER			"Unknown user (%s)\n"
#define T_INVALID_GROUP			"Unknown group (%s)\n"
#define T_GROUP_TOO_HIGH		"Group greater than 32767"
#define T_TOO_MANY_GROUPS		"Too many groups specified"
#define T_TOO_MANY_OWNERS		"Too many users specified"
#define T_TOO_MANY_PATTERNS 	"Too many patterns specified"
#define T_TOO_MANY_MODES		"Too many modes specified"
#define T_UID_RANGE				"Userid not in valid range"
#define T_NUM_PARSE_ERROR		"Error parsing number near '%s'\n"
#define T_OUT_OF_RANGE			"Number must be < 65536 (%s)\n"
#define T_ILLEGAL_MODE_SPEC 	"Illegal mode spec '%s'\n"
#define T_NUM_OUT_OF_RANGE  	"Octal number out of range (%s)\n"

#define TXT(s) s

#define isoctal(c) (c>='0' && c<='7')

#define NOT_CHARACTER ('!')

#define HEAVY TRUE 		/* means 'if no time specified, assume 23:59:59' */
#define LIGHT FALSE		/* means 'if no time specified, assume 00:00:00' */

#define OWNER_FLAG	1	/* in parse_nums, can be owner # or owner name */
#define GROUP_FLAG  2	/* in parse_nums, can be group # or group name */

/*
----------------------------------------------------- structs & typedefs ---
*/

struct modemode
{
	unsigned fmode;
	bool	 negative;
};

typedef struct modemode mode;

/*
------------------------------------------------------------- externs ------
*/

//extern char *malloc();

/*
-------------------------------------------------------- globble globbles --
*/

static int all_not_patterns;
static uint16_t numpatterns, numgroups, numowners, nummodes;
static int Maxgroups, Maxpatterns, Maxowners, Maxmodes;
time_t	_PM_afterdate, _PM_beforedate; /* NOT STATIC */
static uint16_t	*group_table;
static uint16_t	*owner_table;
static char		**pattern_table;
static mode  	*mode_table;

/*
----------------------------------------------------------- prototypes -----
*/

#ifdef __STDC__

	static int16_t   atoo		( char *string );
	uint8_t	octalorliteral		( char *string );

	static uint16_t  parse_nums	( uint16_t *table, char *string,
					  uint16_t *num,char *errstring,
					  uint16_t max, uint16_t flags); 
	static int16_t	parse_patterns	( char *table[],  char *string,
				 uint16_t *num, bool update_all_not_patterns,
				 char *errstring, uint16_t max );
	static int	parse_modes	( mode *table,  char *string,
					  uint16_t *num);
	static int	ctod		( char *s);
	static int	comp		( char *s1, char *s2 );
	static time_t	ascdatetotime	( char *string, int heavy );
#else 
	extern long ascdatetotime();
	extern int16_t parse_patterns();
	extern uint16_t parse_nums();
#endif

void
terminate(char *string)
{
	fprintf(stderr,"%s",string);
	exit(1);
}

/*
----------------------------------------------------- patmodule_init() -------
*/
int patmodule_init( int max_patterns,
					int max_groups,
					int max_owners,
					int max_modes )
{
	Maxpatterns = max_patterns;
	Maxgroups   = max_groups;
	Maxowners   = max_owners;
	Maxmodes    = max_modes;

	#ifdef DIAG
		fprintf(stderr,"Allocating memory for %d groups\n",Maxgroups);
	#endif
	group_table = malloc(Maxgroups*sizeof(*group_table));

	#ifdef DIAG
		fprintf(stderr,"Allocating memory for %d owners\n",Maxowners);
	#endif
	owner_table = malloc(Maxowners*sizeof(*owner_table));

	#ifdef DIAG
		fprintf(stderr,"Allocating memory for %d patterns\n",Maxpatterns+1);
	#endif
	pattern_table = malloc(Maxpatterns*sizeof(*pattern_table));

	#ifdef DIAG
		fprintf(stderr,"Allocating memory for %d modes\n",Maxmodes+1);
	#endif
	mode_table    = malloc(Maxmodes*sizeof(*mode_table));

	#ifdef DIAG
		fprintf(stderr,"Resetting dates, counts & flags\n");
	#endif

	_PM_afterdate   = _PM_beforedate = -1L;
	numgroups = numowners = numpatterns = 0;

	all_not_patterns = TRUE;

	#ifdef DIAG
		fprintf(stderr,"Finished patmodule_init()\n");
	#endif
	if ((group_table!=NULL) && (owner_table!=NULL) && (pattern_table!=NULL) && (mode_table!=NULL)) return(0);
	else return(-1);
}


/*
----------------------------------------------------- patmodule_free() -------
*/
void patmodule_free()
{
	free(group_table);
	free(owner_table);
	free(pattern_table);
	free(mode_table);
}
	
/*
----------------------------------------------------- patmodule_enter() ------
*/
int patmodule_enter(int type, char *opt_arg)
{
	/* fprintf(stderr,"type = %d, optarg = '%s'\n",type,optarg); */
	switch(type)
	{
		case P_AFTERDATE:  return(((_PM_afterdate=ascdatetotime(opt_arg,HEAVY))==-1)?-1:0);
		case P_BEFOREDATE: return(((_PM_beforedate=ascdatetotime(opt_arg,LIGHT))==-1)?-1:0);
		case P_OWNER: return(parse_nums(owner_table,opt_arg,&numowners,TXT(T_TOO_MANY_OWNERS),Maxowners,OWNER_FLAG));
		case P_GROUP: return(parse_nums(group_table,opt_arg,&numgroups,TXT(T_TOO_MANY_GROUPS),Maxgroups,GROUP_FLAG));
		case P_MODE: return(parse_modes(mode_table,opt_arg,&nummodes));
		case P_PATTERN: return(parse_patterns(pattern_table,opt_arg,&numpatterns,TRUE,TXT(T_TOO_MANY_PATTERNS),Maxpatterns));
	}
	return(-1);
}

/*
----------------------------------------------------- patmodule_commit() -----
*/

void patmodule_commit()
{
	static char all[2]="*";

	if (numpatterns && all_not_patterns)
	{
		parse_patterns(pattern_table,all,&numpatterns,TRUE,TXT(T_TOO_MANY_PATTERNS),Maxpatterns+1);
	}

	#ifdef DIAG
    {
		int i;
		
		for (i=0;i<numpatterns;i++)
		{
			fprintf(stderr,"pattern[%d] = '%s'\n",i,pattern_table[i]);
		}

		for (i=0;i<numgroups;i++)
		{
			fprintf(stderr,"group[%d] = %d\n",i,group_table[i]);
		}

		for (i=0;i<numowners;i++)
		{
			fprintf(stderr,"owner[%d] = %d\n",i,owner_table[i]);
		}

		for (i=0;i<nummodes;i++)
		{
			fprintf(stderr,"mode[%d] = %s%09o\n",i,mode_table[i].negative?"(not)":"",mode_table[i].fmode);
		}

	}		
	#endif
}


/*
----------------------------------------------------- patmodule_check() ------
*/

bool patmodule_check (char *fname, struct stat *statbufp)
{
	register char *p;
	char match_flag;
	int i;
	
	#ifdef DIAG
		fprintf(stderr,"ok_to_exec('%s',---)\n",fname);
	#endif

	/* check file group */
	if (numgroups) 
	{
		for (i=0;i<numgroups;i++)
		{
			if (group_table[i] == statbufp->st_gid) break;
		}

		if (i==numgroups) return(FALSE);
	}

	/* check owner */
	if (numowners)
	{
		for (i=0;i<numowners;i++)
		{
			if (owner_table[i] == statbufp->st_uid) break;
		}

		if (i==numowners) return(FALSE);
	}

	/* look at file modified time */

	if (_PM_afterdate!=-1L && (statbufp->st_mtime < _PM_afterdate)) return(0);
	if (_PM_beforedate!=-1L && (statbufp->st_mtime > _PM_beforedate)) return(0);

	#ifdef MATCHDIAG
		fprintf(stderr,"fname = '%s'\n",fname);
	#endif

	/* check patterns */
	for(match_flag = FALSE, i = 0 ; i < numpatterns ; ++i)
	{
		if(*(p = pattern_table[i]) == NOT_CHARACTER) {
			if(!fnmatch(p + 1,fname,0))	return(FALSE);
		} else {
			if(!(fnmatch(p,fname,0)))
			{
				match_flag = 1;
				#ifdef MATCHDIAG
					fprintf(stderr,"matched '%s'\n",p);
				#endif
			}
			#ifdef MATCHDIAG
			  else {
				 fprintf(stderr,"nomatch '%s'\n",p);
			  }
			#endif
		}
	}

	if (numpatterns && !match_flag) return(0);

	#ifdef DIAG
		fprintf(stderr,"ok_to_exec: nummodes = %d\n",nummodes);
	#endif

	{
		bool all_negative_modes = TRUE;
		for(match_flag=FALSE, i=0; i<nummodes ; ++i)
		{
			if(mode_table[i].negative) {
				#ifdef DIAG
					fprintf(stderr,"ok_to_exec: (!) st_mode = 0%o, fmode[%d] = 0%o\n",statbufp->st_mode,i,mode_table[i].fmode);
				#endif
				if ((statbufp->st_mode & mode_table[i].fmode)==mode_table[i].fmode) return(FALSE);
			} else {
				#ifdef DIAG
					fprintf(stderr,"ok_to_exec:     st_mode = 0%o, fmode = 0%o\n",statbufp->st_mode,mode_table[i].fmode);
				#endif
				all_negative_modes = FALSE;
				if ((statbufp->st_mode & mode_table[i].fmode)==mode_table[i].fmode)
				{
					#ifdef DIAG
						fprintf(stderr,"ok_to_exec: MATCH!\n");
					#endif
					match_flag = TRUE;
				}
			}
		}
		return((nummodes == 0)  ||  match_flag || all_negative_modes );
	}
}
	
/*
---------------------------------------------------------- cmd line parsing --
*/

static int16_t atoo(char *string)
{
	int16_t sum = 0;

	if (!string) return(-1);
	for (;isoctal(*string);string++)
	{
		sum *= 8;
		sum += *string - '0';
	}
	if ((sum<0)||(*string)) return(-1);
	else return(sum);
}

uint8_t octalorliteral (char *string)
{
	uint8_t rc;
	uint16_t numdigit;

	/* read	in octal sequence */
	for (	rc = 0,	numdigit=0;
			isoctal(*string) && (numdigit<3);
			numdigit++
		)
	{
		rc *= (uint8_t) 8;
		rc += (uint8_t) (*string++ - '0');
	}

	/* if it wasn't an octal sequence, take next char literal */
	if (!numdigit)
	{
		rc = *++string;
	}

	#ifdef DIAG
		fprintf(stderr,"Octal %d\n",(int) rc);
	#endif

	return(rc);
}

static uint16_t  parse_nums (
	uint16_t *table,
	char *string,
	uint16_t *num,
	char *errstring,
	uint16_t max,
	uint16_t flags)		/* OWNER_FLAG | GROUP_FLAG */
{
	char *ptr;
	char *item;
	bool hex;
	unsigned long ltemp;
	char *endptr;

	/* interpret 0x000 or nnnn decimal */

	for(ptr=string;*ptr;)
	{
		if ((flags==OWNER_FLAG)||(flags==GROUP_FLAG)) {
			char *comma;
			long result;

			comma = strchr(ptr,',');
			if (comma!=NULL) *comma='\000';

			if (flags==OWNER_FLAG) {
				struct passwd *pwtemp;
					
				pwtemp = getpwnam(ptr);
				if (!pwtemp) {
					if (strspn(ptr,"0123456789x")!=strlen(ptr)) {
						fprintf(stderr,TXT(T_INVALID_USER),ptr);
						exit(EXIT_FAILURE);
					}
				} else {
					result = (long) pwtemp->pw_uid;
					ptr = (comma==NULL) ? strchr(ptr,0) : (comma+1);
					if (comma!=NULL) *comma=',';
					table[(*num)++] = result;
					continue;
				}
			} else if (flags==GROUP_FLAG) {
				struct group *grtemp;
					
				grtemp = getgrnam(ptr);
	
				if (grtemp==NULL) {
					if (strspn(ptr,"0123456789x")!=strlen(ptr)) {
						fprintf(stderr,TXT(T_INVALID_GROUP),ptr);
						exit(EXIT_FAILURE);
					}
				} else {
					result = (long) grtemp->gr_gid;
					ptr = (comma==NULL)?strchr(ptr,0):(comma+1);
					if (comma!=NULL) *comma=',';
					table[(*num)++] = result;
					continue;
				}
			}


			if (comma!=NULL) *comma=',';
		}

		if (hex = ((ptr[0]=='0') && (ptr[1] == 'x')))
			ptr+=2;
	
		item=ptr;

		for (;hex?isdigit(*ptr):isxdigit(*ptr);ptr++);
		if (*ptr && (*ptr!=','))
		{
			fprintf(stderr,TXT(T_NUM_PARSE_ERROR),item);
			errno = EINVAL;
			return(-1);
		}

		if ((*num+1)>max) {
			fprintf(stderr,"%s (%d max)\n",errstring,max);
			return(-1);
		}

		if (*ptr) {
			*ptr = (char) 0x00;
			ptr++;				/* null term item, make ptr point past , */
		}

		if (hex) ltemp = strtoul(item,&endptr,16);
		else	 ltemp = strtoul(item,&endptr,10);

		if (errno || (ltemp>65535)) {
			fprintf(stderr,TXT(T_OUT_OF_RANGE),item);
			errno = EINVAL;
			return(-1);
		}

		table[(*num)++] = ltemp;
	}		
	
	return(0);
}

static void eliminate_escape(char *string)
{
	char *read, *write;

	for (read=string,write=string;*read;read++) {
		if (*read!='\\'||(*(read+1)&&*(read+1)!=',')) {
			*write=*read;
			write++;
		}
	}
	*write=0;
}

static int16_t
parse_patterns (char *table[],
				char *string,
				uint16_t *num,
				bool update_all_not_patterns,
				char *errstring,
				uint16_t max)
{
	char *ptr;
	char *item;
	bool escaped;

	for(ptr=string;*ptr;)
	{
		item = ptr;    	

		for (escaped=0;(*ptr)&&((*ptr!=',')||escaped);ptr++) escaped=(*ptr=='\\');

		if ((*num+1)>max) {
			fprintf(stderr,"%s (%d max)\n",errstring,max);
			return(-1);
		}

		if (*ptr) 
		{
			*ptr = (char) 0x00;
			ptr++;			
			eliminate_escape(item);
			table[(*num)++] = item;
		} else {
			eliminate_escape(item);
			table[(*num)++] = item;
		}				

		if ((update_all_not_patterns)&&(*item!=NOT_CHARACTER))
		{
			all_not_patterns = FALSE;
		}
	}		
	
	return(0);
}


static int parse_modes (mode *table, char *string, uint16_t *num)
{
	char	*ptr;
	char	*temptable[32];
	uint16_t	tempnum = 0;
	int		i,i2,factor;

	/* call parse_patterns(). Clever, huh? */
	if (parse_patterns(temptable,string,&tempnum,FALSE,TXT(T_TOO_MANY_MODES),32)==-1) return(-1);

	/* tempnum now has # of items in temptable,
	   temptable has pointers to the individual mode specifiers */

	if ((*num+tempnum)>Maxmodes) {
		fprintf (stderr,"%s (%d max)\n",TXT(T_TOO_MANY_MODES),Maxmodes);
		return(-1);
	}

	for (i=0;i<tempnum;i++)
	{
		ptr = temptable[i];

		#ifdef DIAG
			fprintf(stderr,"parse_modes: parsing '%s'\n",ptr);
		#endif

		if (*ptr == NOT_CHARACTER)
		{
			ptr++;
			table[i+*num].negative = TRUE;
		} else table[i+*num].negative = FALSE;

		if (!*ptr)
		{
			fprintf(stderr,TXT(T_ILLEGAL_MODE_SPEC),temptable[i]);
			exit(0xBAD);
		}

		/* is it a nonnegative octal number? */
		if (isoctal(*ptr))
		{
			int j;

			j = atoo(ptr);
			if (j<0) 
			{
				fprintf(stderr,TXT(T_NUM_OUT_OF_RANGE),temptable[i]);
				exit(0xBAD);
			} else {     
				#ifdef DIAG
					fprintf(stderr,"parse_modes: got octal 0%o\n",j);
				#endif
				table[i+*num].fmode = j;
			}
		} else {
			/* it is a symbolic type definition */
			for (factor=0;*ptr!='=';ptr++)
			{
				switch(*ptr)
				{
					case 'u':
						factor |= 0100;
						break;
					case 'g':
						factor |= 010;
						break;
					case 'o':
						factor |= 01;
						break;
					case 'a':
						factor = 0111;
						break;
					default:
						fprintf(stderr,TXT(T_ILLEGAL_MODE_SPEC),temptable[i]);
						exit(0xBAD);
				}
			}
			
			/* ptr now points to '=' */
			
			for (++ptr,i2=0;*ptr;ptr++)
			{
				switch(*ptr)
				{
					case 'r':
						i2 |= S_IROTH;
						break;
					case 'w':
						i2 |= S_IWOTH;
						break;
					case 'x':
					case 's':					
						i2 |= S_IXOTH;
						break;			
	        		default:
						fprintf(stderr,TXT(T_ILLEGAL_MODE_SPEC),temptable[i]);
						exit(0xBAD);
				}
			}

			table[i+*num].fmode = i2*factor;
			#ifdef DIAG
				fprintf(stderr,"parse_modes: table[%d].fmode = 0%o\n",i+*num,table[i+*num].fmode);
			#endif
        }  /* else */
	} /* for */

	*num += tempnum;

	return(0);
}
			




#define SKIP_WHITE(ptr)    { for(;(*ptr) && iswhite(*ptr);ptr++);}
#define SKIP_TO_COLON(ptr) { for(;(*ptr) && (*ptr!=':');ptr++);}
#define SKIP_TO_WHITE(ptr) { for(;(*ptr) && !iswhite(*ptr);ptr++);}
#define SKIP_TO_SLASH(ptr) { for(;(*ptr) && (*ptr!='/');ptr++);}
#define BREAK_ON_END(ptr)  { if(!*ptr) break; *(ptr++)=(char)0x00;}

static time_t ascdatetotime(char *string, bool heavy)
{
	bool utc = FALSE;
	
	static char *months[] = {"---", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
							"Aug", "Sep", "Oct", "Nov", "Dec", "---", "---", "---"};

	static unsigned msize[] ={ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	struct tm tm, *tm2;

	time_t lsec;
	char am_pm, flg24;
	char *ptr = string;

	char *day="", *month="", *year="";
	char *hour="",*minute="",*second="";

	/* check - does it start with 'UTC'? */
	if ((!strncmp(ptr,"UTC",3))||(!strncmp(ptr,"utc",3))) {
		utc = TRUE;
		ptr+=3;
	}

	/* change default from 0:0:0 to 59:59:59 if heavy */
	if (heavy) {
		hour = "23";
		minute = second = "59";
	} else {
		hour = minute = second = "0";
	}

	/* "dd/mm/yy hh:mm:ss" */

	do
	{
		SKIP_WHITE(ptr);
		day = ptr;    
	
		SKIP_TO_SLASH(ptr);	
		BREAK_ON_END(ptr);
		month = ptr;
	
		SKIP_TO_SLASH(ptr);
		BREAK_ON_END(ptr);
		year = ptr;
	
		SKIP_TO_WHITE(ptr);
		BREAK_ON_END(ptr);
	
		SKIP_WHITE(ptr);
		minute = second = NULL;
	
		hour = ptr;	
	
		SKIP_TO_COLON(ptr);	
		BREAK_ON_END(ptr);
		minute = ptr;
	
		SKIP_TO_COLON(ptr);
		BREAK_ON_END(ptr);
		second = ptr;
		SKIP_TO_WHITE(ptr);
		BREAK_ON_END(ptr);

		break;	
	} while (FALSE);

	#ifdef TDIAG
		fprintf(stderr,"day = '%s'\n",day);
		fprintf(stderr,"month = '%s'\n",month);
		fprintf(stderr,"year = '%s'\n",year);
		fprintf(stderr,"hour = '%s'\n",hour);
		fprintf(stderr,"minute = '%s'\n",minute);
		fprintf(stderr,"second = '%s'\n",second);
	#endif

	if (!utc) {
		/* learn whether we are in daylight savings time or not */
		lsec = time(NULL);
		tm2 = localtime(&lsec);
		tm.tm_isdst = tm2->tm_isdst;
	} else tm.tm_isdst = 0;
	
	tm.tm_min = 0;
	am_pm = 0;
	flg24 = 1;

	tm.tm_sec = ctod(second);
	if(tm.tm_sec > 59)
		terminate("Time - Invalid second\n");

	tm.tm_min = ctod(minute);
	if(tm.tm_min > 59)
		terminate("Time - Invalid minute\n");

	tm.tm_hour = ctod(hour);
	if(tm.tm_hour > 23)
		terminate("Time - Invalid hour\n");

	if(am_pm  &&  !flg24  &&  tm.tm_hour < 12)
		tm.tm_hour += 12;

	if(tm.tm_hour > 11)
		am_pm = 1;

	tm.tm_year = ctod(year);
	if (tm.tm_year>=0 && tm.tm_year<=38) tm.tm_year+=2000;	// 00-38 is 2000-2038
	if(tm.tm_year >= 1970) 		tm.tm_year -= 1900;
	if(tm.tm_year < 70 ||  tm.tm_year > 155)
		terminate("Date - Invalid year\n");

	if(month[0] >= '0'  &&  month[0] <= '9')
	{
			tm.tm_mon = ctod(month) - 1;
	}
	else
	{
		for(tm.tm_mon = 0; tm.tm_mon < 12; ++tm.tm_mon)
			if(comp(months[tm.tm_mon + 1], month)) break;
	}

	if(tm.tm_mon > 11)
		terminate("Date - Invalid month\n");

	if(tm.tm_mon == 1  &&  (tm.tm_year % 4) == 2)
		msize[2] = 29; /* leap years allow feb 29 */

	tm.tm_mday = ctod(day);
	if(tm.tm_mday > msize[tm.tm_mon + 1]  ||  tm.tm_mday == 0)
		terminate("Date - Invalid day\n");

	lsec = mktime(&tm);

	if (utc) 
#if defined(__CYGWIN32__) || defined(__MINGW32__)
		lsec = lsec - _timezone;
#else
		lsec = lsec - timezone;
#endif

	#ifdef TDIAG
		fprintf(stderr,"lsec = %ld\n",lsec);
	#endif

	return(lsec);
}



static int ctod(char *s)
{
	register int i = 0;
	register char *p = s;

	if (!p) return(0);

	while(*p >= '0' && *p <= '9')
		i = i * 10 + *p++ - '0';

	if(*p != '\0')
		terminate("DATE: Numbers may only contain the digits 0-9.\n      Separate each (hour, min ...) by a space\n");

	return(i);
}

static int comp(char *s1, char *s2)
{
	register char *p1 = s1;
	register char *p2 = s2;

	while(*p1)
		if((*p1++ | ' ') != (*p2++ | ' '))
			return(0);
	return(1);
}
