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
#include <sys/types.h>
#include "malloc-lib.h"
#include "mallocint.h"
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/neutrino.h>
#include <sys/procfs.h>
#include <sys/debug.h>
#include <sys/link.h>
#include <sys/elf.h>
#include <sys/types.h>
#include <sys/stat.h>

extern unsigned long _etext;
extern unsigned long _edata;
extern unsigned long __bss_start;
extern unsigned long _end;
extern pthread_t __ctid;

void *
malloc_data()
{
    return (void *)&_etext;
}

void *
malloc_edata()
{
    return (void *)&_edata;
}

void *
malloc_bss()
{
    return (void *)&__bss_start;
}

void *
malloc_end()
{
    return (void *)&_end;
}

/*
 * Scan through the data segment
 */
static int
_fill_dss(mse_t *dss, int n)
{
  extern struct r_debug _r_debug;
  Link_map *lp;
  int i;
  int cnt = 0;

	cnt=0;

  dss[cnt].mse_start = (ulong_t)malloc_data();
  dss[cnt].mse_end = (ulong_t)malloc_edata();
  cnt++;
  dss[cnt].mse_start = (ulong_t)malloc_bss();
  dss[cnt].mse_end = (ulong_t)malloc_end();
  cnt++;

  for (lp = _r_debug.r_map; cnt < n && lp; lp = lp->l_next) {
		Elf32_Ehdr	*ehdr;

		/* Get ELF file's Program header, look at text vaddr and
	 	* compare it to loaded address for an offset
	 	*/
		ehdr = (Elf32_Ehdr *)lp->l_addr;	

		/* Do we have to worry about xlat on the ELF file? */

		for (i = 0; i < ehdr->e_phnum; i++) {
	   	Elf32_Phdr *phdr;

			phdr = (Elf32_Phdr *)((char *)ehdr+ehdr->e_phoff+i* ehdr->e_phentsize);

	    /*
	     * Look at the rw- segment for data, get the vaddr and memsz,
	     * and add the offset to the vaddr to get the loaded base
	     */
	    if (phdr->p_type == PT_LOAD
	        && (phdr->p_flags & (PF_R|PF_W)) == (PF_R|PF_W)
	        && phdr->p_memsz > 0) {

	        mse_t *ds = &dss[cnt++];

          ds->mse_start = 
            ((lp->l_name == NULL) ? 0 : 
              ((ehdr->e_type == ET_EXEC) ? 0 : lp->l_addr)) + phdr->p_vaddr;
          ds->mse_end = phdr->p_memsz + ds->mse_start;
	    }
		}
  }
  return cnt;
}

/*
 * Fill the stack segment information
 */
static int
_fill_sss(mse_t *sss, int n, ulong_t top)
{
  procfs_status status;
  procfs_info status1;
  int tid;
  //int lasttid;
  int tids = 0;
  pthread_t self = pthread_self();
  char procname[32];
  int cnt = 0;
  int procfd;
	int numt;

  sprintf(&procname[0], "/proc/%d", getpid());
  procfd = open(&procname[0], O_RDONLY, 0);

  if (procfd < 0) {
		mse_t *ss = &sss[cnt++];

    ss->mse_start = top;
    ss->mse_end = (ulong_t)__tls();
    return cnt;
  } 

  if (devctl(procfd, DCMD_PROC_INFO, &status1, sizeof status1, 0) != EOK) {
		mse_t *ss = &sss[cnt++];

    ss->mse_start = top;
    ss->mse_end = (ulong_t)__tls();
    return cnt;
	}
	numt = status1.num_threads;
	tids = 0;
	tid = 1;
	while (tids < numt) {
		mse_t *ss;

    status.tid = tid;
    if (devctl(procfd, DCMD_PROC_TIDSTATUS, &status, sizeof status, 0) != EOK) {
		tid++;
    	continue;
		}
    tids++;

		ss = &sss[cnt++];
		ss->mse_start = ((status.sp + 4) & ~3);	/* low address - round up */
		if (status.sp & ~3)
			ss->mse_start = ((status.sp + 4) & ~3);	/* low address - round up */
		else
			ss->mse_start = status.sp;	/* low address - round up */
		//ss->mse_start = status.tls;
		if ((tid == (int)self) && (ss->mse_start < top)) { 
			ss->mse_start = top;			/* don't start below top */
			//ss->mse_start = (ulong_t)__tls();			/* don't start below top */
		}
		ss->mse_end = status.stkbase + status.stksize; /* high address */
		if (cnt >= n)
			break;
		tid++;
  }
  close(procfd);
  return cnt;
}

mse_t *_dss;
int _nds;

mse_t *_sss;
int _nss;

int
_malloc_scan_start(mse_t *dss, int n, ulong_t top)
{
    int cnt;
    cnt = _fill_dss(dss, n);
    _dss = dss;
    _nds = cnt;
    cnt = _fill_sss(dss+_nds, n-_nds, top);
    _sss = dss+_nds;
    _nss = cnt;
    return _nds+cnt;
}

int
_malloc_scan_finish()
{
	return(0);
}
