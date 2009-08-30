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
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/procfs.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/syspage.h>
#include <sys/mman.h>
#include <limits.h>
#include "procfs_util.h"

/* procfs_get_process_mapinfo()
 *
 * Return the mapinfo for the process indicated by the fd in mapinfo_pp.
 * Return the number of elements as the function result. If -1, then
 * there is NO memory allocated. Otherwise, the caller is responsible
 * for freeing the memory. Note that if this routine is called on a
 * zombie process, the num var will be set to some absurdly high value.
 * this means that we need to check it, since a process can go zombie
 * at any time (ie, after we've opened the fd). Setting num to 0 doesn't
 * help, it gets overwritten with trash.
 *
 */
 
#define MAX_ATTEMPTS 10

int procfs_get_process_mapinfo(int fd, procfs_mapinfo ** mapinfo_pp)
{
    procfs_mapinfo 	*mem = NULL;
    int			num, new_num;
    int                 iterations;
    
    if ( (devctl(fd, DCMD_PROC_MAPINFO, NULL, 0, &num) < 0) || num <= 0) {
        *mapinfo_pp = NULL;
        return -1;
    }

   /*
    * alloc space for 5 more blocks, this will help incase process made
    * more memory allocations after the last devctl call.
    */
    num += 5;
    mem = malloc(num * sizeof(procfs_mapinfo));
    if (!mem) {
        *mapinfo_pp = mem;
        return -1;
    }

    for (new_num = num + 1, iterations = 0; new_num > num
               && iterations < MAX_ATTEMPTS; iterations++) {
	if (devctl(fd, DCMD_PROC_MAPINFO, mem, num * sizeof(procfs_mapinfo),
			&new_num) < 0) {
	    free(mem);
            mem = NULL;
            new_num = -1;
	    break;
	} else if (new_num > num) {
            num = new_num + 5;
            mem = realloc(mem, num * sizeof(procfs_mapinfo));
            if (!mem) {
                new_num = -1;
                break;
            }
        }
    }

    if (iterations == MAX_ATTEMPTS) {
        /*
         * something wrong we are taking too many attempts to get the info
         * about this process, return error.
         */
        free(mem);
        mem = NULL;
        new_num = -1;
    }

    *mapinfo_pp = mem;
    return(new_num);
}

/*
 * This is a low memory version of the function which doesn't do any
 * mallocs, it just uses the buffer passed.
 */
int 
procfs_get_process_mapinfo_lowmem(FILE *fp, int fd, procfs_mapinfo *mapinfo_pp,
                                      int count)
{
    int		num;
    int		rv;
    
    rv = devctl(fd, DCMD_PROC_MAPINFO, mapinfo_pp, 
                          count * sizeof(procfs_mapinfo), &num);
    if (rv != EOK) {
	return -1;
    }

    if (num > count) {
        fprintf(fp, 
        "procfs_get_process_mapinfo_lowmem: couldn't get all the meminfo \n"
        "for the process, process has %d segments\n", num);
        num = count;
    }

    return(num);
}

size_t
procfs_get_mem_size(int pid, enum MEM_TYPE mtype)
{
    int     fd;
    int     i, num;
    char    buff[50];
    size_t  dynamic;
    size_t  text;
    size_t  data;
    size_t  stack;
    procfs_mapinfo *mem;

    snprintf(buff, sizeof(buff) - 1, "/proc/%d", pid);
    if ((fd = open(buff, O_RDONLY)) == -1)
	return -1;

    num = procfs_get_process_mapinfo(fd, &mem);
    if (num == -1) {
        close(fd);
        return -1;
    }

    dynamic = 0;
    text = 0;
    data = 0;
    stack = 0;
    for (i = 0; i < num; i++) {
	if ((mem[i].flags & MAP_TYPE) == MAP_SHARED) {
	    /* Ignore it */
	    continue;
	}
	switch(mem[i].dev) {
	case 1:
	    if (mem[i].flags & MAP_ELF) {
		/* This is text or data */
		if (text > 0) {
		    if (mem[i].flags & PG_HWMAPPED) {
			data += mem[i].size;
		    }
		} else {
		    text = mem[i].size;
		}
	    }
	    break;
	case 2:
	    /* This is mapped memory */
	    if (mem[i].flags & MAP_FIXED) {
		/* This is device memory most likely. Ignore it. */
	    } else if ((mem[i].flags & (MAP_STACK|PG_HWMAPPED)) == (MAP_STACK|PG_HWMAPPED)) {
		/* This is stack and it has a physcial backing store */
		stack += mem[i].size;
	    } else if (mem[i].flags & PG_HWMAPPED) {
		/* This is not device memory. It has a physical backing store.
		 * Count is as data
		 */
	    dynamic += mem[i].size;
	    }
	    break;
	case 3:
	    /* This is shared memory */
	    break;
	case 4:
	    /* This is DLL text. Ignore it */
	    break;
	}
    }

    close(fd);
    free(mem);

    switch (mtype) {
    case TEXT:
        return text;
    case DATA:
        return data;
    case STACK:
        return stack;
    case DYNAMIC:
        return dynamic;
    }

    return -1;
}

/*
 * procfs_get_memory_name()
 *
 * Get the file name associated with a memory segment
 */

