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
 * S35390 register offsets
 */
#define S35390_STATUS_1				0
#define S35390_STATUS_2				1
#define S35390_REAL_TIME_DATA_1		2
#define S35390_REAL_TIME_DATA_2		3
#define S35390_INT1_1				4
#define S35390_INT1_2				5
#define S35390_CLOCK_ADJUST			6
#define S35390_FREE					7

/* 
 * S35390 real-time data register byte offsets
 */
#define S35390_YEAR         0   /* 00-99 */
#define S35390_MONTH        1   /* 01-12 */
#define S35390_DATE         2   /* 01-31 */
#define S35390_DAY          3   /* 00-06 */
#define S35390_HOUR         4   /* 00-23 */
#define S35390_MIN          5   /* 00-59 */
#define S35390_SEC          6   /* 00-59 */

#define S35390_STS_24HR		0x02
#define S35390_RTD_PM		0x40

#define S35390_I2C_ADDRESS  0x30
#define S35390_I2C_DEVNAME  "/dev/i2c1"

static int fd = -1;

/*
 * Fixes data retrieved from S35390A.
 * We need to flip (mirror) bits within each byte.
 */
static void
s35390_fixup(uint8_t *val, int num)
{
	int i, j;
	uint8_t temp1, temp2;

	for (i = 0; i < num; i++) {
		temp1 = 0;
		for (j = 0; j < 8; j++) {
			temp2 = (val[i] >> j) & 1;
			temp1 |= (temp2 << (7 - j));
		}
		val[i] = temp1;
	}
}

static int
s35390_i2c_read(uint8_t reg, uint8_t *val, int num)
{
	struct {
		i2c_recv_t recv;
		uint8_t rbuf[7];
	} receive;
	
	int i;

	receive.recv.slave.addr = S35390_I2C_ADDRESS + reg;
	receive.recv.slave.fmt = I2C_ADDRFMT_7BIT;
	receive.recv.len = num;
	receive.recv.stop = 1;
	if (devctl(fd, DCMD_I2C_RECV, &receive, sizeof(receive), NULL) != EOK)
		return -1;

	for (i = 0; i < num; i++)
		val[i] = receive.rbuf[i];
		
	return 0;
}

static int
s35390_i2c_write(uint8_t reg, uint8_t *val, int num)
{
	struct {
		i2c_send_t send;
		uint8_t sbuf[7];
	} send;
	
	int i;
	
	for (i = 0; i < num; i++)
		send.sbuf[i] = val[i];

	send.send.slave.addr = S35390_I2C_ADDRESS + reg;
	send.send.slave.fmt = I2C_ADDRFMT_7BIT;
	send.send.len = num;
	send.send.stop = 1;
	if (devctl(fd, DCMD_I2C_SEND, &send, sizeof(send), NULL) != EOK)
		return -1;

	return 0;
}

int
RTCFUNC(init,s35390)(struct chip_loc *chip, char *argv[])
{
    fd = open((argv && argv[0] && argv[0][0])? 
            argv[0]: S35390_I2C_DEVNAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Unable to open I2C device\n");
        return -1;
    }
    return 0;
}

int
RTCFUNC(get,s35390)(struct tm *tm, int cent_reg)
{
    uint8_t		date[7];
    uint8_t		status = 0;

    if (s35390_i2c_read(S35390_REAL_TIME_DATA_1, date, 7)) {
    	fprintf(stderr, "Unable to read data from I2C device\n");
    	return -1;
    }

	s35390_fixup(date, 7);

    if (s35390_i2c_read(S35390_STATUS_1, &status, 1)) {
    	fprintf(stderr, "Unable to read data from I2C device\n");
    	return -1;
    }

	s35390_fixup(&status, 1);

    tm->tm_sec  = BCD2BIN(date[S35390_SEC]);
    tm->tm_min  = BCD2BIN(date[S35390_MIN]);
    tm->tm_hour = BCD2BIN(date[S35390_HOUR] & 0x3f);
    tm->tm_mday = BCD2BIN(date[S35390_DATE]);
    tm->tm_mon  = BCD2BIN(date[S35390_MONTH]) - 1;
    tm->tm_year = BCD2BIN(date[S35390_YEAR]);
    tm->tm_wday = BCD2BIN(date[S35390_DAY]);

	/* adjust hour data if RTC is using a 12-hour clock */
    if (!(status & S35390_STS_24HR)) {
    	if (date[S35390_HOUR] & S35390_RTD_PM)
			tm->tm_hour += 12;
    }

	/* no century bit in RTC, so this will have to do for now */
	if (tm->tm_year < 70) 
		tm->tm_year += 100;

    return(0);
}

int
RTCFUNC(set,s35390)(struct tm *tm, int cent_reg)
{
    uint8_t		date[7];
    uint8_t		status = 0;

    if (s35390_i2c_read(S35390_STATUS_1, &status, 1)) {
    	fprintf(stderr, "Unable to read data from I2C device\n");
    	return -1;
    }

	s35390_fixup(&status, 1);

    date[S35390_SEC]   = BIN2BCD(tm->tm_sec);
    date[S35390_MIN]   = BIN2BCD(tm->tm_min);

	/* adjust hour data if RTC is using a 12-hour clock */
    if (status & S35390_STS_24HR)
	    date[S35390_HOUR]  = BIN2BCD(tm->tm_hour);
    else
	    date[S35390_HOUR]  = (tm->tm_hour < 11) ? BIN2BCD(tm->tm_hour) : (S35390_RTD_PM | BIN2BCD(tm->tm_hour - 12));

    date[S35390_DAY]   = BIN2BCD(tm->tm_wday);
    date[S35390_DATE]  = BIN2BCD(tm->tm_mday);
    date[S35390_MONTH] = BIN2BCD(tm->tm_mon + 1);
    date[S35390_YEAR]  = BIN2BCD(tm->tm_year % 100);

	s35390_fixup(date, 7);

    if (s35390_i2c_write(S35390_REAL_TIME_DATA_1, date, 7) != EOK) {
    	fprintf(stderr, "Unable to write data to I2C device\n");
    	return -1;
    }

    return(0);
}
