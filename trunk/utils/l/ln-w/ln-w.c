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





#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <lib/compat.h>
#include <process.h>

char cp_host[]="cp";
char cp_host_w_args[]="cp -vf";

void fullpath(char *, char *);
void basename_nd(char *path, char *buf);

int symbolic;

int
main (int argc, char *argv[]) {
	int i, ret;
	char *targ_file = NULL, *link_name = NULL;
	char *path;
	
	if (argc < 2 || argc > 4) {
		fprintf(stderr, "Usage: ln-w [-s] source [destination]\n");
		exit(1);
	}
	for(i = 1 ; i < argc ; i++){
		if(strcmp(argv[i], "-s") == 0){
			symbolic = 1;
			continue;
		}
		if(targ_file)
			link_name = argv[i];
		else
			targ_file = argv[i];
	}

	if(!link_name){
		/* If no link name, use targ_file as link name. */
		link_name = strdup(targ_file);
		basename_nd(targ_file, link_name);
	}

        path = malloc(strlen(link_name)+strlen(targ_file)+2);
	if(symbolic && !IS_ABSPATH(targ_file)){
		fullpath(link_name, path);
		strcat(path, targ_file);
		targ_file = path;
	}

	argv[0]=cp_host_w_args;
	argv[1]=targ_file;
	argv[2]=link_name;
	argv[3]=0;
	printf("%s %s %s \n", argv[0], targ_file, link_name);

	ret = spawnvpe(P_WAIT, cp_host, argv, NULL);

	return ret;
}

void
basename_nd(char *path, char *buf){
	int i = strlen(path) - 1;
	int end;

	/* Discard trailing dirseps. */
	while(i && IS_DIRSEP(path[i]))
		i--;
	end = i+1;
	
	while(i && !IS_DIRSEP(path[i]))
		i--;

	if(IS_DIRSEP(path[i]))
		i++;

	strncpy(buf, path + i, end - i);
	buf[end - i] = '\0';
}

void
fullpath(char *src, char *dest) {
	int i;
	strcpy(dest, src);
	for (i=strlen(src)-1;i>=0;i--) {
		if (IS_DIRSEP(src[i])) break;
	}
	if (i!=(strlen(src)-1))
		dest[i+1]='\0';
}
