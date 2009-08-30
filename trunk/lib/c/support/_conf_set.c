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
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <sys/conf.h>

int _conf_set(long **plist, int cmd, int name, long value, const char *str) {
	long				*list = *plist;
	int					pos = 0;

	if((list = *plist)) {
		long				item;

		while((item = *list++) != _CONF_END) {
			int					match;

			switch(item & _CONF_CMD_MASK) {
			case _CONF_VALUE:

				match = item & _CONF_NAME_MASK;
				if(match == _CONF_NAME_LONG) {
					match = *list++;
				}
				if(match == name) {
					if(str && (item & _CONF_STR)) {
						char				*p;

						if(!(p = realloc((char **)*list, value + 1))) {
							return ENOMEM;
						}
						strncpy(p, str, value);
						p[value] = '\0';
						*(char **)list = p;
						return EOK;
					}
					if(!str && (item & _CONF_NUM)) {
						*list = value;
						return EOK;
					}
				}
				/* Fall through */
			case _CONF_CALL:
			case _CONF_LINK:
				list++;
				break;
			default:
				break;
			}
		}
		pos = (list - *plist) - 1;
	}

	if(str) {
		char				*s;

		if(!(cmd & _CONF_STR)) {
			return EINVAL;
		}
		if(!(s = malloc(value + 1))) {
			return ENOMEM;
		}
		strncpy(s, str, value);
		s[value] = '\0';
		value = (intptr_t)s;
	} else if(!(cmd & _CONF_NUM)) {
		return EINVAL;
	}

	if(!(list = realloc(*plist, (pos + (name >= _CONF_NAME_LONG ? 4 : 3)) * sizeof *list))) {
		if(str) {
			free((void *)value);
		}
		return ENOMEM;
	}
	*plist = list;
	list += pos;

	if(name >= _CONF_NAME_LONG) {
		list[1] = name;
		list[2] = value;
		list[3] = _CONF_END;
		name = _CONF_NAME_LONG;
	} else {
		list[1] = value;
		list[2] = _CONF_END;
	}
	list[0] = name | (cmd & ~_CONF_NAME_LONG) | _CONF_VALUE;
	return EOK;
}

__SRCVERSION("_conf_set.c $Rev: 153052 $");
