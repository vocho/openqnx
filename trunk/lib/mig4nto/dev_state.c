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
 * dev_state.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <errno.h>
#include <devctl.h>
#include <sys/dcmd_chr.h>
#include <mig4nto.h>

static int check_for_input(int fd, unsigned int *eventbits);
static int check_if_drained(int fd, unsigned int *eventbits);
static int check_if_exrdy(int fd, unsigned int *eventbits);
static int check_for_output(int fd, unsigned int *eventbits);

/*
 *  dev_state
 *
 *  Note: This only allows you to query the current state.  The
 *        __bits and __mask parameters are ignored.
 *  
 *  The following can be returned:
 *
 *	_DEV_EVENT_INPUT    Input is available from the device.
 *	_DEV_EVENT_DRAIN    The output has drained on this device.
 *	_DEV_EVENT_EXRDY    An exception or out-of-bound character is
 *						available to be read with dev_readex()
 *	_DEV_EVENT_OUTPUT   There's room in the output buffer to transmit
 *						N chars, ( by default, N is 1 ).
 */
unsigned
dev_state(int __fd, unsigned __bits, unsigned __mask)
{
	unsigned int	oldeventbits = 0;

	if (check_for_input(__fd, &oldeventbits))
		return -1;
	if (check_if_drained(__fd, &oldeventbits))
		return -1;
	if (check_if_exrdy(__fd, &oldeventbits))
		return -1;
	if (check_for_output(__fd, &oldeventbits))
		return -1;

	return oldeventbits;
}

/*
 *  check_for_output
 *
 *  Note: The size of the output buffer is Resource Manager dependent 
 *        and CANNOT be changed via dev_osize() using the migration
 *        functions.
 */
static int
check_for_output(int fd, unsigned int *eventbits)
{
	int			rc;
	unsigned    bufsize;
	unsigned    nbytesinbuf;

	if ((rc = devctl(fd, DCMD_CHR_OSSIZE, &bufsize,
								sizeof(bufsize), NULL)) != EOK) {
		errno = rc;
		return -1;
	}

	if ((rc = devctl(fd, DCMD_CHR_OSCHARS, &nbytesinbuf,
								sizeof(nbytesinbuf), NULL)) != EOK) {
		errno = rc;
		return -1;
	}
	if (bufsize == nbytesinbuf)
		(*eventbits) &= ~(_DEV_EVENT_OUTPUT);
	else
		(*eventbits) |= _DEV_EVENT_OUTPUT;
	return 0;
}

static int
check_if_exrdy(int fd, unsigned int *eventbits)
{
	int		rc;
	char	outofbanddata;
	char	outofbandmask =
				_OBAND_SER_OE |   /* Overrun Error */
				_OBAND_SER_PE |   /* Parity Error  */  
				_OBAND_SER_FE |   /* Framing Error */
				_OBAND_SER_BI;    /* Break         */

	rc = devctl(fd, DCMD_CHR_GETOBAND, &outofbanddata, sizeof(char), NULL);
	if (rc == EOK) {
		if (outofbanddata & outofbandmask) {	/* OOB Data seen. */
			(*eventbits) |= _DEV_EVENT_EXRDY;

			/* Put the OOB data back, as there is no way to detect 
			   the data is there except by reading it.  */
			if ((rc = devctl(fd, DCMD_CHR_PUTOBAND, &outofbanddata,
							sizeof(char), NULL)) != EOK) {
				errno = rc;
				return -1;
			}
		} else {
			(*eventbits) &= ~(_DEV_EVENT_EXRDY);
		}
	} else if (rc == EAGAIN || rc == ENOSYS) {
			/* this resource manager doesn't have Out-of-band */
			(*eventbits) &= ~(_DEV_EVENT_INTR);
	} else {
		errno = rc;
		return -1;
	}
	return 0;
}

static int
check_if_drained(int fd, unsigned int *eventbits)
{
	int			rc;
	unsigned	nbytesinbuf;

	if ((rc = devctl(fd, DCMD_CHR_OSCHARS, &nbytesinbuf,
						sizeof(nbytesinbuf), NULL)) != EOK) {
		errno = rc;
		return -1;
	}
	if (nbytesinbuf == 0)
		(*eventbits) |= _DEV_EVENT_DRAIN;
	else
		(*eventbits) &= ~(_DEV_EVENT_DRAIN);
	return 0;
}

static int
check_for_input(int fd, unsigned int *eventbits)
{
	int			rc;
	unsigned    nbytesinbuf;

	if ((rc = devctl(fd, DCMD_CHR_ISCHARS, &nbytesinbuf,
						sizeof(nbytesinbuf), NULL)) != EOK) {
		errno = rc;
		return -1;
	}
	if (nbytesinbuf > 0)
		(*eventbits) |= _DEV_EVENT_INPUT;
	else
		(*eventbits) &= ~(_DEV_EVENT_INPUT);
	return 0;
}
