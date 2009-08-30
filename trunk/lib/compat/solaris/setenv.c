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
#include <strings.h>
#include <stdlib.h>

typedef struct envlist{
	char *var;
	char *val;
	struct envlist *next;
} envlist_t;

envlist_t *GlobalEnvListHead = NULL;

envlist_t *check_list(const char *var)
{
	envlist_t *tmp = GlobalEnvListHead;
	while(tmp)
		if(!strcmp(tmp->var, var))
			break;
		else
			tmp = tmp->next;
	return(tmp);
}

envlist_t *add_list(const char *var, const char *val)
{
	envlist_t *tmp;
	tmp = (envlist_t*)malloc(sizeof(envlist_t));
	if(!tmp) {
		perror("setenv failed");
		return(0);
	}
	if(!(tmp->var = strdup(val))){
		perror ("setenv failed");
		free(tmp->var);
		free(tmp);
		return(0);
	}
	tmp->val = (char*)malloc(strlen(var)+strlen(val)+2); //room for "%s=%s" + NULL
	if(!tmp->val) {
		free(tmp->var);
		free(tmp->val);
		free(tmp);
		perror("setenv failed");
		return(0);
	}
	sprintf(tmp->val,"%s=%s", var, val);

	tmp->next = GlobalEnvListHead;
	GlobalEnvListHead = tmp;
	return(GlobalEnvListHead);
}

int
setenv(const char *var, const char *val, int overwrite) {
	envlist_t *cur;

	cur = check_list(var);
	if(!cur)
	{
		if (getenv(var) && !overwrite)
			cur = add_list(var, getenv(var));
		else
			cur = add_list(var, val);
		if(!cur)
			return(-1);
	}
	else if(overwrite)
	{
		free(cur->val);
		cur->val = (char*)malloc(strlen(var)+strlen(val)+2);
		sprintf(cur->val,"%s=%s", var, val); 
	}
	
    if (overwrite || !getenv(var)) 
		putenv(cur->val);

	return(0);
}
