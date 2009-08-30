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

#include "externs.h"
#include <confname.h>
#include <sys/conf.h>
#include <sys/procmgr.h>

int sysmgr_conf_get(int name, long *value, char *str);
int sysmgr_strconf(int name, long *value, char *str);
const long					sysmgr_conf_table[] = {
	_CONF_CALL_FCN,				(intptr_t)sysmgr_conf_get,
	_CONF_VALUE_NUM |			_SC_ARG_MAX, 				ARG_MAX,
	_CONF_VALUE_MIN |			_SC_NGROUPS_MAX, 			NGROUPS_MAX,
//	_CONF_VALUE_NUM |			_SC_JOB_CONTROL,			1,
	_CONF_VALUE_NUM |			_SC_VERSION,				_POSIX_VERSION,
	_CONF_VALUE_MIN|_CONF_INDIRECT |	_SC_PAGESIZE, 		(intptr_t)&memmgr.pagesize,
	_CONF_STR|_CONF_CALL_FCN,	(intptr_t)sysmgr_strconf,
	_CONF_LINK,					(intptr_t)kernel_conf_table
};

struct conf_entry {
	struct conf_entry			*next;
	struct conf_entry			**prev;
	pid_t						pid;
	long						*list;
}							*conf_list;

pthread_mutex_t				conf_mutex = PTHREAD_MUTEX_INITIALIZER;

void sysmgr_conf_destroy(PROCESS *prp) {
	struct conf_entry			*p;

	if((p = prp->conf_table)) {
		pthread_mutex_lock(&conf_mutex);
		if((*p->prev = p->next)) {
			p->next->prev = p->prev;
		}
		pthread_mutex_unlock(&conf_mutex);
		if(p->list) {
			_conf_destroy(p->list);
		}
		_sfree(p, sizeof *p);
		prp->conf_table = 0;
	}
}

int sysmgr_conf_get(int name, long *value, char *str) {
	struct conf_entry			*p;
	int							ret = -1;
	
	if(pthread_mutex_lock(&conf_mutex) == EOK) {
		for(p = conf_list; p; p = p->next) {
			if(p->list) {
				if(_conf_get(p->list, name, value, str) != -1) {
					ret = p->pid;
					break;
				}
			}
		}
		pthread_mutex_unlock(&conf_mutex);
	}
	return ret;
}

int sysmgr_strconf(int name, long *value, char *str) {
	struct entry {
		uint32_t			type;
		char				info[1];
	}					*ent;

	for(ent = (struct entry *)SYSPAGE_ENTRY(typed_strings)->data; ent->type != _CS_NONE;
			ent = (struct entry *)((char *)ent + ((offsetof(struct entry, info) + strlen(ent->info) + 1 + 3) & ~3))) {
		if(ent->type == name) {
			int					size;
			char				*p = ent->info;

			for(size = 0; size < *value && (str[size] = *p); p++, size++) {
				if((*p == ' ') && (name < _CS_VENDOR)) {
					str[size] = '_';
				}
			}
			while(*(p++)) {
				size++;
			}
			*value = ++size;
			return 0;
		}
	}
	return -1;
}

int sysmgr_conf_set(pid_t pid, int cmd, int name, long value, const char *str) {
	PROCESS						*prp;
	struct conf_entry			*table;
	int							status;

	if(!(prp = proc_lock_pid(pid ? pid : SYSMGR_PID))) {
		return EL2HLT;
	}
	
	if(pid) {
		if(str) {
			char						buff[1];
			long						tmp = sizeof buff;

			pid = _conf_get(sysmgr_conf_table, name, &tmp, buff);
		} else {
			long						tmp = -1;

			pid = _conf_get(sysmgr_conf_table, name, &tmp, 0);
		}

		if(pid != -1 && pid != 0 && pid != prp->pid) {
			return proc_error(EBUSY, prp);
		}
	}

	if(!(table = prp->conf_table)) {
		if(!(table = prp->conf_table = _scalloc(sizeof *table))) {
			return proc_error(ENOMEM, prp);
		}
		table->pid = prp->pid;
		table->prev = &conf_list;
		pthread_mutex_lock(&conf_mutex);
		if((table->next = conf_list)) {
			conf_list->prev = &table->next;
		}
		conf_list = table;
		pthread_mutex_unlock(&conf_mutex);
	}

	status = _conf_set(&table->list, cmd, name, value, str);
	if(status == EOK) {
		procmgr_trigger(str ? PROCMGR_EVENT_CONFSTR : PROCMGR_EVENT_SYSCONF);
	}
	return proc_error(status, prp);
}

int sysmgr_conf(resmgr_context_t *ctp, sys_conf_t *msg) {
	pid_t				pid;
	long				value;

	if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
		ENDIAN_SWAP16(&msg->i.subtype);
		ENDIAN_SWAP32(&msg->i.cmd);
		ENDIAN_SWAP32(&msg->i.name);
		ENDIAN_SWAP64(&msg->i.value);
	}
	switch(msg->i.subtype) {
	case _SYS_SUB_SET:
		if(!proc_isaccess(0, &ctp->info)) {
			return EPERM;
		}
		if(msg->i.cmd & (_CONF_CMD_MASK | (_CONF_FLAGS_MASK & ~_CONF_STICKY))) {
			return EINVAL;
		}
		pid = (msg->i.cmd & _CONF_STICKY) ? SYSMGR_PID : ctp->info.pid;
		msg->i.cmd |= _CONF_VALUE;
		if(msg->i.cmd & _CONF_STR) {
			char					*p;

			if(msg->i.value >= ctp->msg_max_size - sizeof msg->i) {
				return EINVAL;
			}
			p = (char *)(&msg->i + 1);
			p[msg->i.value] = '\0';
			if(strchr(p, ' ')) {
				return EINVAL;
			}
			return sysmgr_conf_set(pid, msg->i.cmd, msg->i.name, msg->i.value, p);
		} else if(msg->i.cmd & _CONF_NUM) {
			return sysmgr_conf_set(pid, msg->i.cmd, msg->i.name, msg->i.value, 0);
		}
		return EINVAL;
		
	case _SYS_SUB_GET:
		if(msg->i.cmd & _CONF_STR) {
			int					room = ctp->msg_max_size - sizeof msg->i;
			size_t				size;
		
			size = msg->i.value;
			value = min(msg->i.value, room);
			msg->o.match = _conf_get(sysmgr_conf_table, msg->i.name, &value, (char *)(&msg->o + 1));
			msg->o.value = min(value, room);
			if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
				ENDIAN_SWAP32(&msg->o.match);
				ENDIAN_SWAP64(&msg->o.value);
			}
			return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + min(size, msg->o.value));
		} else if(msg->i.cmd & _CONF_NUM) {
			value = msg->i.value;
			msg->o.match = _conf_get(sysmgr_conf_table, msg->i.name, &value, 0);
			msg->o.value = value;
			if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
				ENDIAN_SWAP32(&msg->o.match);
				ENDIAN_SWAP64(&msg->o.value);
			}
			return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
		}
		return EINVAL;

	default:
		break;
	}
	return ENOSYS;
}

__SRCVERSION("sysmgr_conf.c $Rev: 199396 $");
