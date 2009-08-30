/*
 * $QNXLicenseC:  
 * Copyright 2005, QNX Software Systems. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software 
 * Systems (QSS) and its licensors.  Any use, reproduction, modification, 
 * disclosure, distribution or transfer of this software, or any software 
 * that includes or is based upon any of this code, is prohibited unless 
 * expressly authorized by QSS by written agreement.  For more information 
 * (including whether this source code file has been published) please
 * email licensing@qnx.com. $
*/


#include "rtc.h"
#include <time.h>
#include <arm/pxa250.h>


/* 
 * Dallas/Maxim DS1339 Serial RTC
 */
#define DS1339_SEC			0	/* 00-59 */
#define DS1339_MIN			1	/* 00-59 */
#define DS1339_HOUR			2	/* 0-1/00-23 */
#define DS1339_DAY			3	/* 01-07 */
#define DS1339_DATE			4	/* 01-31 */
#define DS1339_MONTH		5	/* 01-12 */
#define DS1339_YEAR			6	/* 00-99 */


#define DS1339_I2C_ADDRESS	0x68
#define	I2C_READ			1
#define	I2C_WRITE			0

#define GPIO_bit(x)			(1 << ((x) & 0x1f))
#define	RTC_SCL_GPIO		3
#define	RTC_SDA_GPIO		4
#define	GPIO_OUT			0x80
#define	GPIO_IN				0x00

#define	i2c_gpio_bit_delay(x)	nanospin_ns(x * 5000);

#define i2c_gpio_set_clk(value)		i2c_gpio_set_pin(RTC_SCL_GPIO, value)
#define i2c_gpio_set_data(value)	i2c_gpio_set_pin(RTC_SDA_GPIO, value)
#define i2c_gpio_get_data()			gpio_tst_bit(RTC_SDA_GPIO)
#define i2c_gpio_read_ack()			(i2c_gpio_read_bit() == 0)
#define i2c_gpio_write_ack(ack)		i2c_gpio_write_bit(!ack)

#define i2c_gpio_pin_dir(out) {                 \
	if (out) {		\
		if (!(chip_read(PXA250_GPDR0, 32) & GPIO_bit(RTC_SDA_GPIO)))	\
			pxa_gpio_mode(RTC_SDA_GPIO | GPIO_OUT);	\
	}	\
	else {			\
		if ((chip_read(PXA250_GPDR0, 32) & GPIO_bit(RTC_SDA_GPIO)))	\
			pxa_gpio_mode(RTC_SDA_GPIO | GPIO_IN);		\
	}	\
}

#define	i2c_gpio_set_pin(gpio, value) {	\
	if (value)	\
		chip_write(PXA250_GPSR0, GPIO_bit(gpio), 32);	\
	else	\
		chip_write(PXA250_GPCR0, GPIO_bit(gpio), 32);	\
}

#define i2c_gpio_startbit() {	\
	i2c_gpio_set_data(1);		\
	i2c_gpio_pin_dir(1);		\
	i2c_gpio_set_clk(1);		\
	i2c_gpio_bit_delay(1);		\
	i2c_gpio_set_data(0);		\
	i2c_gpio_bit_delay(1);		\
	i2c_gpio_set_clk(0);		\
	i2c_gpio_bit_delay(1);		\
}

#define i2c_gpio_stop() {		\
	i2c_gpio_set_data(0);		\
	i2c_gpio_pin_dir(1);		\
	i2c_gpio_set_clk(1);		\
	i2c_gpio_bit_delay(1);		\
	i2c_gpio_set_data(1);		\
	i2c_gpio_bit_delay(1);		\
}

#define i2c_gpio_write_bit(value) {     \
	i2c_gpio_set_data(value);	\
	i2c_gpio_pin_dir(1);		\
	i2c_gpio_bit_delay(1);		\
	i2c_gpio_set_clk(1);		\
	i2c_gpio_bit_delay(1);		\
	i2c_gpio_set_clk(0);		\
	i2c_gpio_bit_delay(1);		\
}


static inline int gpio_tst_bit(int gpio)
{
	return (chip_read(PXA250_GPLR0, 32) & GPIO_bit(gpio)) ? 1 : 0;
}

static void pxa_gpio_mode(int gpio_mode)
{
	unsigned gpdr;

	InterruptDisable();
	gpdr = chip_read(PXA250_GPDR0, 32);

	if (gpio_mode & GPIO_OUT)
		gpdr |= GPIO_bit(gpio_mode);
	else
		gpdr &= ~GPIO_bit(gpio_mode);

	chip_write(PXA250_GPDR0, gpdr, 32);
	InterruptEnable();
}