int procfs_get_memory_name(int fd, _uint64 vaddr, char * filename, int length)
{
    char		path[sizeof(procfs_debuginfo) + _POSIX_PATH_MAX];
    procfs_debuginfo	*name = (procfs_debuginfo *)path;
    int 		rv;

    name->vaddr = vaddr;
    rv = devctl(fd, DCMD_PROC_MAPDEBUG, name, sizeof(path), 0);
    if (rv != -1) {
        strncpy(filename, name->path, length);
        filename[length-1]='\0'; // strncpy might not zero-terminate if name->path is too long
    }
    return rv;
}

/*
 * procfs_get_process_name_from_map()
 *
 * Return the process name indicated by the fd in filename. The
 * map passed in contains the memory map for the process, as obtained
 * by the function procfs_get_process_mapinfo() above.
 * The result is 0 if the name was found, or -1 if it wasn't.
 */

int procfs_get_process_name_from_map(
    int fd,			/* fd of open file in /proc */
    procfs_mapinfo * mapinfo_p, /* mem map of process */
    int num,			/* number of map elements */
    char * filename,		/* filename to store name in */
    int length)			/* Length of filename buffer */
{
    int			i;
    int 		rv = -1;

    if (num >= 0) {
	for (i = 0; i < num; i++) {
	    if (mapinfo_p[i].flags & MAP_ELF) {
		/* We have an ELF segment. Lets return the name of the
		 * file associated with it as the process name.
		 */
		rv = procfs_get_memory_name(
		    fd, mapinfo_p[i].vaddr,
		    filename, length);
		if (rv == -1) {
		    /* Try again with the next one */
		    continue;
		}
		break;
	    }
	}
    }
    return (rv);
}

/*
 * procfs_get_process_name()
 *
 * Return the process name indicated by the fd in filename. The
 * Length parameter indicates the length of the buffer. The function
 * result is 0 if the name was found, or -1 if it wasn't.
 */

int procfs_get_process_name(int fd, char *filename, int length)
{
   static struct {
       procfs_debuginfo  info;
       char              buff[_POSIX_PATH_MAX];
   } name;

   if (devctl(fd, DCMD_PROC_MAPDEBUG_BASE, &name, sizeof name, 0) != EOK) {
       return -1;
   }
   strncpy(filename, name.info.path, length);
   filename[length-1]='\0'; // strncpy might not zero-terminate if name.info.path is too long
   return 0;
}

/*
 * This API for getting a process's name.
 */
int 
procfs_get_name(pid_t pid, char *pname, int len)
{
    int  fd, rv;
    char buff[50];

    snprintf(buff, sizeof(buff) - 1, "/proc/%d", pid);
    if ((fd = open(buff, O_RDONLY)) == -1) {
	return -1;
    }

   rv = procfs_get_process_name(fd, pname, len);
   close(fd);
   return rv;
}

/*
 * procfs_segment_name()
 *
 * return a string containing a list of segment type names
 */

static
struct bitname_type
{
    unsigned long bit;
    char	  *name;
} bitnames[] =
{
    { MAP_FIXED, "FIXED" },
    { MAP_ELF, "ELF" },
    { MAP_NOSYNCFILE, "NOSYNCFILE" },
    { MAP_LAZY, "LAZY" },
    { MAP_STACK, "STACK" },
    { MAP_BELOW, "BELOW" },
    { MAP_PHYS, "PHYS" },
    { MAP_NOX64K, "NOX64K" },
    { MAP_BELOW16M, "BELOW16M" },
    { MAP_SYSRAM, "SYSRAM" },
    { 0, 0 }
};

char * procfs_get_segment_name(unsigned long flags)
{
    static char segment[256];
    struct bitname_type * p;
    int n = 0;

    segment[0] = '\0';

    switch(flags & MAP_TYPE) {
    case MAP_SHARED:
	strcpy(segment, "SHARED");
	break;
    case MAP_PRIVATE:
	strcpy(segment, "PRIVATE");
	break;
    case MAP_ANON:
	strcpy(segment, "ANON");
	break;
    case MAP_FILE:
	strcpy(segment, "FILE");
	break;
    }
    n += (flags & MAP_TYPE);
    
    for (p = bitnames; p->bit; p++) {
	if (flags & p->bit) {
	    if (n++) {
		strcat(segment, " ");
	    }
	    strcat(segment, p->name);
	}
    }
    return segment;
}

/*
 * procfs_for_each_pid()
 *
 * Read the /proc dir on the node, and call the function for
 * each pid you find.
 */
void procfs_for_each_pid(char * node, void (*f)(char *, int), int pid)
{
    DIR * dir;
    char fname[_POSIX_PATH_MAX];

    if (pid > 0) {
	f(node, pid);
    } else {
	if (node == NULL) {
	    dir = opendir("/proc");
	} else {
	    sprintf(fname, "/dev/proc.d/%s", node);
	    dir = opendir(fname);
	}

	if (dir) {
	    struct dirent * dirent;

	    while ((dirent = readdir(dir)) != NULL) {
		if (isdigit(dirent->d_name[0])) {
		    f(node, atoi(dirent->d_name));
		}
	    }
	    closedir(dir);
	}
    }
}

/*
 * low mem version of above function, uses a already opened directory.
 */
void procfs_for_each_pid_lowmem(void (*f)(char *, int), DIR *dir)
{
    struct dirent * dirent;

    rewinddir(dir);
    while ((dirent = readdir(dir)) != NULL) {
        if (isdigit(dirent->d_name[0])) {
            f(0, atoi(dirent->d_name));
        }
    }
}


