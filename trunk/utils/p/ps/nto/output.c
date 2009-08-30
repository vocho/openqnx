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





#include "output.h"
#include "filter.h"	// For fullListing

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

// this enum identifies each field name and corresponds to 'varNames', 'defaultFieldName'
// and 'defaultFieldWidth'
enum FieldNames
{
	RUSER,
	USER,
	RGROUP,
	GROUP,
	PID,
	PPID,
	PGID,
	PCPU,
	VSZ,
	NICE,
	ETIME,
	TIME,
	TTY,
	COMM,
	ARGS,	
			// the fields below are not required for POSIX-compliance
	TTY_TTY,
	UID,
	ENV,
	PFLAGS,
	PRI,
	SID,
	SUID,
	SGID,
	UMASK,
	SIGIGN,
	SIGPEND,
	SIGQUEUE,
	THREADS,
	STIME,
	CMD,
	SZ,
			// tid must be the first thread-specific field in the lists so that the code
			// can determine whether thread fields are being displayed or not
	TID,
	TFLAGS,
	TSIGBLK,
	CPU,
	STATE,
	WCHAN,
	ADDR,
	PSCHED,
	DFLAGS,

	MAX_FIELD_COUNT = DFLAGS + 2
};

// varNames stores the names of the fields as they should appear on the command line when the
// user is specifying their desired layout or new headers
char *varNames[]=
{
	"ruser",
	"user",
	"rgroup",
	"group",
	"pid",
	"ppid",
	"pgid",
	"pcpu",
	"vsz",
	"nice",
	"etime",
	"time",
	"tty",
	"comm",
	"args",

	"tty_tty",
	"uid",
	"env",
	"f",
	"pri",
	"sid",
	"suid",
	"sgid",
	"umask",
	"sigign",
	"sigpend",
	"sigqueue",
	"threads",
	"stime",
	"cmd",
	"sz",

	"tid",
	"tflags",
	"tsigblk",
	"cpu",
	"s",
	"wchan",
	"addr",
	"c",
	"dflags"
};

// defaultFieldName stores the default header for each field 
char *defaultFieldName[]=
{
	"RUSER",
	"USER",
	"RGROUP",
	"GROUP",
	"PID",
	"PPID",
	"PGID",
	"%CPU",
	"VSZ",
	"NI",
	"ELAPSED",
	"TIME",
	"TT",
	"COMMAND",
	"COMMAND",

	"TTY",
	"UID",
	"ENV",
	"F",
	"PRI",
	"SID",
	"SUID",
	"SGID",
	"UMASK",
	"SIGNALS IGNORED",
	"SIGNALS PENDING",
	"SIGNALS QUEUED",
	"THREADS",
	"STIME",
	"CMD",
	"SZ",

	"TID",
	"TFLAGS",
	"SIGNALS BLOCKED",
	"CPU",
	"S",
	"WCHAN",
	"ADDR",
	"C",
	"DFLAGS"
};

// defaultFieldWidth stores the default width of each field; a negative value indicates that
// the field should be right-justified; each value should be the max of the length of the header
// and the length of the typical value
int defaultFieldWidth[] =
{
	-8,
	-8,
	-8,
	-8,
	10,
	10,
	10,
	4,
	5,
	3,
	11,
	11,
	-5,
	-16,
	-16,

	// fields not required for POSIX-compliance
	-5,
	5,
	-16,
	8,
	3,
	10,
	6,
	6,
	-17,
	16,
	16,
	16,
	7,
	5,
	-16,
	5,

	// thread information
	6,
	8,
	16,
	3,
	1,
	-5,
	4,
	2,
	8
};

int 	fieldCount = 0,			// how many fields to display
	fieldWidth[MAX_FIELD_COUNT],	// the width of each field (a negative value
					//    means the field is right-justified)
	headerFormat[MAX_FIELD_COUNT]; 	// the value to display for each field
char 	*fieldName[MAX_FIELD_COUNT];	// the header for each field
int	usingThreads = 0;		// is the user displaying thread information?
int	needArgs = 0;
int	needEnv = 0;
int	needThread = 0;
int columns = 0;

