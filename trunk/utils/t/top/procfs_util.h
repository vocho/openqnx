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





#ifndef __procfs_util__
#define __procfs_util__

#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/procfs.h>

/* procfs_get_process_mapinfo()
 *
 * Return the mapinfo for the process indicated by the fd in mapinfo_pp.
 * Return the number of elements as the function result. If -1, then
 * there is NO memory allocated. Otherwise, the caller is responsible
 * for freeing the memory.
 *
 */
 
int procfs_get_process_mapinfo(
    int procfs_util_fd,			/* fd of an open file in /proc */
    procfs_mapinfo ** procfs_util_mapinfo_pp); /* storage returned here */

/*
 * Low memory version of above function which doesn't alloc any memory.
 * Caller has to supply a buffer for count elements. If the process has
 * more than count mapinfo structures, the API will return erraneous info.
 */
int procfs_get_process_mapinfo_lowmem(FILE *fp, int fd, 
    procfs_mapinfo *mapinfo_pp, int count);

enum MEM_TYPE {
    TEXT,
    DATA,
    STACK,
    DYNAMIC
};

/*
 * returns the size of a memory segment type passed as second argument.
 */
size_t procfs_get_mem_size(int pid, enum MEM_TYPE mtype);

/*
 * procfs_get_process_name_from_map()
 *
 * Return the process name indicated by the fd in filename. The
 * map passed in contains the memory map for the process, as obtained
 * by the function procfs_get_process_mapinfo() above.
 * The result is 0 if the name was found, or -1 if it wasn't.
 */

int procfs_get_process_name_from_map(
    int procfs_fd,		/* fd of open file in /proc */
    procfs_mapinfo * procfs_mapinfo_p, /* mem map of process */
    int procfs_num,		/* number of map elements */
    char * procfs_filename,	/* filename to store name in */
    int procfs_length);		/* Length of filename buffer */

/*
 * procfs_get_memory_name()
 *
 * Return the name (if any) associated with a bit of vm
 */

int procfs_get_memory_name(
    int procfs_fd,		/* File desc to open /proc file */
    _uint64 procfs_vaddr,	/* address to look up */
    char * procfs_filename,	/* name to store result in */
    int procfs_length);		/* length of name buffer */
/*
 * procfs_get_process_name()
 *
 * Return the process name indicated by the fd in filename. The
 * Length parameter indicates the length of the buffer. The function
 * result is 0 if the name was found, or -1 if it wasn't.
 */

int procfs_get_process_name(
    int procfs_util_fd,			/* fd of an open file in /proc */
    char * procfs_util_filename,	/* storage for the name */
    int procfs_util_length);		/* Length of the name */

/*
 * This API for getting a process's name.
 */
int procfs_get_name(pid_t pid, char *pname, int len);

/*
 * procfs_segment_name()
 *
 * Return a pointer to a text string containing a printable
 * version of the flags.
 *
 */

char * procfs_get_segment_name(unsigned long procfs_util_flags);

/*
 * procfs_for_each_pid()
 *
 * Read the /proc dir on the node, and call the function for
 * each pid you find. If the passed in pid is non-zero, then just
 * call it for that pid.
 */
void procfs_for_each_pid(
    char * procfs_node, 	/* name of node (or NULL for this one */
    void (*procfs_f)(char *, int),	/* function to call for each pid */
    int procfs_pid);		/* If this isn't 0, call only it */

/*
 * low mem version of above function.
 */
void procfs_for_each_pid_lowmem(
     void (*f)(char *, int), 
     DIR *dir);			/* opendir(/proc) return value */
#endif

