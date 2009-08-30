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
#include <ppc/mgt5200_cpu.h>

#define RTC_REGISTER_SIZE 32

/***************************************************************************
* Function : init_mgt5200
*
* Description: Setup the mgt/mpc5200 RTC 
*
* Notes:       
*
**************************************************************************/
int
RTCFUNC(init,mgt5200)(struct chip_loc *chip, char *argv[])
{
    if (chip->phys == NIL_PADDR)
    {
        /* Set the base address to the MBAR_BASE that way we can use simple
           register offsets allready defined. 
        */
        chip->phys = MGT5200_MBAR_BASE; 
    }
    if (chip->century_reg == UNSET)
    {
        /* There is no century register avaliable year is stored in 12 bits */
        chip->century_reg = -1;     
                                       
    }

    if (chip->access_type == NONE)
    {
        chip->access_type = MEMMAPPED;
    }

    chip->reg_shift = 0;
    
    /* return size of memory to be mapped */
    return(0x20);                   
}

/***************************************************************************
* Function : get_mgt5200
*
* Description: Get the current time from the RTC in the mpc5200
*
* Notes:       
*
**************************************************************************/
int
RTCFUNC(get,mgt5200)(struct tm *tm, int cent_reg)
{
    unsigned int current_time;
    unsigned int current_date;

    current_time = chip_read(MGT5200_MBAR_RTC_CTIME, RTC_REGISTER_SIZE);
    current_date = chip_read(MGT5200_MBAR_RTC_CDATE, RTC_REGISTER_SIZE);

    tm->tm_sec  = (current_time & MGT5200_RTC_CTIME_SEC_MASK) 
                  >> MGT5200_RTC_CTIME_SEC_SHIFT;
    
    tm->tm_min  = (current_time & MGT5200_RTC_CTIME_MIN_MASK) 
                  >> MGT5200_RTC_CTIME_MIN_SHIFT;

    tm->tm_hour = (current_time & MGT5200_RTC_CTIME_HRS_MASK) 
                  >> MGT5200_RTC_CTIME_HRS_SHIFT;

    tm->tm_mday = (current_date & MGT5200_RTC_CDATE_DAY_MASK) 
                  >> MGT5200_RTC_CDATE_DAY_SHIFT;

    tm->tm_mon  = ((current_date & MGT5200_RTC_CDATE_MON_MASK) 
                  >> MGT5200_RTC_CDATE_MON_SHIFT) - 1;

    tm->tm_year = ((current_date & MGT5200_RTC_CDATE_YEAR_MASK) 
                  >> MGT5200_RTC_CDATE_YEAR_SHIFT) - 1900;  

    return(0);
}

