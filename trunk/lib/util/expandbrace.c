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



#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <expandbrace.h>

/* these should be defined extern by anything that wants to change these */
char path_char='@', del_char='`', query_char='?';

/*
	expandbrace(char *dest,char *orig, char *string, unsigned char flags)

	each @ in orig is replaced with string
	each ` causes the character in the destination preceding it to be deleted
       (like a backspace)
	each {[n[,m]]} is replaced with string, less n characters from strings's
       end, and the first m chars of string. If not specified, n and m are 0.
*/

void expandbrace(char *dest, char *orig, char *string, unsigned char flags)
{
	int end=strlen(string);
	char *fulldest = dest;
	char *origorig = orig;

	for (;;) {
		if (flags&REPLACE_AT) {
			if (*orig == path_char) {
				memcpy(dest,string,end);
				dest+=end;
				orig++;
				continue;
			} else if (*orig == del_char) {
				if (dest!=fulldest) dest--;
				orig++;
				continue;
			}
		}

		if (!(flags&REPLACE_BRACES)) {
			*dest=*orig;
			dest++; orig++;
			continue;
		}

		switch(*dest=*orig) {
			case 0:
				return;
			case '\\':
				*dest = *++orig;
				if (!*dest) return;	/* string ended with \ */
			break;

			case '{':
				{
					char *scanstart = orig;
					long num1=0,num2=0,numchars=0;

					if (flags&QUOTE) *dest++='\'';

					*dest = 0;

					scanstart++;

					if (*scanstart!='}' && *scanstart!=',') {
						errno=0;
		        		num2 = strtol(scanstart,&scanstart,0);
						if (errno) {
							perror(orig);
							fprintf(stderr,"Illegal {} construct (bad numerical value) near \"%s\" (%s)\n",orig,origorig);
							exit(EXIT_FAILURE);
						}
					}
	
					switch (*scanstart) {
						case ',':
							scanstart++;
                            errno=0;
							num1 = strtol(scanstart,&scanstart,0);
							if (errno) {
								perror(orig);
								fprintf(stderr,"Illegal {} construct (bad numerical value) near \"%s\" (%s)\n",orig,origorig);
								exit(EXIT_FAILURE);
							}
	
							if (*scanstart!='}') {
								fprintf(stderr,"Illegal {} construct at '%c' in \"%s\" (%s)\n",*scanstart,orig,origorig);
								exit(EXIT_FAILURE);
							} 
						break;

						case '}': break;

						default:							
							/* syntax error */
							fprintf(stderr,"Illegal {} construct at '%c' in \"%s\" (%s)\n",*scanstart,orig,origorig);
							exit(EXIT_FAILURE);
						break;
					}
					/* num2 contains # chars from end to stop 0= go
	                   to end of string. num1 contains # chars to skip
	                   at beginning */
	
	                numchars = end-num2-num1;
					if (numchars<0) numchars = 0;

					orig = scanstart;

					{
						char *read=string+(int)num1;
						int  numtoread=numchars;
			
						while (numtoread--) {
							switch(*read) {
								case '\'':
									if (flags&QUOTE) {
										/* 'blah'\''blah' */
								        *dest++='\'';	/* end quote */
								        *dest++='\\';	/* escape the literal quote */
								        *dest++='\'';	/* literal quote */
								        *dest++='\'';	/* start quote */
										read++;
									} else *dest++=*read++;
									break;

								default:
									*dest++=*read++;
							}
						}				
					}

					if (flags&QUOTE) *dest++='\'';
					*dest = 0;
					dest--;
					
				} /* for scope of auto vars */
			break;
		}
		dest++;
		orig++;
	}
}


#ifdef TESTPROGRAM

main()
{
	char buffer[256];
	char orig[128];
	char replacement[64];
	char *ptr;

	while (1) {
		fgets(orig,sizeof(orig),stdin);
		fgets(replacement,sizeof(replacement),stdin);
		if (ptr=strrchr(orig,'\n')) *ptr = 0;
		if (ptr=strrchr(replacement,'\n')) *ptr = 0;

		fprintf(stdout,"orig=\"%s\" repl=\"%s\"\n",orig,replacement);
		expandbrace(buffer,orig,replacement,REPLACE_BRACES|REPLACE_AT);
		fprintf(stdout,"%s\n",buffer);
	}

}

#endif
