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





#include <stddef.h>
#include <dlfcn.h>
#include <sys/syspage.h>
#include <hw/sysinfo.h>
#include "rtc.h"

#define RTC_ENV_NAME		"CfgHwRtc"
#define RTC_SYSPAGE_NAME	"/cfg/hw/rtc"

static char *
find_rtc_name() {
	char	*name;

	name = getenv(RTC_ENV_NAME);
	if(name != NULL) return(name);
	name = SYSPAGE_ENTRY(strings)->data;
	for( ;; ) {
		if(name[0] == '\0') return(NULL);
		if(memcmp(name, RTC_SYSPAGE_NAME "=", sizeof(RTC_SYSPAGE_NAME)) == 0) break;
		name += strlen(name) + 1;
	}
	return(&name[sizeof(RTC_SYSPAGE_NAME)]);
}

static char *
use_hwinfo(struct chip_loc *chip) {
	char				*name;
	hwi_tag				*tag;
	unsigned			off;
	unsigned			loc;
	struct asinfo_entry	*as;

	off = HWI_NULL_OFF;
	for( ;; ) {
		off = hwi_find_tag(off, 0, HWI_TAG_NAME_device);
		if(off == HWI_NULL_OFF) return(NULL);
		tag = hwi_off2tag(off);
		if(tag->item.owner == HWI_NULL_OFF) continue;
		tag = hwi_off2tag(tag->item.owner);
		if(strcmp(__hwi_find_string(tag->item.itemname), HWI_ITEM_DEVCLASS_RTC) == 0) break;
	}
	tag = hwi_off2tag(off);
	name = __hwi_find_string(tag->item.itemname);
	loc = hwi_find_tag(off, 1, HWI_TAG_NAME_location);
	if(loc != HWI_NULL_OFF) {
		tag = hwi_off2tag(loc);
		chip->phys = tag->location.base;
		chip->reg_shift = tag->location.regshift;
		if(tag->location.addrspace == 0xffff) {
			fprintf(stderr, "\nWrong address space for RTC.\n");
			return NULL;
		}
		as = &SYSPAGE_ENTRY(asinfo)[tag->location.addrspace/sizeof(*as)];
		chip->access_type = (strcmp(__hwi_find_string(as->name), "memory") == 0);
	}
	off = hwi_find_tag(off, 1, HWI_TAG_NAME_regname);
	if(off != HWI_NULL_OFF) {
		tag = hwi_off2tag(off);
		chip->century_reg = tag->regname.offset;
	}
	return(name);
}


char *
query_clock_hw(struct chip_loc *chip) {
	char		*rtc_name;
	char		*parm;
	char		*name;
	unsigned	len;
	
	rtc_name = find_rtc_name();
	if(rtc_name == NULL) {
		name = use_hwinfo(chip);
		if(name == NULL) {
			fprintf(stderr,"Unable to determine RTC chip type\n");
		}
		return(name);
	}
	parm = strchr(rtc_name, ' ');
	if(parm == NULL) {
		len = strlen(rtc_name);
	} else {
		len = parm - rtc_name;
		chip->phys = strtoul(parm+1, &parm, 16);
		if(*parm == ',') {
			chip->reg_shift = strtoul(parm+1, &parm, 16);
		}
		if(*parm == ',') {
			chip->access_type = strtoul(parm+1, &parm, 0);
		}
		if(*parm == ',') {
			chip->century_reg = strtoul(parm+1, &parm, 0);
		}
	}
	name = malloc(len + 1);
	if(name == NULL) {
		fprintf(stderr, "No memory\n");
		return(NULL);
	}
	memcpy(name, rtc_name, len);
	name[len] = '\0';
	return(name);
}

static int
(*get_func(void *rtc_dl, const char *name))() {
	int			(*func)();

	func = (int (*)())dlsym(rtc_dl, name);
	if(func == NULL) {
		fprintf(stderr, "Unable to find function '%s': %s\n", name, dlerror());
	}
	return(func);
}

int
load_external_clock(const char *given_name, struct rtc_desc *clk) {
	char		buff[256];
	unsigned	(**reader)(unsigned off, unsigned size);
	void		(**writer)(unsigned off, unsigned val, unsigned size);
	void		*rtc_dl;

	if(strcmp(given_name, "NONE") == 0) {
		fprintf(stderr, "No real time clock hardware available\n");
		return(-1);
	}

	sprintf(buff, "rtc-%s.so", given_name);
	rtc_dl = dlopen(buff, RTLD_NOW);
	if(rtc_dl == NULL) return(0);
	clk->init = get_func(rtc_dl, "RTC_init");
	clk->get = get_func(rtc_dl, "RTC_get");
	clk->set = get_func(rtc_dl, "RTC_set");
	if(clk->init == NULL || clk->get == NULL || clk->set == NULL) {
		return(-1);
	}
	reader = dlsym(rtc_dl, "chip_read");
	writer = dlsym(rtc_dl, "chip_write");
	if(reader == NULL || writer == NULL) {
		fprintf(stderr, "Unable to hook up read/write functions\n");
		return(-1);
	}
	*reader = chip_read;
	*writer = chip_write;
	return 1;
}
