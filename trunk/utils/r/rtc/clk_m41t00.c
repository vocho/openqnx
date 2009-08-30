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
 * STM M41T00 Serial Access Timekeeper
 */
#define M41T00_SEC          0   /* 00-59 */
#define M41T00_MIN          1   /* 00-59 */
#define M41T00_HOUR         2   /* 0-1/00-23 */
#define M41T00_DAY          3   /* 01-07 */
#define M41T00_DATE         4   /* 01-31 */
#define M41T00_MONTH        5   /* 01-12 */
#define M41T00_YEAR         6   /* 00-99 */
#define M41T00_CTRL         7

#define M41T00_SEC_ST       0x80    /* oscillator Stop bit */
#define M41T00_HOUR_CEB     0x80    /* Century Enable bit */
#define M41T00_HOUR_CB      0x40    /* Century bit */

#define M41T00_I2C_ADDRESS  (0xD0 >> 1)
#define M41T00_I2C_DEVNAME  "/dev/i2c0"

/*
 * Century bit convention:
 * 0 = 2000
 * 1 = 1900 OR 2100
 *  if (year < 70)
 *      2100
 *  else
 *      1900
 */

static int fd = -1;

static int
m41t00_i2c_read(unsigned char reg, unsigned char val[], unsigned char num)
{
    iov_t           siov[2], riov[2];
    i2c_sendrecv_t  hdr;

    hdr.slave.addr = M41T00_I2C_ADDRESS;
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
m41t00_i2c_write(unsigned char reg, unsigned char val[], unsigned char num)
{
    iov_t           siov[3];
    i2c_send_t      hdr;

    hdr.slave.addr = M41T00_I2C_ADDRESS;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.len = num + 1;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));
    SETIOV(&siov[1], &reg, sizeof(reg));
    SETIOV(&siov[2], val, num);

    return devctlv(fd, DCMD_I2C_SEND, 3, 0, siov, NULL, NULL);
}

int
RTCFUNC(init,m41t00)(struct chip_loc *chip, char *argv[])
{
    fd = open((argv && argv[0] && argv[0][0])? 
            argv[0]: M41T00_I2C_DEVNAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Unable to open I2C device\n");
        return -1;
    }
    return 0;
}

int
RTCFUNC(get,m41t00)(struct tm *tm, int cent_reg)
{
    unsigned char   date[7];
    unsigned char   century;

    m41t00_i2c_read(M41T00_SEC, date, 7);

    tm->tm_sec  = BCD2BIN(date[M41T00_SEC] & ~M41T00_SEC_ST);
    tm->tm_min  = BCD2BIN(date[M41T00_MIN]);
    tm->tm_hour = BCD2BIN(date[M41T00_HOUR] & 
                          ~(M41T00_HOUR_CEB|M41T00_HOUR_CB));
    tm->tm_mday = BCD2BIN(date[M41T00_DATE]);
    tm->tm_mon  = BCD2BIN(date[M41T00_MONTH]) - 1;
    tm->tm_year = BCD2BIN(date[M41T00_YEAR]);
    tm->tm_wday = BCD2BIN(date[M41T00_DAY]) - 1;

    if (date[M41T00_HOUR] & M41T00_HOUR_CEB) {
        century = date[M41T00_HOUR] & M41T00_HOUR_CB;
        if (century) {
            if (tm->tm_year < 70) 
                tm->tm_year += 200;
        } else {
            tm->tm_year += 100;
        }
    } else {
        if(tm->tm_year < 70) 
            tm->tm_year += 100;
    }
    return(0);
}

int
RTCFUNC(set,m41t00)(struct tm *tm, int cent_reg)
{
    unsigned char   date[7];
    char   century;

    date[M41T00_SEC]   = BIN2BCD(tm->tm_sec); /* implicitly clears stop bit */
    date[M41T00_MIN]   = BIN2BCD(tm->tm_min);
    date[M41T00_HOUR]  = BIN2BCD(tm->tm_hour) | M41T00_HOUR_CEB;
    date[M41T00_DAY]   = BIN2BCD(tm->tm_wday + 1);
    date[M41T00_DATE]  = BIN2BCD(tm->tm_mday);
    date[M41T00_MONTH] = BIN2BCD(tm->tm_mon + 1);
    date[M41T00_YEAR]  = BIN2BCD(tm->tm_year % 100);

    century = tm->tm_year / 100 - 1;
    if (century != 0)
        date[M41T00_HOUR] |= M41T00_HOUR_CB;

    m41t00_i2c_write(M41T00_SEC, date, 7);

    return(0);
}