char *format_umask (int umask, char *buffer)
// this function formats a umask for display (so that it looks the same
// as the output as it should appear when the user runs 'umask -S')
{
	char *b = buffer;
	int i, u;
#if 1
	int s[3] = {'u', 'g', 'o'};

	// the umask represents the permissions that are
	// turned off
	umask = ~umask;	
	for (i = 0; i < 3; i++)
	{	// iter through each group of 3 bits,
		// starting at the left-most bits
		u = (umask >> ((2 - i) * 3)) & 0x7;	

		*buffer++ = s[i];
		*buffer++ = '=';
		if (u & 0x04) *buffer++ = 'r';
		if (u & 0x02) *buffer++ = 'w';
		if (u & 0x01) *buffer++ = 'x';

		// we should terminate the end of the string
		// with a '\0', not a ','.
		*buffer++ = (i == 2) ? '\0' : ',';
	}
	return b;
#else
	// this code generates the "ls" style of permissions
	// build up the umask in reverse
	buffer[9] = '\0';
	buffer += 6;
	for (i = 0; i < 3; i++)
	{
		u = umask & 0x7;
		umask >>= 3;
		buffer[0] = (u & 0x04) ? 'r' : '-';
		buffer[1] = (u & 0x02) ? 'w' : '-';
		buffer[2] = (u & 0x01) ? 'x' : '-';
		buffer -= 3;
	}
	return buffer + 3;
#endif
}

char *format_size (int i, char *buffer)
// this function formats a size for display
{
	char *suffix;

#if 0
	i /= 1024; // display the size in K
	suffix = "K";
#else
/*	this code picks the best scale with which to 
	represent the number without losing precision
*/
	if ((i < 1024) || (i % 1024))
		suffix = "";
	else if (((i /= 1024) < 1024) || (i % 1024))
		suffix = "K";
	else if (((i /= 1024) < 1024) || (i % 1024))
		suffix = "M";
	else if (((i /= 1024) < 1024) || (i % 1024))
		suffix = "G";
	else
	{
		i /= 1024;
		suffix = "T";
	}
#endif

	itoa (i, buffer, 10);
	strcat (buffer, suffix);
	return buffer;
}

char *format_signal (sigset_t *s, char *buffer)
// this function formats signal masks for display
{
	unsigned int sig0, sig1;

	sig0 = ((unsigned int *) s)[0];
	sig1 = ((unsigned int *) s)[1];
	sprintf (buffer, "%08x%08x", sig0, sig1);
	return buffer;
}

char *format_etime (time_t t, char *buffer)
// this function formats elapsed time for display
{
	*buffer = '\0';
	if(t > 60 * 60) {
		int			hours;

		if(t > 60 * 60 * 24) {
			int			days;

			days = t / (60 * 60 * 24);
			t = t - days * (60 * 60 * 24);
			buffer += sprintf(buffer, "%2d-", days);
		}

		hours = t / (60 * 60);
		t = t - hours * (60 * 60);
		buffer += sprintf(buffer, "%02d:", hours);
	}
	sprintf(buffer, "%02d:%02d", t / 60, t % 60);

	return buffer;
}

char *format_time (time_t t, char *buffer)
// this function formats start time for display
{
	int			hours;

	*buffer = '\0';
	if(t > 60 * 60 * 24) {
		int			days;

		days = t / (60 * 60 * 24);
		t = t - days * (60 * 60 * 24);
		buffer += sprintf(buffer, "%2d-", days);
	}

	hours = t / (60 * 60);
	t = t - hours * (60 * 60);
	sprintf(buffer, "%02d:%02d:%02d", hours, t / 60, t % 60);
	return buffer;
}

char *format_stime (time_t t, char *buffer)
// this function formats start time for display
{
	struct tm	*tm, buff;

	if(!(tm = localtime_r(&t, &buff)))
		return strcpy(buffer, "-");

	if(t - time(0) > 24 * 60 * 60)
		strftime(buffer, 10, "%b%d", tm);
	else
		strftime(buffer, 10, "%R", tm);
	return buffer;
}