static int i2c_gpio_init(void)
{
	int result = 0;

	i2c_gpio_set_clk(1);
	pxa_gpio_mode(RTC_SCL_GPIO | GPIO_OUT);

	i2c_gpio_set_data(0);
	i2c_gpio_pin_dir(1);
	i2c_gpio_bit_delay(10);	

	i2c_gpio_pin_dir(0);
	i2c_gpio_bit_delay(10);	
	if (i2c_gpio_get_data() == 0)
		result = -ENODEV;

	i2c_gpio_set_data(1);
	i2c_gpio_pin_dir(1);
	i2c_gpio_bit_delay(10);	

	return result;
}

static inline int i2c_gpio_read_bit()
{
	int bit;

	i2c_gpio_set_data(1);
	i2c_gpio_pin_dir(0);
	i2c_gpio_bit_delay(1);
	bit = i2c_gpio_get_data();
	i2c_gpio_set_clk(1);
	i2c_gpio_bit_delay(1);
	i2c_gpio_set_clk(0);
	i2c_gpio_bit_delay(1);

	return bit;
}

static int i2c_gpio_read_byte()
{
	int data = 0;
	int i;

	for (i = 0; i < 8; i++)
		data = (data << 1) | i2c_gpio_read_bit();

	i2c_gpio_write_ack(0);

	return data;
}

static int i2c_gpio_write_byte(unsigned char data)
{
	int result = 0;
	int i;

	for (i = 7; i >= 0; i--)
		i2c_gpio_write_bit(data & (1 << i));

	if (!i2c_gpio_read_ack()) 
		result = -ENODEV;

	return result;
}

static int i2c_gpio_start(unsigned char chip_id, int rw)
{
	i2c_gpio_startbit();

	return i2c_gpio_write_byte((chip_id << 1) | rw);
}

static int ds1339_read_reg(unsigned char reg)
{
	int result;

	result = i2c_gpio_init();
	if (result != 0)
		return result;

	result = i2c_gpio_start(DS1339_I2C_ADDRESS, I2C_WRITE);

	if (result == 0)
		result = i2c_gpio_write_byte(reg);
	else
		return result;

	i2c_gpio_stop();

	result = i2c_gpio_start(DS1339_I2C_ADDRESS, I2C_READ);

	if (result == 0) {
		result = i2c_gpio_read_byte();
		if (result < 0) 
			return result;
	}
	else 
		return result;

	i2c_gpio_stop();

	return result;
}

static int ds1339_write_reg(int regno, unsigned char val)
{
	int result;

	result = i2c_gpio_init();
	if (result != 0)
		return result;

	result = i2c_gpio_start(DS1339_I2C_ADDRESS, I2C_WRITE);
	if (result == 0)
		result = i2c_gpio_write_byte(regno);

	if (result == 0) {
		result = i2c_gpio_write_byte(val);
		if (result != 0)
			fprintf(stderr, "Failed to write %02x to reg %02x of DS1339\n", val, regno);
	}

	i2c_gpio_stop();

	return result;
}


int
RTCFUNC(init,triton)(struct chip_loc *chip, char *argv[])
{
	if (chip->phys == NIL_PADDR)
		chip->phys = PXA250_GPIO_BASE;

	if (chip->access_type == NONE)
		chip->access_type = MEMMAPPED;

	chip->reg_shift = 0;

	return PXA250_GPIO_SIZE;
}

int
RTCFUNC(get,triton)(struct tm *tm, int cent_reg)
{
	unsigned char	reg, date[7];

	do {
		for (reg = 0; reg <= 6; reg++)
			date[reg] = ds1339_read_reg(reg);
	} while (date[DS1339_SEC] != ds1339_read_reg(DS1339_SEC));

	tm->tm_sec  = BCD2BIN(date[DS1339_SEC]);
	tm->tm_min  = BCD2BIN(date[DS1339_MIN]);
	tm->tm_hour = BCD2BIN(date[DS1339_HOUR] & 0x3F);
	tm->tm_mday = BCD2BIN(date[DS1339_DATE]);
	tm->tm_mon  = BCD2BIN(date[DS1339_MONTH] & 0x1F) - 1;
	tm->tm_year = BCD2BIN(date[DS1339_YEAR]);
	tm->tm_wday = BCD2BIN(date[DS1339_DAY]) - 1;

	if (tm->tm_year < 70) 
		tm->tm_year += 100;

	return (0);
}

int
RTCFUNC(set,triton)(struct tm *tm, int cent_reg)
{
	unsigned char	reg, date[7];

	date[DS1339_SEC]   = BIN2BCD(tm->tm_sec); 
	date[DS1339_MIN]   = BIN2BCD(tm->tm_min);
	date[DS1339_HOUR]  = BIN2BCD(tm->tm_hour);
	date[DS1339_DATE]  = BIN2BCD(tm->tm_mday);
	date[DS1339_MONTH] = BIN2BCD(tm->tm_mon + 1);
	date[DS1339_YEAR]  = BIN2BCD(tm->tm_year % 100);
	date[DS1339_DAY]   = BIN2BCD(tm->tm_wday + 1);

	for (reg = 0; reg <= 6; reg++)
		ds1339_write_reg(reg, date[reg]);

	return (0);
}
