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

#ifdef VERBOSE_SUPPORTED
#define VERBOSE_OPTIONS "v"
#else
#define VERBOSE_OPTIONS
#endif

#ifdef SLOWADJUST
#define SLOWADJUST_OPTIONS "r:S:"
#else
#define SLOWADJUST_OPTIONS
#endif

#ifdef SLOWADJUST
long		rate = 100;
long        maxsec = 60;
#endif
#ifdef VERBOSE_SUPPORTED
int			verbose=0;
#endif
struct chip_loc	chip;
union {
	int			fd;
	volatile uint8_t	*p;
	unsigned		port;
}			chip_base;


unsigned
chip_read(unsigned off, unsigned size) {
	unsigned	disp = (off << chip.reg_shift);
	unsigned	val = 0;

	switch (chip.access_type) {
	case RESMGR:
		off |= chip.dev_read_addr << 16;
		switch(size) {
		case 8:
			pread(chip_base.fd, &val, 1, off);
			break;
		case 16:
			pread(chip_base.fd, &val, 2, off);
			break;
		case 32:
			pread(chip_base.fd, &val, 4, off);
			break;
		}
		break;
	case MEMMAPPED:
	default:
		switch(size) {
		case 8:
			val = chip_base.p[disp];
			break;
		case 16:
			val = *(volatile uint16_t *)&chip_base.p[disp];
			break;
		case 32:
			val = *(volatile uint32_t *)&chip_base.p[disp];
			break;
		}
		break;
	case IOMAPPED:
		switch(size) {
		case 8:
			val = in8(chip_base.port + disp);
			break;
		case 16:
			val = in16(chip_base.port + disp);
			break;
		case 32:
			val = in32(chip_base.port + disp);
			break;
		}
		break;
	}
	return(val);
}

void
chip_write(unsigned off, unsigned val, unsigned size) {
	unsigned	disp = (off << chip.reg_shift);

	switch (chip.access_type) {
	case RESMGR:
		off |= chip.dev_write_addr << 16;
		switch(size) {
		case 8:
			pwrite(chip_base.fd, &val, 1, off);
			break;
		case 16:
			pwrite(chip_base.fd, &val, 2, off);
			break;
		case 32:
			pwrite(chip_base.fd, &val, 4, off);
			break;
		}
		break;
	case MEMMAPPED:
	default:
		switch(size) {
		case 8:
			chip_base.p[disp] = val;
			break;
		case 16:
			*(volatile uint16_t *)&chip_base.p[disp] = val;
			break;
		case 32:
			*(volatile uint32_t *)&chip_base.p[disp] = val;
			break;
		}
		break;
	case IOMAPPED:
		switch(size) {
		case 8:
			out8(chip_base.port + disp, val);
			break;
		case 16:
			out16(chip_base.port + disp, val);
			break;
		case 32:
			out32(chip_base.port + disp, val);
			break;
		}
		break;
	}
}

