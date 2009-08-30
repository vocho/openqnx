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

/*==============================================================================
 * 
 * apmgr_misc
 * 
 * Miscellaneous support routines
 * 
*/

#include "apmgr.h"


/*******************************************************************************
 * isProcessPath
 * 
 * this routine will return true as long as the first '/' delimited characters
 * of path represent an actual pid_t. It does not matter what follows
 * 
 * ex.
 * 	if <path> is
 * 
 * 		"1234"
 * 		"1234/"
 * 		"1234/this/that"
 * 
 * and '1234' represents a valid process, then TRUE is returned and <*pid_p> will
 * be set to the pid value 1234.
 * On any error, FALSE is returned and <*pid_p> is garbage
*/
#define MAX_PID_T_DIGITS	((sizeof(pid_t) <= 4) ? \
							sizeof("4294967295") : sizeof("18446744073709551615"))

bool isProcessPath(char *path, pid_t *pid_p)
{
	char *p;

	CRASHCHECK((path == NULL) || (*path == '\0') || (pid_p == NULL));

	/* search at most, the number of digits in a 'pid_t' */
	if ((p = memchr(path, '/', MAX_PID_T_DIGITS)) != NULL)
		*p = '\0';
	*pid_p = strtoul(path, NULL, 10);
	if (p) *p = '/';	// restore <path>

	return (proc_lookup_pid(*pid_p) == NULL) ? bool_t_FALSE : bool_t_TRUE;
}

/*******************************************************************************
 * nametoolong
 * 
 * Check whether <name> is too long. The check is based on <chktype> which
 * indicates whether or not <name> should be checked against the maximum path
 * size (PATH_MAX) or the maximum file/dirent size (NAME_MAX)
 * 
 * When considering the maximum name lengths, we must consider the resource
 * manager prefix as well as the procfs prefix.
 * For example, the name that would be passed to this function for a PATH_MAX
 * check from the memory partitioning resource manager could be
 * "sysram/p0/p0.1/p0.1.1". The fully qualified pathname however is
 * "/partition/mem/sysram/p0/p0.1/p0.1.1" and so the memory partition resource
 * manager prefix must be considered in the calculation. In addition, since
 * the partition namespace is accessible from the /proc namespace, we must also
 * consider this prefix since to list the partitions a particular process is
 * associated with, we would use the fully qualified pathname
 * "/proc/<pid>/partition/mem/sysram/p0/p0.1/p0.1.1" and so this prefix must
 * also be considered.
 * The same applies to the scheduling partition resource manager and so the
 * argument <apmgrtype> is used to indicate which resource manager is making
 * the call. A test is done against the xxx_open() handler for the supported
 * callers.
 * 
 * Returns: true or false depending on the result
 * 
 * FIX ME - consider this to be an inlined function
*/
bool nametoolong(const char *name, unsigned chktype, void *apmgrtype)
{
	unsigned prefix_len;

#ifndef NDEBUG
// FIX ME
extern dev_t apsmgr_devno;
	if (name == NULL) crash();
	if ((chktype != PATH_MAX) && (chktype != NAME_MAX)) crash();
	if ((apmgrtype != (void *)apmgr_devno) &&
		(apmgrtype != (void *)apmmgr_devno) &&
		(apmgrtype != (void *)apsmgr_devno)) crash();
#endif	/* NDEBUG */

	prefix_len = (sizeof("/proc/") - 1) +
				 MAX_PID_T_DIGITS +
				 (sizeof("/partition/") - 1) +
				 /* for apmgr_devno and apsmgr_devno, we use "sched/", the longest */
				 (((apmgrtype == (void *)apmmgr_devno) ? sizeof("mem/") : sizeof("sched/")) - 1);

	switch (chktype)
	{
		case PATH_MAX:
		{
			/*
			 * FIX ME
			 * there is a yet to be explained crash (due to corruption) sometimes
			 * in pathmgr_resolve(), sometimes in heap_chk() when creating a huge
			 * partition hierarchy (have been able to get just over 200) but does
			 * not exceed PATH_MAX. I am going to artificially cap the depth at 20
			 * for now until I figure out the real problem
			*/
			char *p = (char *)name;
			unsigned max_depth = 20;
			while (((p = strchr(p, '/')) != NULL) && (--max_depth > 0))
				++p;
			if (max_depth == 0)
				return bool_t_TRUE;
			else
				return (strlen(name) > (PATH_MAX - prefix_len)) ? bool_t_TRUE : bool_t_FALSE;
		}

		case NAME_MAX:
		{
			char *name_p = (char *)name;
			char *chk_name = name_p;

			while ((name_p = strchr(chk_name, '/')) != NULL)
			{
				*name_p = '\0';
				if (strlen(chk_name) > NAME_MAX)
					return bool_t_TRUE;
				*name_p++ = '/';
				chk_name = name_p;
			}
			return (strlen(chk_name) > NAME_MAX) ? bool_t_TRUE : bool_t_FALSE;
		}
		default:
#ifndef NDEBUG
			crash();	// can't get here right?
#endif	/* NDEBUG */
			return bool_t_TRUE;	// fail it
	}
}



__SRCVERSION("$IQ: apmgr_misc.c,v 1.23 $");

