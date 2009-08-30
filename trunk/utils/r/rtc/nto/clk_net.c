
/*
 * $QNXLicenseC: 
 * Copyright 2007, 2008, QNX Software Systems.  
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */



#include "rtc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/procfs.h>
#include <sys/netmgr.h>
#include <fcntl.h>

static int fd;
static uint32_t nd;

int
RTCFUNC(init,net)(struct chip_loc *chip, char *argv[]) {
	char buf[512], buf2[512];

	if(argv[0] == NULL)
		nd = ND_LOCAL_NODE;
	else
		nd = netmgr_strtond(argv[0],NULL);

	if(nd == -1)
		return -1;

	if(netmgr_ndtostr(ND2S_DIR_SHOW,nd,buf2,sizeof(buf2)) == -1){
		perror("netmgr_ndtostr:");
		return -1;
	}
	sprintf(buf, "%sproc/1/as", buf2);
	fd = open(buf, O_RDONLY);
	if(fd == -1)
		return -1;
	return 0;
}

int
RTCFUNC(get,net)(struct tm *tm, int cent_reg) {
	uint64_t nsecs;
	int secs, retval, scratch;
	union {
		struct syspage_entry ent;
		char padding[8192];
	} sp, tmpsp;

	do {
		devctl(fd, DCMD_PROC_SYSINFO, &tmpsp, sizeof tmpsp, &scratch);
		retval = devctl(fd, DCMD_PROC_SYSINFO, &sp, sizeof sp, &scratch);
	} while(retval == EOK && (_SYSPAGE_ENTRY(&sp.ent, qtime)->nsec - 
		_SYSPAGE_ENTRY(&tmpsp.ent, qtime)->nsec > 1e+9) );

	if(retval != EOK)
		return -1;
	nsecs = _SYSPAGE_ENTRY(&sp.ent, qtime)->nsec +
		_SYSPAGE_ENTRY(&sp.ent, qtime)->nsec_tod_adjust ;
	secs = nsecs / 1e+9;
	*tm = *gmtime((time_t *)&secs);
	return 0;
}

int
RTCFUNC(set,net)(struct tm *tm, int cent_reg) {
	struct timespec	times;

	if(ND_NODE_CMP(nd, ND_LOCAL_NODE)){
		fprintf(stderr,"Cannot set remote time on Neutrino (yet...)\n");
		return -1;
	}

	times.tv_sec = mktime(tm);
	times.tv_nsec = 0L;
	if(clock_settime( CLOCK_REALTIME, &times)==-1){
		perror("clock_settime");
		return(-1);
	}
	return(0);
}
