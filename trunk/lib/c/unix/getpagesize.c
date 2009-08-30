/*
 * $QNXLicenseC$
 */

#include <unistd.h>
#include <sys/mman.h>
#include <confname.h>

int
getpagesize(void)
{
	int pagesize;
   
	pagesize = sysconf(_SC_PAGESIZE);
	return (pagesize == -1) ? __PAGESIZE : pagesize;
}

__SRCVERSION("getpagesize.c $Rev$");
