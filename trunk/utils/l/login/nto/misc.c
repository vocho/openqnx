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





#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <grp.h>
#include "proto.h"

int setusercontext(const struct passwd *pwd, uid_t uid, unsigned int flags) {
    mode_t	mymask = 022;

    /*
	if (flags & LOGIN_SETPATH)
	pathvars[0].def = uid ? _PATH_DEFPATH : _PATH_STDPATH;
	*/

    /* we need a passwd entry to set these */
    if (pwd == NULL)
		flags &= ~(LOGIN_SETGROUP | LOGIN_SETLOGIN);

	/*
	if (setpriority(PRIO_PROCESS, 0, (int)p) != 0)
		syslog(LOG_WARNING, "setpriority '%s' (%s): %m",
		    pwd->pw_name, lc ? lc->lc_class : LOGIN_DEFCLASS);
	*/

    /* Setup the user's group permissions */
    if (flags & LOGIN_SETGROUP) {
		if (setgid(pwd->pw_gid) != 0) {
		    return -1;
		}
		if (initgroups(pwd->pw_name, pwd->pw_gid) == -1) {
		    return -1;
		}
    }

    /* Set the sessions login */
#if !defined(__QNXNTO__)
    if ((flags & LOGIN_SETLOGIN) && setlogin(pwd->pw_name) != 0) {
		return -1;
    }
#endif

    /* This needs to be done after anything that needs root privs */
    if ((flags & LOGIN_SETUSER) && setuid(uid) != 0) {
		return -1;	
    }

    /* Finally, set any umask we've found */
    if (flags & LOGIN_SETUMASK) {
		umask(mymask);
	}
    return 0;
}

void add_env(char *str) {
	char *value;
	char saved = 0;

	//Don't allow changing the SHELL or the PATH dirs
	if (!str || !strcmp(str, "SHELL") || !strcmp(str, "PATH"))
		return;

	if ((value = (char*)strchr(str, '=')) == NULL) {
		value = "1";
	}
	else if (value != str) {
		saved = *value;
		*value++ = '\0';
	}
	else {
		return;
	}

	setenv(str, value, 1);
	if (saved) {
		*--value = saved;
	}
}

#define LINE_LEN 1024
char *save_list(char *fname, int *buflen, int preserve) {
	char *buffer, *head, *entry, *value;
	char line[LINE_LEN];
	int	 size;
	FILE *fp;

	buffer = head = NULL;
	size = 0;
	if (!fname || !(fp = fopen(fname, "r")))
		return(buffer);

	while (fgets(line, LINE_LEN-1, fp)) {
		if (!(entry = strchr(line, '\n'))) {
			fprintf(stderr, "Warning line from [%s] is to long\n", (fname) ? fname : "NULL");
			while (fgets(line, LINE_LEN-1, fp)) {
				if (strchr(line, '\n'))
					break;
			}
			continue;
		}
		*entry = '\0';

		//Determine what the default value is if it exists
		if ((value = strchr(line, '=')) != NULL) {
			value++;
		}

		//Determine the name of the variable 
		entry = line;
		while (*entry && *entry != '=' && *entry != ' ') { entry++; }
		*entry = '\0';

		entry = (char*)getenv(line);

		//We have the env variable, and we are not preserving the system
		if (entry && !preserve) { 
			value = entry;
			entry = line;
		//We have a default value, but no current env variable  
		} else if (value && *value && !entry) {
			entry = line;
		} else {
			value = entry = NULL;
		}

		if (entry && value) {		
			int len;
			len = strlen(entry) + strlen(value) + 2;
			if (!(head = (char*)realloc(head, size + len)))  {
				fclose(fp);
				return(NULL);
			}

			buffer = (size) ? &head[size] : head;
			strcpy(buffer, entry);
			strcat(buffer, "=");
			strcat(buffer, value);
			size += len;
		}
	}
	fclose(fp);
	*buflen = size;
	return(head);
}

#if 0
int main(int argc, char **argv, char **env) {
	int len, indx = 1;
	char *save_env;

	while (indx < argc) {
		printf("Argument %s \n", argv[indx]);
		add_env(argv[indx++]);
	}

	if (!(save_env = save_list(DEFAULT_FILE, &len))) 
		printf("Nothing to save in the environment \n");
	else {
		printf("Length is %d \n", len);
		while (len > 0 && strlen(save_env) != 0) {
			printf("Saving (%d):\n%s\n", strlen(save_env), save_env);
			len -= strlen(save_env);
			save_env += strlen(save_env)+1;
			printf("Len %d \n", len); 
		}
	}

	return(0);
}
#endif
