/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

/*
 * dev_readex.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <errno.h>
#include <sys/dcmd_chr.h>
#include <mig4nto.h>

/*
 *  dev_readex
 *
 *	This function returns one byte of data for the devc-* drivers.  Other
 *	drivers could return more than one byte of exception data.  That is
 *	why the incoming buffer is memset to zeroes and after the devctl call
 *	the buffer is scanned for out of band characters in order to return 
 *	the number of bytes read.
 */
int
dev_readex(int __fd, char *__buf, int __nbytes)
{
	int		count, rval, rc;
	char	outofbandmask = 
				_OBAND_SER_OE |   /* Overrun Error */
				_OBAND_SER_PE |   /* Parity Error  */  
				_OBAND_SER_FE |   /* Framing Error */
				_OBAND_SER_BI;    /* Break         */

	if (__buf == NULL || __nbytes <= 0) {
		errno = EINVAL;
		return -1;
	}

	memset(__buf, 0, __nbytes);

	if ((rc = devctl(__fd, DCMD_CHR_GETOBAND, __buf, __nbytes, NULL)) != EOK) {
		if (rc == EAGAIN)
			return 0; /* no OOB data */
		errno = rc;
		return -1;
	}
	
	/* Count number of OOB data bytes seen. */
	for (count = 0, rval = 0; count < __nbytes; count++)
		if ((__buf[count] & outofbandmask) != 0)
			rval++;
	return rval;
}
