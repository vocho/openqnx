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
#include <time.h>
#include <fcntl.h>
#include <hw/i2c.h>

/* 
 * Dallas/Maxim DS3232 Serial RTC
 */
#define DS3232_SEC          0   /* 00-59 */
#define DS3232_MIN          1   /* 00-59 */
#define DS3232_HOUR         2   /* 0-1/00-23 */
#define DS3232_DAY          3   /* 01-07 */
#define DS3232_DATE         4   /* 01-31 */
#define DS3232_MONTH        5   /* 01-12 */
#define DS3232_YEAR         6   /* 00-99 */


#define DS3232_I2C_ADDRESS  (0xD0 >> 1)
#define DS3232_I2C_DEVNAME  "/dev/i2c1"


static int fd = -1;

static int
ds3232_i2c_read(unsigned char reg, unsigned char val[], unsigned char num)
{
    iov_t           siov[2], riov[2];
    i2c_sendrecv_t  hdr;

    hdr.slave.addr = DS3232_I2C_ADDRESS;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.send_len = 1;
    hdr.recv_len = num;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));
    SETIOV(&siov[1], &reg, sizeof(reg));

    SETIOV(&riov[0], &hdr, sizeof(hdr));
    SETIOV(&riov[1], val, num);

    return devctlv(fd, DCMD_I2C_SENDRECV, 2, 2, siov, riov, NULL);
}

static int
ds3232_i2c_write(unsigned char reg, unsigned char val[], unsigned char num)
{
    iov_t           siov[3];
    i2c_send_t      hdr;

    hdr.slave.addr = DS3232_I2C_ADDRESS;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.len = num + 1;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));
    SETIOV(&siov[1], &reg, sizeof(reg));
    SETIOV(&siov[2], val, num);

    return devctlv(fd, DCMD_I2C_SEND, 3, 0, siov, NULL, NULL);
}

int
RTCFUNC(init,ds3232)(struct chip_loc *chip, char *argv[])
{
    fd = open((argv && argv[0] && argv[0][0])? 
            argv[0]: DS3232_I2C_DEVNAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Unable to open I2C device\n");
        return -1;
    }
    return 0;
}

int
RTCFUNC(get,ds3232)(struct tm *tm, int cent_reg)
{
    unsigned char   date[7];

    ds3232_i2c_read(DS3232_SEC, date, 7);

    tm->tm_sec  = BCD2BIN(date[DS3232_SEC] & 0x7f);
    tm->tm_min  = BCD2BIN(date[DS3232_MIN] & 0x7f);

	if ((date[DS3232_HOUR] & 0x40)) {
		/* the rtc is in 12 hour mode */ 
		int hour = BCD2BIN(date[DS3232_HOUR] & 0x1f);

		if ((date[DS3232_HOUR] & 0x20))  
			tm->tm_hour = (hour == 12) ? 12 : hour + 12; /* pm */
		else
			tm->tm_hour = (hour == 12) ? 0 : hour;  	 /* am */

	} else { 
		/* rejoice! the rtc is in 24 hour mode */ 
     	tm->tm_hour = BCD2BIN(date[DS3232_HOUR] & 0x3f);
	}

    tm->tm_mday = BCD2BIN(date[DS3232_DATE] & 0x3f);
    tm->tm_mon  = BCD2BIN(date[DS3232_MONTH] & 0x1f) - 1;
    tm->tm_year = BCD2BIN(date[DS3232_YEAR] & 0xff);

	if ((date[DS3232_MONTH] & 0x80))  
		tm->tm_year += 100;

    tm->tm_wday = BCD2BIN(date[DS3232_DAY] & 0x7) - 1;

    return(0);
}

int
RTCFUNC(set,ds3232)(struct tm *tm, int cent_reg)
{
    unsigned char   date[7];

	/* 
	 * Note: this function will set the clock incorrectly after 2099 
	 * And it sets the clock in 24 hour mode 
	 */

    date[DS3232_SEC]   = BIN2BCD(tm->tm_sec); 
    date[DS3232_MIN]   = BIN2BCD(tm->tm_min);
    date[DS3232_HOUR]  = BIN2BCD(tm->tm_hour);
    date[DS3232_DATE]  = BIN2BCD(tm->tm_mday);
    date[DS3232_MONTH] = BIN2BCD(tm->tm_mon + 1);
    date[DS3232_YEAR]  = BIN2BCD(tm->tm_year % 100);
    date[DS3232_DAY]   = BIN2BCD(tm->tm_wday + 1);

	if (tm->tm_year >= 100) 
	 	date[DS3232_MONTH]|= 0x80;

    ds3232_i2c_write(DS3232_SEC, date, 7);

    return(0);
}
