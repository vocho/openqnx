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


#include <unistd.h>		// determines whether _POSIX_SPAWN is defined or not
#ifdef _POSIX_SPAWN

#include <spawn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>


static char *find_envvar(char *const envp[], char *const envvar);
static int parse_envvar_str(char *str, char *token, char ***value);

/*
 * posix_spawnp
 * 
 * the only difference between posix_spawnp() and posix_spawn() is that
 * posix_spawnp() accepts the name of an executable without its full path
 * having been specified. Therefore, all this function needs to do is to locate
 * <file> and then prefix it with its location (as required). posix_spawn() can
 * then be called to do the work.
 * 
 * Returns:
 * 		ENOENT	- the <file> argument could not be found in any of the 'PATH'
 * 				  environment variable locations
 * 
 * See posix_spawn() for additional errors that can be returned
*/

/*
 * for our 2.95.3 compiler (and C++) _Restrict expands to __restrict however
 * 2.95.3 compiler (and C++) does not like argv[__restrict] and envp[__restrict].
 * Once we no longer support 2.95.3 compiler, this can be reduced to __cplusplus
 * (see spawn.h and posix_spawn.c also)
*/
int posix_spawnp(pid_t *_Restrict pid,
				 const char *_Restrict file,
				 const posix_spawn_file_actions_t *file_actions,
				 const posix_spawnattr_t *_Restrict attrp,
#if (__GNUC__ == 2 && __GNUC_MINOR__ >= 9) || defined(__cplusplus)
				 char *const argv[],
				 char *const envp[])
#else	/* (__GNUC__ == 2 && __GNUC_MINOR__ >= 9) || defined(__cplusplus) */
				 char *const argv[_Restrict],
				 char *const envp[_Restrict])
#endif	/* (__GNUC__ == 2 && __GNUC_MINOR__ >= 9) || defined(__cplusplus) */		 
{
	if ((file == NULL) || (*file == '\0')) return EINVAL;
	if (*file == '/') {
		/* full path already specified */
		return posix_spawn(pid, file, file_actions, attrp, argv, envp);
	} else {
		int num_paths;
		int r = -1;
		unsigned i;
		char *envvar_str;
		char **env_path;
		char path[PATH_MAX+1];

		/* lets search for <file> using the 'PATH' environment variable of the caller */
		if ((envvar_str = find_envvar(environ, "PATH")) == NULL) return ENOENT;
		/* make a copy of the environment variable string so the original is not destroyed */
		if ((envvar_str = strdup(envvar_str)) == NULL) return ENOMEM;
		
		if ((num_paths = parse_envvar_str(envvar_str, ":", &env_path)) <= 0) {
			free(envvar_str);
			return ENOENT;
		}
		/* lets try each of the path variables */
		for (i=0; i<num_paths; i++)
		{
			struct stat dummy;
			if (strcmp(env_path[i], "./") == 0) {
				char *b;
				int trys = 5;
				/* try 5 times to get the 'cwd' on an EAGAIN, otherwise try next env_path[i] */
				while ((--trys >= 0) && ((b = getcwd(path, sizeof(path))) == NULL) && ((r = errno) == EAGAIN))
				{};
				if (b == NULL) {
					if (r == EAGAIN) break;	/* too many EAGAIN's, give up */
					continue;	/* try next env_path[i] */
				}
				(void)strlcat(path, "/", sizeof(path));
				(void)strlcat(path, file, sizeof(path));
			} else {
				snprintf(path, sizeof(path)-1, "%s/%s", env_path[i], file);
			}
			if (stat(path, &dummy) == 0) {
				r = posix_spawn(pid, path, file_actions, attrp, argv, envp);
				/*
				 * as per POSIX, chap 8 "Environment Variables" description for PATH ...
				 * 	   "The list shall be searched from beginning to end, applying the
				 * 		filename to each prefix, until an executable file with the specified
				 * 		name and appropriate execution permissions is found."
			    */
				if (r != EACCES) break;	// if EACCES, continue search
			} else {
				r = errno;	/* get the stat() errno */
			}
		}
		free(env_path);
		free(envvar_str);
		return (r != -1) ? r : ENOENT;
	}
}

/*
 * Given an environment pointer <envp>, search for the environment variable
 * <envvar> and if found, return a pointer to it. If not found, return NULL
 * 
 * ex. If looking for <envvar> "PATH", and "PATH=./:/foo", then a pointer to
 * 		"./:/foo" will be returned
*/
static char *find_envvar(char *const envp[], char *const envvar)
{
	while (*envp != NULL)
	{
		char *s1 = *envp;
		unsigned len = strlen(envvar);
		if ((strncmp(s1, envvar, len) == 0) && (*(s1+=len) == '=')) {
			return ++s1;
		}
		++envp;
	}
	return NULL;
}

/*
 * Given an string <str>, parse its values into individual strings. Space for
 * the strings is dynamically allocated and the string pointers are placed into
 * the <value> array. <str> is modified by substituting <token> for '\0'
 * The number of individual string tokens found for <str> is returned.
 * 
 * If >0 is returned, the caller is repsonisble for freeing <value>.
 * If <=0 is returned, no memory will have been allocated
*/
static int parse_envvar_str(char *str, char *token, char ***value)
{
	unsigned n = 0;
	char *saveptr;
	char *s = strtok_r(str, token, &saveptr);

	*value = NULL;
	while (s != 0)
	{
		*value = realloc(*value, (n+1) * sizeof(**value));
		(*value)[n++] = s;
		s = strtok_r(NULL, token, &saveptr);
	}
	return n;
}


__SRCVERSION("posix_spawnp.c $Rev: 211778 $");

#endif	/* _POSIX_SPAWN */
