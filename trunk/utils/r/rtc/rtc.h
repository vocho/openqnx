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




#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __QNXNTO__

#include <sys/neutrino.h>
#include <hw/inout.h>

#define VERBOSE_SUPPORTED
#define SLOWADJUST
#define _enable()   InterruptEnable()
#define _disable()  InterruptDisable()

#else

#include <sys/timers.h>
#include <sys/osinfo.h>
#include <i86.h>
#include <conio.h>

#define SLOWADJUST
#define VERBOSE_SUPPORTED
#define out8(a,b) outp(a,b)
#define in8(a) inp(a)
#define out16(a,b) outpw(a,b)
#define in16(a) inpw(a)
#define out32(a,b) outpd(a,b)
#define in32(a) inpd(a)

#define strtoull(a, b, c)	strtoul(a, b, c)
#define mmap_device_io(__len, __io)	(__io)
extern void *mmap_device_memory(void *__addr, size_t __len, int __prot, int __flags, unsigned __physical);
typedef unsigned	paddr64_t;

#endif

#define NIL_PADDR	(~(paddr64_t)0)

#define	UNSET		(-2)

struct chip_loc {
	paddr64_t	phys;
	unsigned	reg_shift;
	int			century_reg;
	enum {
			NONE = UNSET,
			IOMAPPED = 0,
			MEMMAPPED,
			RESMGR,
	} access_type;
	char		dev_write_addr;
	char		dev_read_addr;
	char		resmgr_path[PATH_MAX+1];
};

struct rtc_desc {
		const char 	*name;
		int			(*init)(struct chip_loc *, char **);
		int			(*get)(struct tm *, int);
		int			(*set)(struct tm *, int);
};

#ifdef VERBOSE_SUPPORTED
extern int verbose;
#endif

#define BIN2BCD(A)	(((((A) % 10000)/1000) << 12) + ((((A) % 1000)/100) << 8) + ((((A) % 100)/10) << 4) + ((A) % 10))
#define BCD2BIN(A)	((((A) & 0xf000) >> 12) * 1000 + (((A) & 0xf00) >> 8) * 100 + (((A) & 0xf0) >> 4) * 10 + ((A) & 0x0f))

#define RTCFUNC(type,chip)	type##_##chip

extern int load_external_clock(const char *given_name, struct rtc_desc *clk);
extern char *query_clock_hw(struct chip_loc *);

extern unsigned	chip_read(unsigned off, unsigned size);
extern void chip_write(unsigned off, unsigned val, unsigned size);

#define chip_read8(off)			chip_read(off, 8)
#define chip_write8(off,val)	chip_write(off, val, 8)
#define chip_read16(off)			chip_read(off, 16)
#define chip_write16(off,val)	chip_write(off, val, 16)

extern int init_net(struct chip_loc *chip, char *argv[]);
extern int get_net(struct tm *tm, int cent_reg);
extern int set_net(struct tm *tm, int cent_reg);

extern int init_nto_net(struct chip_loc *chip, char *argv[]);
extern int get_nto_net(struct tm *tm, int cent_reg);
extern int set_nto_net(struct tm *tm, int cent_reg);

extern int init_mc146818(struct chip_loc *chip, char *argv[]);
extern int get_mc146818(struct tm *tm, int cent_reg);
extern int set_mc146818(struct tm *tm, int cent_reg);

extern int init_ds1386(struct chip_loc *chip, char *argv[]);
extern int get_ds1386(struct tm *tm, int cent_reg);
extern int set_ds1386(struct tm *tm, int cent_reg);

extern int init_ds1743(struct chip_loc *chip, char *argv[]);
extern int get_ds1743(struct tm *tm, int cent_reg);
extern int set_ds1743(struct tm *tm, int cent_reg);

extern int init_ds15x1(struct chip_loc *chip, char *argv[]);
extern int get_ds15x1(struct tm *tm, int cent_reg);
extern int set_ds15x1(struct tm *tm, int cent_reg);

extern int init_rtc72423(struct chip_loc *chip, char *argv[]);
extern int get_rtc72423(struct tm *tm, int cent_reg);
extern int set_rtc72423(struct tm *tm, int cent_reg);

extern int init_m48t5x(struct chip_loc *chip, char *argv[]);
extern int get_m48t5x(struct tm *tm, int cent_reg);
extern int set_m48t5x(struct tm *tm, int cent_reg);

extern int init_m41t00(struct chip_loc *chip, char *argv[]);
extern int get_m41t00(struct tm *tm, int cent_reg);
extern int set_m41t00(struct tm *tm, int cent_reg);

