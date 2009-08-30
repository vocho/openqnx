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
 * RA-8581 Serial RTC on GE Energy UCSA board
 */
#define RA8581_SEC          0
#define RA8581_MIN          1
#define RA8581_HOUR         2
#define RA8581_DAY          3
#define RA8581_DATE         4
#define RA8581_MONTH        5
#define RA8581_YEAR         6
#define RA8581_CTRL        15 


#define PCA9544_I2C_ADDRESS	(0xE0 >> 1)
#define RA8581_I2C_ADDRESS  (0xA2 >> 1)
#define RA8581_I2C_DEVNAME  "/dev/i2c0"


static int fd = -1;

static int
ucsa_i2c_read(uint8_t reg, uint8_t val[], uint8_t num)
{
    iov_t           siov[2], riov[2];
	i2c_send_t		hdr0;
    i2c_sendrecv_t  hdr;
	int				status;
	uint8_t			ctrl = 5;

	/*
	 * Lock I2C bus
	 */
    status = devctl(fd, DCMD_I2C_LOCK, NULL, 0, NULL);
	if (status)
		return status;

    hdr0.slave.addr = PCA9544_I2C_ADDRESS;
    hdr0.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr0.len = 1;
    hdr0.stop = 1;

    SETIOV(&siov[0], &hdr0, sizeof(hdr0));
    SETIOV(&siov[1], &ctrl, sizeof(ctrl));

	/*
	 * Select RA-8581
	 */
    status = devctlv(fd, DCMD_I2C_SEND, 2, 0, siov, NULL, NULL);
	if (status)
		goto done;

	/*
	 * Wait for PCA9544 to settle down
	 */
	delay(10);

    hdr.slave.addr = RA8581_I2C_ADDRESS;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.send_len = 1;
    hdr.recv_len = num;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));
    SETIOV(&siov[1], &reg, sizeof(reg));

    SETIOV(&riov[0], &hdr, sizeof(hdr));
    SETIOV(&riov[1], val, num);

	/*
	 * Read RTC
	 */
    status = devctlv(fd, DCMD_I2C_SENDRECV, 2, 2, siov, riov, NULL);

done:
    devctl(fd, DCMD_I2C_UNLOCK, NULL, 0, NULL);

	return status;
}

static int
ucsa_i2c_write(uint8_t reg, uint8_t val[], uint8_t num)
{
    iov_t		siov[3];
    i2c_send_t	hdr;
	int			status;
	uint8_t		ctrl = 5;

	/*
	 * Lock I2C bus
	 */
    status = devctl(fd, DCMD_I2C_LOCK, NULL, 0, NULL);
	if (status)
		return status;

    hdr.slave.addr = PCA9544_I2C_ADDRESS;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.len = 1;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));
    SETIOV(&siov[1], &ctrl, sizeof(ctrl));

	/*
	 * Select RA-8581
	 */
    status = devctlv(fd, DCMD_I2C_SEND, 2, 0, siov, NULL, NULL);
	if (status)
		goto done;

	/*
	 * Wait for PCA9544 to settle down
	 */
	delay(10);

    hdr.slave.addr = RA8581_I2C_ADDRESS;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.len = num + 1;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));
    SETIOV(&siov[1], &reg, sizeof(reg));
    SETIOV(&siov[2], val, num);

	/*
	 * Set RTC
	 */
    status = devctlv(fd, DCMD_I2C_SEND, 3, 0, siov, NULL, NULL);

done:
    devctl(fd, DCMD_I2C_UNLOCK, NULL, 0, NULL);
	return status;
}

int
RTCFUNC(init,ucsa)(struct chip_loc *chip, char *argv[])
{
    fd = open((argv && argv[0] && argv[0][0])? 
            argv[0]: RA8581_I2C_DEVNAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Unable to open I2C device\n");
        return -1;
    }
    return 0;
}

int
RTCFUNC(get,ucsa)(struct tm *tm, int cent_reg)
{
    uint8_t   date[7];

    if (ucsa_i2c_read(RA8581_SEC, date, 7))
		return (-1);

    tm->tm_sec  = BCD2BIN(date[RA8581_SEC]);
    tm->tm_min  = BCD2BIN(date[RA8581_MIN]);
    tm->tm_hour = BCD2BIN(date[RA8581_HOUR]);
    tm->tm_mday = BCD2BIN(date[RA8581_DATE]);
    tm->tm_mon  = BCD2BIN(date[RA8581_MONTH]) - 1;
    tm->tm_year = BCD2BIN(date[RA8581_YEAR]);
   	for (tm->tm_wday = 0; tm->tm_wday < 7; tm->tm_wday++) {
		if (date[RA8581_DAY] & (1 << tm->tm_wday))
			break;
	}

    if (tm->tm_year < 70) 
        tm->tm_year += 100;

    return (0);
}

int
RTCFUNC(set,ucsa)(struct tm *tm, int cent_reg)
{
    uint8_t   date[7];

    date[RA8581_SEC]   = BIN2BCD(tm->tm_sec); 
    date[RA8581_MIN]   = BIN2BCD(tm->tm_min);
    date[RA8581_HOUR]  = BIN2BCD(tm->tm_hour);
    date[RA8581_DATE]  = BIN2BCD(tm->tm_mday);
    date[RA8581_MONTH] = BIN2BCD(tm->tm_mon + 1);
    date[RA8581_YEAR]  = BIN2BCD(tm->tm_year % 100);
    date[RA8581_DAY]   = 1 << tm->tm_wday;

    ucsa_i2c_write(RA8581_SEC, date, 7);

    return (0);
}
