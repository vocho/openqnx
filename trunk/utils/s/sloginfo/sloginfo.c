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





#ifdef __USAGE
%C	[options]... [filename]

Options:
 -c       Clear log buffer after displaying all waiting events.
 -h       Print unformated entries in hexadecimal (default: decimal).
 -m code  Display events with this major code (default: display all).
 -s 0..7  Display events with this severity or lower (default: 7).
 -w       Waiting for more events to arrive.
 -t       Print time with millisecond resolution
filename  File containing raw events (default: /dev/slog).
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/slog.h>

int Events[4096];
int Clear;
int Hex;
int NumFmts;
int	Major;
int Severity;
int Wait;

struct fmt_entry {
	int		 major;
	int		 minor;
	char	*fmt;
	} *FmtTable;

char *get_fmt(int major, int minor);
void read_fmts(char *path);

int main(int argc, char *argv[]) {
	int		 flags, n, residual, fd, opt, cnt, *evp;
	int Msecs = 0;

    /* Setup Defaults */
	char	*fname = "/dev/slog";
	Major = -1;					// All events
	Severity = _SLOG_SEVMAXVAL;	// All severities
    Hex = Clear = Wait = 0;

	while ((opt = getopt(argc, argv, "tchm:s:w")) != -1) {
		switch(opt) {
		case 'c':
			Clear = 1;
			break;

		case 'h':
			Hex = 1;
			break;

		case 'w':
			Wait = 1;
			break;

    case 't':
      Msecs = 1;
      break;

		case 'm':
			Major = strtoul(optarg, NULL, 0);
			break;

		case 's':
			Severity = atoi(optarg);
			break;

		default:
			exit(EXIT_FAILURE);
		}
	}

	// Read any format files.
	read_fmts("/etc/slog");

	// Get an fd to a slog file or live events
    if (optind < argc)
        
        fname = argv[optind];

    if ((fd = open(fname, O_RDONLY|O_BINARY|O_NONBLOCK)) == -1)
	{
		fprintf(stderr, "Unable to open %s : %s\n", fname, strerror(errno));
		exit(1);
	} 
	residual = 0;
    
	if (!Msecs)
		printf("Time             Sev Major Minor Args\n");
	else
		printf("Time                 Sev Major Minor Args\n");

    for(;;) {
		int lastmajor = -1;

		// Read some events. Don't block.
		if ((n = read(fd, &Events[residual], sizeof(Events) - sizeof(Events[0]) * residual)) == -1)
		{
			if(errno == EAGAIN)
            {
            /* EAGAIN is triggered when all the data has been read from the
               file and the setting is O_NONBLOCK */
				if(Clear)
                {
					unlink(fname);                    /* Delete file */
                }
                if(!Wait)
                {
                    exit(EXIT_SUCCESS);
                }
                /* User specified -w on the command line, so change the
                   properties of fd so that it will now block on the read if
                   no data is available to be read. */

                flags = fcntl(fd, F_GETFL) & ~O_NONBLOCK;
                if (fcntl(fd, F_SETFL, flags ) == -1)  /* Set Flags */
                {
                    fprintf(stderr, "error setting non-blocking flags\n");
                    return EXIT_FAILURE;
                }  
				
                continue;
			}
            else
            {
			    fprintf(stderr, "Unable to read %s - %s\n", fname, strerror(errno));
			    exit(EXIT_FAILURE);
            }
            
		}
		n += residual * sizeof(int);

		// Converts bytes to ints (all events are composed of ints).
		residual = (n /= sizeof(int));
		if(n == 0)
			break;

		for(evp = Events ; evp < &Events[n] ; evp += cnt) {
			int		major, minor, severity, txt;
			time_t	sec;
			char	timebuf[60];
			int msecs;

			major    = _SLOG_GETMAJOR(evp[1]);
			minor    = _SLOG_GETMINOR(evp[1]);
			cnt      = _SLOG_GETCOUNT(evp[0]) + _SLOG_HDRINTS;
			severity = _SLOG_GETSEVERITY(evp[0]);
			txt      = _SLOG_GETTEXT(evp[0]);
			msecs    = ((evp[0] >> 4) & 0x3ff);

			sec = evp[2];
			strftime(timebuf, sizeof(timebuf), "%h %d %T", localtime(&sec));

			// Is the event split across a buffer (only possible from disk).
			if(evp + cnt > &Events[n]) {
				memmove(&Events[0], evp, (residual = &Events[n] - evp) * sizeof(int));
				break;
			}
			residual = 0;

			// Filter based upon major and severity.
			if(severity > Severity)
				continue;

			if(Major != -1  &&  Major != major)
				continue;

			if(major != 0) {
				if(lastmajor == 0)
					printf("\n");
				if (!Msecs)
					printf("%s    %d %5d   %3d ", timebuf, severity, major, minor);
				else
					printf("%s.%03d    %d %5d   %3d ", timebuf, msecs, severity, major, minor);
			}

			if(txt)
				printf("%s", (char *) &evp[_SLOG_HDRINTS]);
			else {
				int i;
				char *cp, *fmt;

				if((cp = fmt = get_fmt(major, minor)))
					if((cp = strchr(fmt, '%')))
						cp = strchr(cp+1, '%');

				for(i = _SLOG_HDRINTS ; i < cnt ; ++i) {
					if(cp) *cp = '\0';
					printf(fmt ? fmt : (Hex ? "%x " : "%d "), evp[i]);
					fmt = cp;
					if(cp) {
						*cp = '%';
						cp = strchr(cp + 1, '%');
					}
				}
			}

			if(major != 0)
				printf("\n");

			lastmajor = major;
		}

		fflush(stdout);

	}

	exit(EXIT_SUCCESS);
}


