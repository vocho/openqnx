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

#define MC9S08DZ60_REAL_TIME_DATA		2

#define MC9S08DZ60_SEC			    	0    /* 00-59 */
#define MC9S08DZ60_MIN			    	1    /* 00-59 */
#define MC9S08DZ60_HOUR		            2    /* 00-23 */
#define MC9S08DZ60_DAY		            3    /* 01-07 */
#define MC9S08DZ60_DATE				    4    /* 00-30 */
#define MC9S08DZ60_MONTH				5    /* 00-11 */
#define MC9S08DZ60_YEAR			        6    /* 00-99 */

#define MC9S08DZ60_CLK_SRC		(1 << 7)
#define MC9S08DZ60_RTC_RUN		(1 << 7)

#define MC9S08DZ60_I2C_ADDRESS		0x69
#define MC9S08DZ60_I2C_DEVNAME		"/dev/i2c0"

static int fd = -1;

static int
mc9s08dz60_i2c_read(uint8_t reg, uint8_t *val, int num)
{
    iov_t           siov[2], riov[2];
    i2c_sendrecv_t  hdr;

    hdr.slave.addr = MC9S08DZ60_I2C_ADDRESS;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.send_len = 1;
    hdr.recv_len = num;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));
    SETIOV(&siov[1], &reg, 1);

    SETIOV(&riov[0], &hdr, sizeof(hdr));
    SETIOV(&riov[1], val, num);

    return devctlv(fd, DCMD_I2C_SENDRECV, 2, 2, siov, riov, NULL);
}

static int
mc9s08dz60_i2c_write(uint8_t reg, uint8_t *val, int num)
{
    iov_t           siov[3];
    i2c_send_t      hdr;

    hdr.slave.addr = MC9S08DZ60_I2C_ADDRESS;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.len = num + 1;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));
    SETIOV(&siov[1], &reg, 1);
    SETIOV(&siov[2], val, num);

    return devctlv(fd, DCMD_I2C_SEND, 3, 0, siov, NULL, NULL);
}

int
RTCFUNC(init,mc9s08dz60)(struct chip_loc *chip, char *argv[])
{
	uint8_t		ctrl;

    fd = open((argv && argv[0] && argv[0][0])? 
            argv[0]: MC9S08DZ60_I2C_DEVNAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Unable to open I2C device\n");
        return -1;
    }

    if (mc9s08dz60_i2c_read(MC9S08DZ60_REAL_TIME_DATA, &ctrl, 1) != EOK) {
    	fprintf(stderr, "Unable to read data from I2C device\n");
    	close(fd);
    	return -1;
    }

	/* start the RTC if not already started */
	ctrl |= MC9S08DZ60_RTC_RUN;

    if (mc9s08dz60_i2c_write(MC9S08DZ60_REAL_TIME_DATA, &ctrl, 1) != EOK) {
    	fprintf(stderr, "Unable to write data to I2C device\n");
    	close(fd);
    	return -1;
    }

    return 0;
}

int
RTCFUNC(get,mc9s08dz60)(struct tm *tm, int cent_reg)
{
    uint8_t		date[7];

    if (mc9s08dz60_i2c_read(MC9S08DZ60_REAL_TIME_DATA, date, 7)) {
    	fprintf(stderr, "Unable to read data from I2C device\n");
    	return -1;
    }

	tm->tm_sec  = BCD2BIN(date[MC9S08DZ60_SEC]   & 0x7f);
    tm->tm_min  = BCD2BIN(date[MC9S08DZ60_MIN]   & 0x7f);
    tm->tm_hour = BCD2BIN(date[MC9S08DZ60_HOUR]  & 0x3f);
    tm->tm_mday = BCD2BIN(date[MC9S08DZ60_DATE]  & 0x3f) + 1;
    tm->tm_mon  = BCD2BIN(date[MC9S08DZ60_MONTH] & 0x1f);
    tm->tm_year = BCD2BIN(date[MC9S08DZ60_YEAR]);
    tm->tm_wday = BCD2BIN(date[MC9S08DZ60_DAY]   & 0x7) - 1;

	/* no century bit in RTC, so this will have to do for now */
	if (tm->tm_year < 70) 
		tm->tm_year += 100;

    return(0);
}

int
RTCFUNC(set,mc9s08dz60)(struct tm *tm, int cent_reg)
{
    uint8_t		date[7];

    date[MC9S08DZ60_SEC]   = BIN2BCD(tm->tm_sec) | MC9S08DZ60_RTC_RUN;
    date[MC9S08DZ60_MIN]   = BIN2BCD(tm->tm_min);
	date[MC9S08DZ60_HOUR]  = BIN2BCD(tm->tm_hour);
    date[MC9S08DZ60_DAY]   = BIN2BCD(tm->tm_wday + 1);
    date[MC9S08DZ60_DATE]  = BIN2BCD(tm->tm_mday - 1);
    date[MC9S08DZ60_MONTH] = BIN2BCD(tm->tm_mon);
    date[MC9S08DZ60_YEAR]  = BIN2BCD(tm->tm_year % 100);

    if (mc9s08dz60_i2c_write(MC9S08DZ60_REAL_TIME_DATA, date, 7) != EOK) {
    	fprintf(stderr, "Unable to write data to I2C device\n");
    	return -1;
    }

    return(0);
}