static struct rtc_desc clocks[] ={
#define NET_CLOCK_IDX 	0
	{ "net", 		init_net, get_net, set_net },
	{ "hw", 		NULL, NULL, NULL },
	{ "at", 		init_mc146818, get_mc146818, set_mc146818 },
	{ "ps/2", 		init_mc146818, get_mc146818, set_mc146818 },
	{ "mc146818",	init_mc146818, get_mc146818, set_mc146818 },
#ifdef __MIPS__
	{ "ds1307",		init_ds1307, get_ds1307, set_ds1307 },
	{ "m41t81",		init_m41t81, get_m41t81, set_m41t81 },
#endif
	{ "ds1386",		init_ds1386, get_ds1386, set_ds1386 },
	{ "ds15x1",		init_ds15x1, get_ds15x1, set_ds15x1 },
	{ "ds1743",		init_ds1743, get_ds1743, set_ds1743 },
	{ "rtc72423",	init_rtc72423, get_rtc72423, set_rtc72423 },
	{ "m48t5x",		init_m48t5x, get_m48t5x, set_m48t5x },
    { "m41t6x",     init_m41t6x, get_m41t6x, set_m41t6x },
	{ "s35390",		init_s35390, get_s35390, set_s35390 },
	{ "mc9s08dz60", init_mc9s08dz60, get_mc9s08dz60, set_mc9s08dz60 },
#ifdef __PPC__
	{ "rtc8xx",     init_rtc8xx, get_rtc8xx, set_rtc8xx},
	{ "mgt5200",    init_mgt5200, get_mgt5200, set_mgt5200},
    { "ds3232",     init_ds3232, get_ds3232, set_ds3232},
#endif
#ifdef	__SH4__
	{ "rtcsh4",		init_rtcsh4, get_rtcsh4, set_rtcsh4 },
#endif
#ifdef	__ARM__
	{ "sa1100",		init_sa1100, get_sa1100, set_sa1100 },
	{ "pxa250",		init_pxa250, get_pxa250, set_pxa250 },
	{ "primecell",	init_primecell, get_primecell, set_primecell },
	{ "hy7201",		init_hy7201, get_hy7201, set_hy7201 },
    { "s3c2400",    init_s3c2400, get_s3c2400, set_s3c2400 },
    { "omap",		init_omap, get_omap, set_omap },
	{ "tuareg",		init_tuareg, get_tuareg, set_tuareg },
	{ "mc9328mxlads",	init_mc9328mxlads, get_mc9328mxlads, set_mc9328mxlads },
#endif
	{ NULL,			NULL }
};

static void
rtc_list() {
	int		i;

	for(i = 0; clocks[i].name != NULL; ++i) {
		fprintf(stderr, "%s%s", (i==0)? "" : "|", clocks[i].name);
	}
}

#define TXT(s)	s