/***************************************************************************
* Function : set_mgt5200
*
* Description: Set the time in the mpc5200 RTC
*
* Notes:       
*
**************************************************************************/
int
RTCFUNC(set,mgt5200)(struct tm *tm, int cent_reg)
{
    unsigned int shadow_stime, time;
    unsigned int shadow_sdate, date;
    unsigned int shadow_syear, year;
    
    shadow_stime = chip_read(MGT5200_MBAR_RTC_STIME, RTC_REGISTER_SIZE);
    shadow_sdate = chip_read(MGT5200_MBAR_RTC_SDATE, RTC_REGISTER_SIZE);
    shadow_syear = chip_read(MGT5200_MBAR_RTC_SYEAR, RTC_REGISTER_SIZE);

    /* Create the time date and year bit patterns */
    time =
    (
      ((tm->tm_hour << MGT5200_MBAR_RTC_STIME_HOUR_SHIFT) /* Update hours */
        & MGT5200_MBAR_RTC_STIME_HOUR_MASK) |
      ((tm->tm_min << MGT5200_MBAR_RTC_STIME_MIN_SHIFT)   /* Update minutes */
        & MGT5200_MBAR_RTC_STIME_MIN_MASK) |
      ((tm->tm_sec << MGT5200_MBAR_RTC_STIME_SEC_SHIFT)   /* Update seconds */
        & MGT5200_MBAR_RTC_STIME_SEC_MASK)
    );
    
    /* Updatem month. tm_mon is months since January */
    date =
    (
      ((tm->tm_mon + 1 << MGT5200_MBAR_RTC_SDATE_MON_SHIFT) 
        & MGT5200_MBAR_RTC_SDATE_MON_MASK) |
      ((tm->tm_mday << MGT5200_MBAR_RTC_SDATE_DAY_SHIFT)    /* Update day */
        & MGT5200_MBAR_RTC_SDATE_DAY_MASK)
    );
    
    /* Update year. tm_year is given as years since 1900, store
       full 4 digit year in hardware */
    year =
    (
      (((tm->tm_year + 1900) << MGT5200_MBAR_RTC_SYEAR_YEAR_SHIFT) 
        & MGT5200_MBAR_RTC_SYEAR_YEAR_MASK)
    );


    /* Clear out the old values in the set registers */   
    shadow_stime = ((shadow_stime
                & ~(MGT5200_MBAR_RTC_STIME_HOUR_MASK |
                    MGT5200_MBAR_RTC_STIME_MIN_MASK |
                    MGT5200_MBAR_RTC_STIME_SEC_MASK)
                ) | time);

    shadow_sdate = ((shadow_sdate
                    & ~(MGT5200_MBAR_RTC_SDATE_MON_MASK |
                        MGT5200_MBAR_RTC_SDATE_WDAY_MASK |
                        MGT5200_MBAR_RTC_SDATE_DAY_MASK)
                    ) | date);
    
    shadow_syear = ((shadow_syear
                    & ~(MGT5200_MBAR_RTC_SYEAR_YEAR_MASK)) | year); 

    /* Ouput the year now to the year set regiser, the date/time
       will get done when we toggle the pause/set registers */
    chip_write(MGT5200_MBAR_RTC_SYEAR, shadow_syear, RTC_REGISTER_SIZE);

    /* Now we need to sequence the engine to grab the updated values
       Sequence is:
       1) pause_time = 1 set_time = 0
       2) pause_time = 1 set_time = 1
       3) pause_time = 1 set_time = 0
       3) pause_time = 0 set_time = 0

       The sequence should be 4 writes so use only 4 simple writes
       the sequence may be interrupted during read.

       Note: You cannot interlace the write to the set/pause bits
             for the date and time set registers you have to do one full
             sequence on the time register and then one full sequence 
             on the date register.
    */ 

   shadow_stime &= ~(MGT5200_MBAR_RTC_STIME_SET_MASK
                     | MGT5200_MBAR_RTC_STIME_PAUSE_MASK);
   chip_write(MGT5200_MBAR_RTC_STIME, shadow_stime, RTC_REGISTER_SIZE);

   /* 1) Set the pause bit  */
   shadow_stime |= (MGT5200_MBAR_RTC_STIME_PAUSE_MASK);
   chip_write(MGT5200_MBAR_RTC_STIME, shadow_stime, RTC_REGISTER_SIZE);

   /* 2) Set the set bit  */
   shadow_stime |= (MGT5200_MBAR_RTC_STIME_SET_MASK);
   chip_write(MGT5200_MBAR_RTC_STIME, shadow_stime, RTC_REGISTER_SIZE);

   /* 3) clear the set bit  */
   shadow_stime &= ~(MGT5200_MBAR_RTC_STIME_SET_MASK);
   chip_write(MGT5200_MBAR_RTC_STIME, shadow_stime, RTC_REGISTER_SIZE);

   /* 4) clear the pause bit  */
   shadow_stime &= ~(MGT5200_MBAR_RTC_STIME_PAUSE_MASK);
   chip_write(MGT5200_MBAR_RTC_STIME, shadow_stime, RTC_REGISTER_SIZE);


   /* Now do the date */
   /* Set the set and pause bits to a known state 0 and 0 */
   shadow_sdate &= ~(MGT5200_MBAR_RTC_SDATE_SET_MASK
                     | MGT5200_MBAR_RTC_SDATE_PAUSE_MASK);
   chip_write(MGT5200_MBAR_RTC_SDATE, shadow_sdate, RTC_REGISTER_SIZE);

   /* 1) Set the pause bit  */
   shadow_sdate |= (MGT5200_MBAR_RTC_SDATE_PAUSE_MASK);
   chip_write(MGT5200_MBAR_RTC_SDATE, shadow_sdate, RTC_REGISTER_SIZE);

   /* 2) Set the set bit  */
   shadow_sdate |= (MGT5200_MBAR_RTC_SDATE_SET_MASK);
   chip_write(MGT5200_MBAR_RTC_SDATE, shadow_sdate, RTC_REGISTER_SIZE);

   /* 3) clear the set bit  */
   shadow_sdate &= ~(MGT5200_MBAR_RTC_SDATE_SET_MASK);
   chip_write(MGT5200_MBAR_RTC_SDATE, shadow_sdate, RTC_REGISTER_SIZE);

   /* 4) clear the pause bit  */
   shadow_sdate &= ~(MGT5200_MBAR_RTC_SDATE_PAUSE_MASK);   
   chip_write(MGT5200_MBAR_RTC_SDATE, shadow_sdate, RTC_REGISTER_SIZE);

   return(0);
}
