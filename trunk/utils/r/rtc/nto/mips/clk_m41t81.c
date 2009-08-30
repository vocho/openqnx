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

/*
 * RTC Register numbers
 */

#define M41T81REG_TSC   0x00        /* tenths/hundredths of second */
#define M41T81REG_SC    0x01        /* seconds */
#define M41T81REG_MN    0x02        /* minute */
#define M41T81REG_HR    0x03        /* hour/century */
#define M41T81REG_DY    0x04        /* day of week */
#define M41T81REG_DT    0x05        /* date of month */
#define M41T81REG_MO    0x06        /* month */
#define M41T81REG_YR    0x07        /* year */
#define M41T81REG_CTL   0x08        /* control */
#define M41T81REG_WD    0x09        /* watchdog */
#define M41T81REG_AMO   0x0A        /* alarm: month */
#define M41T81REG_ADT   0x0B        /* alarm: date */
#define M41T81REG_AHR   0x0C        /* alarm: hour */
#define M41T81REG_AMN   0x0D        /* alarm: minute */
#define M41T81REG_ASC   0x0E        /* alarm: second */
#define M41T81REG_FLG   0x0F        /* flags */
#define M41T81REG_SQW   0x13        /* square wave register */

#define M41T81_RTC_SIZE 0x14
#define M41T81_CENT     0x40
#define M41T81_CEB      0x80

/*
 * SMB BUS Register Numbers
 */
#define SMB_FREQ1       0x001C
#define SMB_CMD1        0x003c
#define SMB_CTRL1       0x006c
#define SMB_STAT1       0x002c
#define SMB_DATA1       0x005c
#define SMB_STRT1       0x004c

/*
 * Misc SMB definitions
 */
#define FREQ_400KHZ     0x1F
#define SMB_BUSY        0x01
#define SMB_ERR         0x02
#define SMB_STRT_WR1B   0x0000
#define SMB_STRT_WR2B   0x0100
#define SMB_STRT_RD1B   0x0500
#define RTC_SLAVE_ADDR  0x68

#define BCM_REG_BASE	0x10060000

/*
 * Clock setup for the RTC on the BCM1X80 (MIPS core).
 */

static int wait_busy()
{
    uint64_t status;

    for (;;) {
        status = chip_read(SMB_STAT1,32) & 0xFF;
        if (status & SMB_BUSY)
            continue;
        break;
    }

    if (status & SMB_ERR) {
        chip_write(SMB_STAT1, (status & SMB_ERR), 32);
        return -1;
    }

    return 0;
}


static int smbus_readrtc(int slaveaddr,int devaddr)
{
    uint32_t err,data;

    if (wait_busy() < 0)
        return -1;

    chip_write(SMB_CMD1, ((devaddr & 0xFF) & 0xFF),32);
    chip_write(SMB_STRT1, SMB_STRT_WR1B | slaveaddr,32);

    err = wait_busy();
    if (err < 0)
        return err;

    /*
     * Read the data
     */
    chip_write(SMB_STRT1, SMB_STRT_RD1B | slaveaddr, 32);


    err = wait_busy();
    if (err < 0)
        return err;

    data =  chip_read(SMB_DATA1,32);

    return (data & 0xFF);
}


static int smbus_writertc(int slaveaddr,int devaddr,int b)
{
    int err;

    /*
     * Make sure the bus is idle (probably should
     * ignore error here)
     */

    if (wait_busy() < 0) return -1;

    /*
     * Write the device address to the controller. There are two
     * parts, the high part goes in the "CMD" field, and the
     * low part is the data field.
     */

	chip_write(SMB_CMD1,((devaddr & 0xFF) & 0xFF),32);

	chip_write(SMB_DATA1,((b & 0xFF) & 0xFF),32);

    /*
     * Start the command.  Keep pounding on the device until it
     * submits or the timer expires, whichever comes first.  The
     * datasheet says writes can take up to 10ms, so we'll give it 500.
     */

    chip_write(SMB_STRT1,SMB_STRT_WR2B|slaveaddr,32);

    /*
     * Wait till the SMBus interface is done
     */

    err = wait_busy();
    if (err < 0) 
		return err;

    return err;
}



int
RTCFUNC(init,m41t81)(struct chip_loc *chip, char *argv[]) {
    if (chip->phys == NIL_PADDR) {
        chip->phys = BCM_REG_BASE;
    }
    if (chip->access_type == NONE) {
        chip->access_type = MEMMAPPED;
    }
    return M41T81_RTC_SIZE;
}

int
RTCFUNC(get,m41t81)(struct tm *tm, int cent_reg) {
	unsigned	sec;
	unsigned	min;
	unsigned	hour;
	unsigned	mday;
	unsigned	mon;
	unsigned	year;


	// convert BCD to binary 
	sec 	= smbus_readrtc(RTC_SLAVE_ADDR,M41T81REG_SC) & 0xFF;
	min 	= smbus_readrtc(RTC_SLAVE_ADDR,M41T81REG_MN) & 0xFF;	
	hour	= smbus_readrtc(RTC_SLAVE_ADDR,M41T81REG_HR) & ~(M41T81_CENT | M41T81_CEB);
	mday	= smbus_readrtc(RTC_SLAVE_ADDR,M41T81REG_DT) & 0xFF;
	mon		= smbus_readrtc(RTC_SLAVE_ADDR,M41T81REG_MO) & 0xFF;
	year	= smbus_readrtc(RTC_SLAVE_ADDR,M41T81REG_YR) & 0xFF;

	tm->tm_sec 	= BCD2BIN(sec);
	tm->tm_min 	= BCD2BIN(min);
	tm->tm_hour	= BCD2BIN(hour);
	tm->tm_mday	= BCD2BIN(mday);
	tm->tm_mon	= BCD2BIN(mon) - 1;
	tm->tm_year	= BCD2BIN(year);

	if ( smbus_readrtc(RTC_SLAVE_ADDR,M41T81REG_HR) & M41T81_CENT )
		tm->tm_year += 100;

	return(0);
}

int
RTCFUNC(set,m41t81)(struct tm *tm, int cent_reg) {
	unsigned	seconds;
	unsigned	minutes;
	unsigned	hours;
	unsigned	day;
	unsigned	month;
	unsigned	year;

	// convert binary to BCD
	seconds	= BIN2BCD(tm->tm_sec);
	minutes	= BIN2BCD(tm->tm_min);
	hours	= BIN2BCD(tm->tm_hour);
	day 	= BIN2BCD(tm->tm_mday);
	month	= BIN2BCD(tm->tm_mon) + 1;
	year	= BIN2BCD(tm->tm_year % 100);

	if ( tm->tm_year > 100 )
		hours |= M41T81_CENT;
	else
		hours &= ~M41T81_CENT;

	smbus_writertc(RTC_SLAVE_ADDR,M41T81REG_SC,(seconds & 0xff));
	smbus_writertc(RTC_SLAVE_ADDR,M41T81REG_MN,(minutes & 0xff));
	smbus_writertc(RTC_SLAVE_ADDR,M41T81REG_HR,(hours & 0xff));
	smbus_writertc(RTC_SLAVE_ADDR,M41T81REG_DT,(day & 0xff));
	smbus_writertc(RTC_SLAVE_ADDR,M41T81REG_MO,(month & 0xff));
	smbus_writertc(RTC_SLAVE_ADDR,M41T81REG_YR,(year & 0xff));

	return(0);
}
