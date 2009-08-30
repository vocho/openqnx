/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. All Rights Reserved.
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

#include <stdio.h>
#include <string.h>
#include "struct.h"

void fixenviron(char *line, int size){
	char *p;
	int offset = 0;
	char* startIndex;
	char mangle[256];
	int i = 0;
	char *val;
	int brace;
	int csize = strlen(line);
	p = strchr(line, '$');
	while(p != NULL){
		/* Replace the $ with a %, and add a % to the other side */
		csize += 1;

		brace = 0;
		/* We don't have enough space to go */
		if (csize >= size)
			return;
		p++;
		startIndex = p;

		/* Look for the end */
		while(isalnum(p[0]) || p[0] == '_' || p[0] == '{'){
			/* If we find a brace in the first position, this is a braced */
			if (p[0] == '{' && p - startIndex == 0){
				brace = 1;
			}
			p++;
		}
		
		/* Copy the string over *
		 * Use the Brace flag to help with offsets */
		strncpy(mangle, startIndex + brace, p - startIndex - brace);

		/* Terminate */
		mangle[p - startIndex - brace] = 0;

		val = getenv(mangle);
		if (val){
		

			/* If the brace is present, it's actually a string shrink */
			if (brace){
				/* If braced, we need to move over one */
				for (i = startIndex - line + 1; i < p - line; i++){
					line[i-1] = line[i];
				}
				for(i = p - line; i < strlen(line); i++){
					line[i] = line[i+1];
				}
				p--;

				/* We actually have more space */
				csize -= 2;
			}else{
				/* Move the last bit forward two spaces */
				for(i = strlen(line) + 1; i >= p - line + 1; i--){
					line[i] = line[i - 1];
				}
			}

			/* Replace the $ with %*/
			line[startIndex - line - 1] = '%';

			/* Add a % to the end */
			p[0] = '%';

		}
		/* Onwards */
		p = strchr(p, '$');
	}
}

