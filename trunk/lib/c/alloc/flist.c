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
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
//Must use <> include for building libmalloc.so
#include <malloc-lib.h>
#include <limits.h>
#include <inttypes.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <assert.h>

static Flink __flist_avail_bins[] =  {
	{ 0, &(__flist_avail_bins[0]),  &(__flist_avail_bins[0]) }, 
	{ 0, &(__flist_avail_bins[1]),  &(__flist_avail_bins[1]) }, 
	{ 0, &(__flist_avail_bins[2]),  &(__flist_avail_bins[2]) }, 
	{ 0, &(__flist_avail_bins[3]),  &(__flist_avail_bins[3]) }, 
	{ 0, &(__flist_avail_bins[4]),  &(__flist_avail_bins[4]) }, 
	{ 0, &(__flist_avail_bins[5]),  &(__flist_avail_bins[5]) }, 
	{ 0, &(__flist_avail_bins[6]),  &(__flist_avail_bins[6]) }, 
	{ 0, &(__flist_avail_bins[7]),  &(__flist_avail_bins[7]) }, 
	{ 0, &(__flist_avail_bins[8]),  &(__flist_avail_bins[8]) }, 
};

FlinkBins  __flist_abins[] = {
  {0},
  {0},
  {0},
  {0},
  {0},
  {0},
  {0},
  {0},
  {0},
};

extern fq_fptr_t __flist_queue_funcptr;
int __flist_nbins = 9;
extern int _min_free_list_size;

Flink *__malloc_getflistptr()
{
	return(__flist_avail_bins);
}

// returns the closest power of 2 <= size
static size_t __closest_power_of_2(size_t size)
{
  int count=0;
  int notpower=0;
  size_t temp;
  temp = size;
  while (temp) {
    if ((!notpower) && (temp % 2))
      notpower=1;
    temp /= 2;
    count++;
  }
  if (notpower)
    return(1 << (count-1));
  return(size);
}

void __init_flist_bins(int minsize)
{
	int i;
	size_t ms;
	ms = __closest_power_of_2(minsize);
	for (i=0; i < __flist_nbins; i++) {
		(__flist_abins[i]).size = (ms << (i+1));
	}
	return;
}

void __flist_enqueue_bin(Flink *item, size_t size, int bin)
{
	if (bin == -1) {
		// we dont know which bin yet
		__FLIST_FIND_NBIN(size, bin);
	}
	(*__flist_queue_funcptr)(item, &(__flist_avail_bins[bin]));
	return;
}

void __flist_dequeue_bin(Flink *item, size_t size, int bin)
{
	Flink *fp;
	Flink *f;
	Flink *fn;
	f = item;
  fp = (f)->f_prev; \
  fn = (f)->f_next; \
  fp->f_next = fn; \
  fn->f_prev = fp; \
	return;
}

Fit _flist_bin_first_fit(size_t alignment, size_t size)
{
  Fit  fit;
  Flink  *cur=NULL;
  Dhead  *dlink = NULL;
	int startbin;
	int foundbin=-1;
	int i;
  long over=0;

	__FLIST_FIND_NBIN(size, startbin);

	for (i=startbin; i < __flist_nbins; i++) {
		for (cur = (__flist_avail_bins[i]).f_next; 
         cur != &(__flist_avail_bins[i]); 
         cur = cur->f_next) {
			over = 0;
    	if (DH_ISBUSY(cur)) {
      	panic("busy object on malloc free list");
			}
			if (cur->f_size >= size) {  /* first-fit */
      	void *vptr = (Dhead *)cur + 1;
      	if (cur->f_size >= (size + _min_free_list_size + _MIN_FSIZE())) {
        	Dhead *candidate;
        	ulong_t caddr;
        	/*
         	* If there's room for size and another
         	* free entry, leave (shrunken) cur and
         	* return next bytes.
         	*/
        	Dtail *dt = HEAD_TO_DT(cur);
        	vptr = (void *)((char *)dt - (size - D_OVERHEAD()));
        	caddr = __TRUNC(vptr, alignment);
        	candidate = (Dhead *)caddr-1;
        	over = (long)((char *)candidate - (char *)cur);
        	if ((over > 0) && (over >= (_min_free_list_size + _MIN_FSIZE()))) {
          	dlink = candidate;
#ifdef MALLOC_DEBUG
        		dlink->arena = ((Dhead *)cur)->arena;
#endif
        	}
      	} else if (__TRUNC(vptr, alignment) == (ulong_t)vptr) {
        	dlink = (Dhead *)cur;
      	}
      	if (dlink != NULL) {
					foundbin = i;
					goto done;
				}
    	}
  	}
	}
done:
 	fit.entry = cur;
 	fit.pos = dlink;
	fit.bin = foundbin;
	fit.over = over;
	fit.list = NULL;
 	return fit;
}

__SRCVERSION("flist.c $Rev: 212306 $");