extern int init_m41t6x(struct chip_loc *chip, char *argv[]);
extern int get_m41t6x(struct tm *tm, int cent_reg);
extern int set_m41t6x(struct tm *tm, int cent_reg);

extern int init_s35390(struct chip_loc *chip, char *argv[]);
extern int get_s35390(struct tm *tm, int cent_reg);
extern int set_s35390(struct tm *tm, int cent_reg);

extern int init_mc9s08dz60(struct chip_loc *chip, char *argv[]);
extern int get_mc9s08dz60(struct tm *tm, int cent_reg);
extern int set_mc9s08dz60(struct tm *tm, int cent_reg);

#ifdef __PPC__
extern int init_mgt5200(struct chip_loc *chip, char *argv[]);
extern int get_mgt5200(struct tm *tm, int cent_reg);
extern int set_mgt5200(struct tm *tm, int cent_reg);

extern int init_ucsa(struct chip_loc *chip, char *argv[]);
extern int get_ucsa(struct tm *tm, int cent_reg);
extern int set_ucsa(struct tm *tm, int cent_reg);
#endif

#ifdef	__ARM__
extern int init_sa1100(struct chip_loc *chip, char *argv[]);
extern int get_sa1100(struct tm *tm, int cent_reg);
extern int set_sa1100(struct tm *tm, int cent_reg);

extern int init_primecell(struct chip_loc *chip, char *argv[]);
extern int get_primecell(struct tm *tm, int cent_reg);
extern int set_primecell(struct tm *tm, int cent_reg);

extern int init_pxa250(struct chip_loc *chip, char *argv[]);
extern int get_pxa250(struct tm *tm, int cent_reg);
extern int set_pxa250(struct tm *tm, int cent_reg);

extern int init_hy7201(struct chip_loc *chip, char *argv[]);
extern int get_hy7201(struct tm *tm, int cent_reg);
extern int set_hy7201(struct tm *tm, int cent_reg);

extern int init_s3c2400(struct chip_loc *chip, char *argv[]);
extern int get_s3c2400(struct tm *tm, int cent_reg);
extern int set_s3c2400(struct tm *tm, int cent_reg);

extern int init_omap(struct chip_loc *chip, char *argv[]);
extern int get_omap(struct tm *tm, int cent_reg);
extern int set_omap(struct tm *tm, int cent_reg);

extern int init_triton(struct chip_loc *chip, char *argv[]);
extern int get_triton(struct tm *tm, int cent_reg);
extern int set_triton(struct tm *tm, int cent_reg);

extern int init_tuareg(struct chip_loc *chip, char *argv[]);
extern int get_tuareg(struct tm *tm, int cent_reg);
extern int set_tuareg(struct tm *tm, int cent_reg);

extern int init_mc9328mxlads(struct chip_loc *chip, char *argv[]);
extern int get_mc9328mxlads(struct tm *tm, int cent_reg);
extern int set_mc9328mxlads(struct tm *tm, int cent_reg);
#endif

#ifdef __MIPS__
extern int init_ds1307(struct chip_loc *chip, char *argv[]);
extern int get_ds1307(struct tm *tm, int cent_reg);
extern int set_ds1307(struct tm *tm, int cent_reg);
extern int init_m41t81(struct chip_loc *chip, char *argv[]);
extern int get_m41t81(struct tm *tm, int cent_reg);
extern int set_m41t81(struct tm *tm, int cent_reg);
#endif

#ifdef __PPC__
extern int init_rtc8xx(struct chip_loc *chip, char *argv[]);
extern int get_rtc8xx(struct tm *tm, int cent_reg);
extern int set_rtc8xx(struct tm *tm, int cent_reg);

extern int init_ds1337(struct chip_loc *chip, char *argv[]);
extern int get_ds1337(struct tm *tm, int cent_reg);
extern int set_ds1337(struct tm *tm, int cent_reg);
#endif

#ifdef __PPC__
extern int init_ds3232(struct chip_loc *chip, char *argv[]);
extern int get_ds3232(struct tm *tm, int cent_reg);
extern int set_ds3232(struct tm *tm, int cent_reg);
#endif

#ifdef	__SH4__
extern int init_rtcsh4(struct chip_loc *chip, char *argv[]);
extern int get_rtcsh4(struct tm *tm, int cent_reg);
extern int set_rtcsh4(struct tm *tm, int cent_reg);
#endif


#define TXT(s) s