void
print_ps (struct _ps *psp)
// this function displays the process (and thread) information according the
// layout
{
	char *field;
	char buffer[100];
	int width = 0;
	int i, len;

	for (i = 0; i < fieldCount; i++)
	{
		if(i) {
			putchar(' ');
			width++;
		}
		switch (headerFormat[i])
		{
		case UID:
		case RUSER:
			field = itoa (psp->ruser, buffer, 10);
			break;
		case USER:
			field = itoa (psp->user, buffer, 10);
			break;
		case RGROUP:
			field = itoa (psp->rgroup, buffer, 10);
			break;
		case GROUP:
			field = itoa (psp->group, buffer, 10);
			break;
		case PID:
			field = itoa (psp->pid, buffer, 10);
			break;
		case PPID:
			field = itoa (psp->ppid, buffer, 10);
			break;
		case PGID:
			field = itoa (psp->pgid, buffer, 10);
			break;
		case PCPU:
			field = itoa (psp->pcpu, buffer, 10);
			break;
		case VSZ:
			field = format_size (psp->vsz, buffer);
			break;
		case SZ:
			field = format_size (psp->sz, buffer);
			break;
		case NICE:
			field = "0";
			break;
		case ETIME:
			field = format_etime (psp->etime, buffer);
			break;
		case TIME:
			field = format_time (psp->time, buffer);
			break;
		case STIME:
			field = format_stime (psp->stime, buffer);
			break;
		case TTY_TTY:
		case TTY:
			//todo: format tty
//todo:			field = itoa (psp->tty, buffer, 10);
//todo:			sprintf (buffer, "/dev/ttyp%d", psp->tty);
//todo:			field = buffer;
			field = "?";
			break;
		case CMD:
			field = fullListing ? psp->args : psp->comm;
			break;
		case COMM:
			field = psp->comm;
			break;
		case ARGS:
			field = psp->args;
			break;
		case ENV:
			field = psp->env;
			break;
		case PFLAGS:
			sprintf (buffer, "%08x", psp->pflags);
			field = buffer;
			break;
		case PRI:
			field = itoa (psp->pri, buffer, 10);
			break;
		case SID:
			field = itoa (psp->sid, buffer, 10);
			break;
		case SUID:
			field = itoa (psp->suid, buffer, 10);
			break;
		case SGID:
			field = itoa (psp->sgid, buffer, 10);
			break;
		case UMASK:
			field = format_umask (psp->umask, buffer);
			break;
		case SIGIGN:
			field = format_signal (&psp->sigign, buffer);
			break;
		case SIGPEND:
		{
			unsigned int sig0, sig1;

			sig0 = ((unsigned int *) &psp->sigpend)[0];
			sig0 |= ((unsigned int *) &psp->tsigpend)[0];
			sig1 = ((unsigned int *) &psp->sigpend)[1];
			sig1 |= ((unsigned int *) &psp->tsigpend)[1];

			sprintf (buffer, "%08x%08x", sig0, sig1);
			field = buffer;
			break;
		}
		case SIGQUEUE:			
			field = format_signal (&psp->sigqueue, buffer);
			break;
		case THREADS:
			field = itoa (psp->threads, buffer, 10);
			break;
		case TID:
			field = itoa (psp->tid, buffer, 10);		
			break;
		case TFLAGS:
			sprintf (buffer, "0x%x", psp->tflags);
			field = buffer;
			break;
		case TSIGBLK:
			field = format_signal (&psp->tsigblk, buffer);
			break;
		case CPU:
			field = itoa (psp->cpu, buffer, 10);
			break;
		case DFLAGS:
			sprintf (buffer, "0x%x", psp->dflags);
			field = buffer;
			break;
		case STATE:
			field = "-";
			break;
		case WCHAN:
			field = "-";
			break;
		case ADDR:
			field = "-";
			break;
		case PSCHED:
			field = "-";
			break;
		default:
			fprintf (stderr, "Internal Error: file: %s, line %d\n",
				 __FILE__, __LINE__);
			return;
		}
		// don't truncate how long each field is - the default values are
		// long enough to keep the table aligned. the args and comm values
		// may be the only ones long enough to ruin the alignment of the
		// output but thats ok
		len = 0;
		if(i + 1 != fieldCount || fieldWidth[i] > 0) {
			len = printf ("%*s", fieldWidth[i], field);
		} else {
			int	size;

			size = columns - (width + strlen(field) + 1);
			if(columns && size < 0)
				len = printf ("%.*s", columns - width - 1, field);
			else
				len = printf ("%s", field);
		}
		width += len;
	}
	printf ("\n");	
}