int
main(int argc, char *argv[]) {
	int 			clk,setflag=0, local=0;
	struct tm 		*p,local_time;
	int 			opt, error=0;
	time_t			timevar;
	int				size;
	char			*given_name;

	chip.phys = NIL_PADDR;
	chip.resmgr_path[0] = '\0';
	chip.access_type = NONE;
	chip.century_reg = UNSET;
	while ((opt=getopt(argc,argv,"b:ls" SLOWADJUST_OPTIONS VERBOSE_OPTIONS))!=-1) {
		switch(opt) {
		case 'b':
			chip.phys = strtoull(optarg, &optarg, 16);
			if(*optarg == ',') {
				chip.reg_shift = strtoul(optarg+1, &optarg, 16);
			}
			if(*optarg == ',') {
				chip.access_type = strtoul(optarg+1, &optarg, 0);
			}
			if(*optarg == ',') {
				chip.century_reg = strtoul(optarg+1, &optarg, 0);
			}
			break;
		case 'l':
			local++;
			break;
#ifdef SLOWADJUST
		case 'r': rate = atol(optarg);	break;
		case 'S': maxsec = atol(optarg); break;
#endif
		case 's':
			setflag++;
			break;
#ifdef VERBOSE_SUPPORTED
		case 'v': verbose++; break;
#endif
		default:
			error++;
			break;
		}
	}

	if((argc-optind)>2) {
		error++;
		fprintf(stderr,"rtc: Too many command line operands.\n");
	} else if ((argc-optind)<1) {
		error++;
		fprintf(stderr,"rtc: Must specify a clock type (");
		rtc_list();
		fprintf(stderr, ").\n");
	}
	
	if(geteuid()) {
		error++;
		fprintf(stderr,"rtc: operation not permitted -- must be root.\n");
	}
#ifdef __QNXNTO__                                                               
	if(ThreadCtl(_NTO_TCTL_IO,NULL) == -1) {
		perror("Can not obtain I/O privledges");
		++error;
	}
#endif

	if(error) exit(EXIT_FAILURE);

	clk = 0;
	given_name = argv[optind];
	for( ;; ) {
		if(clocks[clk].name == NULL) {
			switch(load_external_clock(given_name, &clocks[clk])) {
			case 0:
				fprintf(stderr, "rtc: %s - Unknown clock type. Must be ", given_name);
				rtc_list();
				fprintf(stderr, ".\n");
				/* fall through */
			case -1:	/* already reported problem */
				exit(EXIT_FAILURE);
			}
			break;
		}
		if(strcmp(clocks[clk].name, given_name) == 0) {
			if(clocks[clk].init != NULL) break;
			/*
				Specified 'hw' for the name. Figure out what what hardware
				is on the system.
			*/
			given_name = query_clock_hw(&chip);
			if(given_name == NULL) {
				exit(EXIT_FAILURE);
			}
			clk = -1; /* run the list again */
		}
		++clk;
	}

   	size = clocks[clk].init(&chip, &argv[optind+1]);
	if(size < 0) {
		exit(EXIT_FAILURE);
	}
	if(size > 0) {
		switch (chip.access_type) {
		case RESMGR:
			chip_base.fd = open(chip.resmgr_path, O_RDWR);
			if(chip_base.fd == -1) {
				perror("Can not obtain access to RTC chip");
				exit(EXIT_FAILURE);
			}
			break;
		case MEMMAPPED:
		default:
			chip_base.p = mmap_device_memory(NULL, size << chip.reg_shift, PROT_READ|PROT_WRITE|PROT_NOCACHE, 0, chip.phys);
			if(chip_base.p == MAP_FAILED) {
				perror("Can not obtain access to RTC chip");
				exit(EXIT_FAILURE);
			}
			break;
		case IOMAPPED:
			chip_base.port = mmap_device_io(size << chip.reg_shift, chip.phys);
			if(chip_base.port == -1) {
				perror("Can not obtain access to RTC chip");
				exit(EXIT_FAILURE);
			}
			break;
		}
	}

/*
 *	If -s option is true, read & decompose the QNX System Date.
 */
	if (setflag) {
		time(&timevar);		/* Get UTC	time.				*/
		/*
			printf("time = %ld\n",timevar);
		*/
		if (local) {
			p = localtime(&timevar);	/* Break-down into localtime.	*/
		} else {	
		    p = gmtime(&timevar);		/* Break-down into UTC time.	*/
		}

		#ifdef VERBOSE_SUPPORTED
		if (verbose) {
			printf("rtc set: current OS time is %d/%d/%d %d:%d:%d %s\n",
							p->tm_year+1900,p->tm_mon+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec,
	                       local?"(Local)":"(UTC)");
		}
		#endif
		if(clocks[clk].set(p, chip.century_reg) < 0) {
			exit(EXIT_FAILURE);
		}
	} else {
		p = &local_time;
		memset(p, 0, sizeof(*p));
		p->tm_isdst = -1;
		if(clocks[clk].get(p, chip.century_reg) < 0) {
			exit(EXIT_FAILURE);
		}
		#ifdef VERBOSE_SUPPORTED
		if (verbose) {
			printf("rtc get: current RTC time is %d/%d/%d %d:%d:%d %s\n",
							p->tm_year+1900,p->tm_mon+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec,
	                       local?"(Local)":"(UTC)");
		}
		#endif

		/* If local time, don't use dst as "timezone" subtracted below is always standard time */
		if ( clk != NET_CLOCK_IDX && !local ) p->tm_isdst = 0;

		timevar = mktime(p);

		if(clk != NET_CLOCK_IDX) {
			/*
			 *	MKTIME() assumes local time and will
			 *	subtract timezone to obtain UTC
			 */


			#ifdef DIAG
			printf("tm_isdst = %d\n",p->tm_isdst);
			printf("timezone = %ld\n",timezone);
			printf("timevar = mktime(p) = %u\n",timevar);
			#endif
		
			/* 
			 *	If hardware time was already in UTC, then we
			 *	overcompensated and must re-adjust to UTC.
			 */
	
			if ( !local )  timevar -= timezone;
		}

		/* Set QNX UTC time.	*/
		{
			struct timespec new, cur;
			long sec, usec;
			int rc = -1;
#ifdef SLOWADJUST
#ifndef __QNXNTO__
#ifdef VERBOSE_SUPPORTED
			long int c,d;
#endif
#endif
#endif

			new.tv_nsec = 0;
			new.tv_sec = timevar;

#ifdef SLOWADJUST
#ifndef __QNXNTO__
			getclock(TIMEOFDAY, &cur);

			if((sec = new.tv_sec - cur.tv_sec) >= -maxsec  &&  sec <= maxsec) {
				usec  = new.tv_sec*1000000 + new.tv_nsec/1000;
				usec -= cur.tv_sec*1000000 + cur.tv_nsec/1000;
#ifdef VERBOSE_SUPPORTED
				if (verbose) {
					if (!qnx_adj_time(0,0,&c,&d)) {
						if (d) {
							fprintf(stderr,"Time is currently being adjusted %d ns/tick, %d ticks remaining\n",c,d);
							fprintf(stderr,"Will override.\n");
						}
					}
				}
#endif

				rc = qnx_adj_time(usec, rate, NULL, NULL);
	
#ifdef VERBOSE_SUPPORTED
				if (verbose)
				do {
					sleep(1);
					if (!qnx_adj_time(0,0,&c,&d)) {
						fprintf(stderr,"%d ns/tick, %d ticks remaining\n",c,d);
					} else break;
				} while (d!=0);			
#endif

				}
#else
            /* Neutrino version */

			clock_gettime(CLOCK_REALTIME,&cur);

			/* under nto this is more complicated. You can't just specify 
               the total time to be adjusted, rather, you must specify
               an amount per tick and a number of ticks (the product of
               which is the total time). This means that some sort of
               factoring algorithm must be used to produce an nsec
               per-tick amount and a number of ticks which will equal
               the total time required. */
               
			if((sec = new.tv_sec - cur.tv_sec) >= -maxsec  &&  sec <= maxsec) {
				struct _clockadjust adj;

#ifdef VERBOSE_SUPPORTED
				if (verbose) printf("rtc read: attempting slow adjust (delta = %ld sec.)\n",sec);
#endif
				usec  = new.tv_sec*1000000 + new.tv_nsec/1000;
				usec -= cur.tv_sec*1000000 + cur.tv_nsec/1000;
    			
				/* we want usec*1000 total ns over 'rate' clock ticks */
				adj.tick_nsec_inc=1000L*(usec/rate); /* may be up to 'rate' usec of error */

				/*zzx need to check two things: first, that the error time
                   is not greater than our allowable threshold, and
                   second, that the per tick amount is not greater
                   than the amount of time in a tick */
	
                adj.tick_count = rate;
#ifdef VERBOSE_SUPPORTED
				if (verbose) {
					printf("rtc read: Adjusting clock %ld000 nsec\n",usec);
					printf("rtc read: (%ld nsec per tick over %ld ticks)\n",adj.tick_nsec_inc,adj.tick_count);
				}
#endif
				rc= ClockAdjust(CLOCK_REALTIME,&adj,NULL);
#ifdef VERBOSE_SUPPORTED
				if (verbose>1) {
					do {				
						sleep(1);
						ClockAdjust(CLOCK_REALTIME,NULL,&adj);
						printf("rtc adj: tick_count=%ld\n",adj.tick_count);
					} while (adj.tick_count!=0);
                }
#endif
				}
#endif   /* __QNXNTO__ defined */
#endif   /* SLOWADJUST is supported */

			if(rc == -1) {
#ifdef __QNXNTO__
				if (clock_settime(CLOCK_REALTIME,&new)) {
#else
				if(setclock(TIMEOFDAY, &new)) {
#endif
					perror("Can't set system time");
					exit(EXIT_FAILURE);
				}
			}
		}
	}

    return 0;
}
