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

#define T_CANT_GET_OS_INFO	"rtc: Can't determine machine type!\n"
#define T_UNKNOWN_MACHINE_TYPE	"rtc: Unknown machine type (%s)\n"

char *
query_clock_hw(struct chip_loc *chip) {
	struct _osinfo osdata;

	if (qnx_osinfo(0,&osdata) == -1) {
		perror(TXT(T_CANT_GET_OS_INFO));
		return NULL;
	}
	if( !strcmp(osdata.machine,"AT")
	 || !strcmp(osdata.machine,"PCI")
	 || !strcmp(osdata.machine,"EISA")
	 || !strcmp(osdata.machine,"PS/2")) {
		return "mc146818";
	 }
	fprintf(stderr,TXT(T_UNKNOWN_MACHINE_TYPE),osdata.machine);
	return NULL;
}

int
load_external_clock(const char *given_name, struct rtc_desc *clk) {
	return 0;
}
