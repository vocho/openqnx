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





#undef _FILE_OFFSET_BITS

#include <lib/compat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <util/stdutil.h>

#define LF	0x0a
#define CR	0x0d
#define RS	0x1e

char buffer[256];

enum { TUND, TDOS, TQNX4, TQNX2 };

int type = TQNX4, zflag;
int quiet=0;

int
main(int argc, char *argv[])
{
	int opt, i, cnt = 0, x = 0;
	char c;
	FILE *sfp, *dfp;
	struct stat		sbuf;

	while ((opt = getopt(argc, argv, "clqrz")) != -1) {
		switch(opt) {
		case 'q': quiet++; break;

		case 'c':
			type = TDOS;
			++cnt;
			break;

		case 'l':
			type = TQNX4;
			++cnt;
			break;

		case 'r':
			type = TQNX2;
			++cnt;
			break;

		case 'z':
			zflag = 1;
			break;

		default:
			exit(EXIT_FAILURE);
			}
		}

	if(cnt > 1) {
		fprintf(stderr, "textto: You must specify only one conversion type (-clr).\n");
		exit(EXIT_FAILURE);
		}

	if (optind==argc) {
		/* no arguments; reuse argv[0] to store '-' */
		argv[0][0]='-'; argv[0][1]=0;
		optind=0; argc=1;
	}

	for( i = optind ; i < argc ; ++i) {
		char *cur_filename;

		if (argv[i][0]!='-' || argv[i][1]!=0) {
			if((sfp = fopen(argv[i], "r+b")) == 0) {
				fprintf(stderr, "textto: Unable to open %s. Will skip.\n", argv[i]);
				continue;
				}
			cur_filename=argv[i];
		} else {
			/* filename of - specified; use standard input */
			sfp = stdin;
			cur_filename="(stdin)";
		}

		setvbuf(sfp,NULL,_IOFBF,8192);

		if (fstat(fileno(sfp),&sbuf)==-1) {
			fprintf(stderr,"textto: Unable to stat %s. Will skip.\n", cur_filename);
			if (sfp!=stdin) fclose(sfp);
			continue;
		}
	
	#ifndef __NT__
		if (!S_ISREG(sbuf.st_mode) && sfp!=stdin) {
			fprintf(stderr,"textto: Skipping %s %s.\n",
				S_ISCHR(sbuf.st_mode)?"char special":
				S_ISDIR(sbuf.st_mode)?"directory":
				S_ISBLK(sbuf.st_mode)?"blk special":
				S_ISFIFO(sbuf.st_mode)?"fifo":"",
				cur_filename);

			if (sfp!=stdin) fclose(sfp);
			continue;
		}
	#endif

		if (sfp!=stdin) {
			strcpy(buffer,"temp file");
			dfp = tmpfile();
		} else {
			dfp=stdout;
			strcpy(buffer,"stdout");
		}

		if(dfp == NULL) {
			fprintf(stderr, "textto: Unable to open %s. Will skip %s.\n", buffer, cur_filename);
			if (sfp!=stdin) fclose(sfp);
			continue;
		}
		setvbuf(dfp,NULL,_IOFBF,8192);

		if (!quiet && dfp!=stdout) {
			fprintf(stdout,"Converting %s\n", cur_filename);
		}

		errno = 0;

		switch(type) {
			case TDOS:
				while(!feof(sfp)) {
					c = fgetc(sfp) & 0xff;
					if(c == RS  ||  c == LF) {
						if ((x=putc(CR, dfp))==EOF) break;
						if ((x=putc(LF, dfp))==EOF) break;
					} else if(c != CR  &&  (zflag == 0  ||  c != 0x1a)) {
						if ((x=putc(c, dfp))==EOF) break;
					}
				}
				break;

			case TQNX4:
				while(!feof(sfp)) {
					c = fgetc(sfp) & 0xff;
					if(c == RS) {
						if ((x=putc(LF, dfp))==EOF) break;
					} else if(c != CR  &&  (zflag == 0  ||  c != 0x1a)) {
						if ((x=putc(c, dfp))==EOF) break;
					}
				}
				break;

			case TQNX2:
				while(!feof(sfp)) {
					c = fgetc(sfp) & 0xff;
					if(c == LF) {
						if ((x=putc(RS, dfp))==EOF) break;
					} else if(c != CR  &&  (zflag == 0  ||  c != 0x1a)) {
						if ((x=putc(c, dfp))==EOF) break;
					}
				}
				break;

			default:
				fprintf(stderr, "textto: You must specify a conversion type (-clr).\n");
				exit(EXIT_FAILURE);
			}

		if (fflush(dfp)!=0) x=EOF;	/* indicate that a write err occurred */

		/* if no tempfile created (sent to stdout), get next file */
		if ( dfp == stdout ) {	/* only possible if ALWAYS_COPY not set */
			if (x==EOF) fprintf(stderr,"textto: %s %s\n", buffer, strerror(errno));
            continue;
		}			

		if (errno) {
			if (x!=EOF) {
				/* READ error */
				fprintf(stderr,"textto: getc failed (%s): %s\n",cur_filename,strerror(errno));
			} else {
				/* WRITE error */
				fprintf(stderr,"textto: putc failed (%s): %s\n",buffer,strerror(errno));
			}
			fprintf(stderr,"textto: Cannot convert '%s'.\n",cur_filename);

			if (sfp!=stdin) fclose(sfp);
			fclose(dfp);
			continue;	/* proceed to next file */
		}

		rewind(dfp);

		if(sfp != stdin){
			fclose(sfp);
			sfp = fopen(cur_filename, "wb");
		}
		errno = 0;

		while(!feof(dfp)) {
			c = fgetc(dfp) & 0xFF;
			if ((x=putc(c, sfp)==EOF)) break;
		}

		if (fflush(sfp)!=0) x=EOF;

		if (errno) {
			if (x!=EOF) {
				/* READ error */
				fprintf(stderr,"textto: getc failed (%s): %s\n",buffer,strerror(errno));
				fprintf(stderr,"textto: original lost, some data may be left in '%s'.\n",buffer);
			} else {
				/* WRITE error */
				fprintf(stderr,"textto: putc failed (%s): %s\n",argv[i],strerror(errno));
				fprintf(stderr,"textto: original lost.\n");
			}
			fprintf(stderr,"textto: Cannot convert '%s'.\n",argv[i]);
	
			fclose(sfp);
			fclose(dfp);
			exit(1);
		}

		fclose(dfp);
		fclose(sfp);
	}
	return(0);
}