int
find_fieldid (char *s)
// this function finds the enum FieldNames of a field given its field name
// and returns -1 if it can not be found
{
        int i, field = -1;

        for (i = 0; i < sizeof (varNames) / sizeof (varNames[0]); i++)
                if (strnicmp (varNames[i], s, strlen (varNames[i])) == 0)
                        if (field == -1 || strlen (varNames[i]) > strlen (varNames[field]))
                                field = i;
        return field;
}

int
parse_header (char *header)
// this function parses the layout parameter to the '-o' option and converts it
// to an internal format
{
	int fieldid;
	int l;
	char delim;

	while (1)
	{
		// we can only display so many fields
		if (fieldCount == MAX_FIELD_COUNT)
			return -1;

		// skip over white space
		header += strspn (header, " \t\r\n\b ,");
		if (*header == '\0')
			return 0;

		// find the field name and quit if the name is invalid
		fieldid = find_fieldid (header);
		if (fieldid == -1)
			return -1;

		// store the field name in our layout 
		headerFormat[fieldCount] = fieldid;

		switch(fieldid) {
		case COMM:
		case ARGS:
		case CMD:
			needArgs = 1;
			break;
		case ENV:
			needEnv = 1;
			break;
		case TID:
		case TFLAGS:
		case TSIGBLK:
		case CPU:
		case STATE:
		case WCHAN:
		case ADDR:
		case PSCHED:
		case DFLAGS:
			needThread = 1;
			break;
		default:
			break;
		}

		// if the field contains thread information, make sure we
		// remember to iter through all the threads in a process
		if (usingThreads == 0)
			usingThreads = (fieldid >= TID);

		// does the user want to use a new header?
		l = strlen (varNames[fieldid]);
		delim = header[l];
		if (delim == '=') 
		{	// yup - the user wants to override the header
			fieldName[fieldCount] = header + l + 1;

			// find the new length of the field
			// anything right-justified should stay right justified
			if (fieldWidth[fieldCount] < 0)
				fieldWidth[fieldCount] = 
					-max (strlen (fieldName[fieldCount]), 
						-defaultFieldWidth[fieldid]);
			else
				fieldWidth[fieldCount] = 
					max (strlen (fieldName[fieldCount]), 
						defaultFieldWidth[fieldid]);
			fieldCount++;
			break;	// the header is the rest of the argument so
				// we are done
		}
		else if ((delim == ' ') || (delim == ',') || (delim == '\0'))
		{
			// the user didn't override the header so use the
			// default values
			fieldName[fieldCount] = defaultFieldName[fieldid];
			fieldWidth[fieldCount] = defaultFieldWidth[fieldid];
			fieldCount++;
		
			// if there was no delimiter (ie. we reached the end of the
			// string), then we are done
			if (delim == '\0')
				break;
			header += l + 1;
		}
		else
			return -1; // the user tried to use an invalid delimiter

	}
	return 0; // everything was a-ok
}

void
print_header ()
{
	int i, width = 0;

	// see if all the field names are blank
	for (i = 0; i < fieldCount; i++)
		if (*fieldName[i] != '\0') // found a non-blank field name
			break;

	if (i == fieldCount)
		return;	// yes, so don't print a header line

	for (i = 0; i < fieldCount; i++) {
		if(i) {
			putchar(' ');
			width++;
		}
		if(i + 1 != fieldCount || fieldWidth[i] > 0)
			printf ("%*s", fieldWidth[i], fieldName[i]);
		else
			printf ("%s", fieldName[i]);
		width += abs(fieldWidth[i]);
	}
	printf ("\n");

	// If fields are too wide, turn off column length checking.
	if(width > columns) {
		columns = 0;
	}
}