//
// Rather than print slogb and slogi events as a bunch of ints
// you can create a format file which consists of lines of the form.
//
// major minor format
//
// For example. Major code 12 and minor code 0 which contains a
// single int as a pid.
//
// 12 0 Pid %d
//
void read_fmts(char *path) {
	DIR					*dir;
	struct dirent		*dent;
	FILE				*fp;
	char				line[500], *end;
	struct fmt_entry	*tmp;
	int					fmt_table_size = 0, n;

	chdir(path);
	if((dir = opendir(path))) {
		while((dent = readdir(dir))) {
			if(strstr(dent->d_name, ".fmt")==NULL || (fp=fopen(dent->d_name, "r")) == NULL)
				continue;

			while(fgets(line, sizeof(line), fp)) {
				// Remove trailing newline.
				n = strlen(line);
				if(n > 0 && line[n-1] == '\n')
					line[n-1] = '\0';

				// Grow format table dynamically as we load formats
				if(NumFmts >= fmt_table_size) {
					if((tmp = realloc(FmtTable, NumFmts + 256)) == NULL) {
						fclose(fp);
						break;
					}

					FmtTable = tmp;
					fmt_table_size = NumFmts + 256;
				}

				FmtTable[NumFmts].major = strtol(line, &end, 10);
				FmtTable[NumFmts].minor = strtol(end, &end, 10);
				FmtTable[NumFmts].fmt = strdup(end + 1);
				++NumFmts;
			}

			fclose(fp);
		}

		closedir(dir);
	}
}


//
// Scan for a matching format for none next messages.
//
char *get_fmt(int major, int minor) {
//	static char	fmt[150];
	int			i;

	// Try and match an existing format.
	if(FmtTable)
		for(i = 0 ; i < NumFmts ; ++i)
			if(major == FmtTable[i].major && minor == FmtTable[i].minor) {
//				strcpy(fmt, FmtTable[i].fmt);
//				return(fmt);
				return(FmtTable[i].fmt);
				}

	return(NULL);
	}
