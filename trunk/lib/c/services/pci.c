/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */




#include	<stdio.h>
#include	<stdlib.h>
#include	<stddef.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<errno.h>
#include	<hw/pci.h>
#include	<sys/iomsg.h>
#include	<sys/iomgr.h>
#include	<sys/types.h>
#include	<sys/neutrino.h>
#include	<sys/pci_serv.h>

typedef	struct	_pci_message	MSG;

static int _pci_fd = -1;
static int _pci_rcnt = 0;

/**************************************************************************/
/* Attach user application to the pci server. This must be the first call */
/* by the user application, otherwise none of the function calls will     */
/* work.                                                                  */
/**************************************************************************/

int		pci_attach (unsigned flags)

{
	flags = flags;
	if (!_pci_rcnt ) {
		if ((_pci_fd = open ("/dev/pci", O_RDWR)) == -1) {
			return (-1);
			}
		}
	_pci_rcnt++;
	return (_pci_fd);
}

/**************************************************************************/
/* Detach the user application from the pci server.                       */
/**************************************************************************/

int		pci_detach (unsigned handle)

{
int		status = 0;

	if (_pci_rcnt > 1 && handle != _pci_fd) {
		errno = EBADF;
		return (-1);
		}
	if (_pci_rcnt && !--_pci_rcnt ) {
		if ((status = close (handle)) == 0)
			_pci_fd = -1;
		else
			_pci_rcnt++;
		}
	return (status);
}

/**************************************************************************/
/* Send a request to the pci server.                                      */
/**************************************************************************/

int		_pci_send_message (int type, int length, MSG *msg)

{
int		size;
iov_t	iov, rx_iov;

	size = length + sizeof (struct _pci_reply_hdr) + sizeof (io_msg_t);
	msg->msg_hdr.i.type = _IO_MSG;
	msg->msg_hdr.i.subtype = type;
	msg->msg_hdr.i.mgrid = _IOMGR_PCI;
	msg->msg_hdr.i.combine_len = size;
	SETIOV (&iov, msg, size);
	SETIOV (&rx_iov, msg, sizeof (*msg));
	return (MsgSendv (_pci_fd, &iov, 1, &rx_iov, 1));
}

/**************************************************************************/
/* Find a pci device based on the device id and the vendor id.            */
/**************************************************************************/

int		pci_find_device (unsigned devid, unsigned venid, unsigned index,
                                unsigned *busnum, unsigned *devfuncnum)

{
struct	_pci_device		*device;
int		status;
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	device = &msg.msg.device;
	device->deviceid = devid;
	device->vendorid = venid;
	device->index = index;
	if (_pci_send_message (IOM_PCI_FIND_DEVICE, sizeof (struct _pci_device), &msg) == -1)
		return (-1);
	if ((status = msg.rep_hdr.reply_status) != PCI_SUCCESS)
		return (status);
	*busnum = msg.msg.device.busnum;
	*devfuncnum = msg.msg.device.devfuncnum;
	return (PCI_SUCCESS);
}

/**************************************************************************/
/* Read pci configuration space in bytes, words or double words depending */
/* on the size field.                                                     */
/**************************************************************************/

int		pci_read_config (void *handle, unsigned offset, unsigned cnt,
								size_t size, void *bufptr)

{
struct	_pci_config_hdl		*cfg;
int		status;
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	cfg = &msg.msg.config_hdl;
	cfg->handle = handle;
	cfg->offset = offset;
	cfg->count = cnt;
	cfg->size = size;
	if (_pci_send_message (IOM_PCI_READ_CONFIG_HANDLE, sizeof (struct _pci_config_hdl), &msg) == -1)
		return (-1);
	if ((status = msg.rep_hdr.reply_status) != PCI_SUCCESS)
		return (status);
	memcpy ((char *) bufptr, &msg.msg.config_hdl.buffer, cnt * size);
	return (PCI_SUCCESS);
}

/**************************************************************************/
/* Write to pci configuration space in bytes, words or double words       */
/* depending on the size field.                                           */
/**************************************************************************/

int		pci_write_config (void *handle, unsigned offset, unsigned cnt,
								size_t size, const void *bufptr)

{
struct	_pci_config_hdl		*cfg;
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	cfg = &msg.msg.config_hdl;
	cfg->handle = handle;
	cfg->offset = offset;
	cfg->count = cnt;
	cfg->size = size;
	memcpy (&cfg->buffer, (char *) bufptr, cnt * size);
	if (_pci_send_message (IOM_PCI_WRITE_CONFIG_HANDLE, sizeof (struct _pci_config_hdl), &msg) == -1)
		return (-1);
	return (msg.rep_hdr.reply_status);
}

/**************************************************************************/
/* Attach the user application to a specific device. A handle will be     */
/* returned to the user application which must be used for all further    */
/* configuration calls to the server. A pci_dev_info structure will be    */
/* filled in by the server and returned to the user application.          */
/**************************************************************************/

void	*pci_attach_device (void *handle, _uint32 flags, _uint16 idx, void *bufptr)

{
struct	_pci_attach	*atch;
int		status;
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	atch = &msg.msg.lock;
	atch->flags = flags;
	atch->handle = handle;
	atch->idx = idx;
	memcpy ((char *) &atch->configbuffer, (char *) bufptr, sizeof (struct pci_dev_info));
	if (_pci_send_message (IOM_PCI_ATTACH_DEVICE, sizeof (struct _pci_attach), &msg) == -1)
		return ((void *) NULL);
	if ((status = msg.rep_hdr.reply_status) != PCI_SUCCESS)
		return ((void *) NULL);
	memcpy ((char *) bufptr, (char *) &msg.msg.lock.configbuffer, sizeof (struct pci_dev_info));
	return (msg.msg.lock.handle);
}

/**************************************************************************/
/* Detach the user application from a device.                             */
/**************************************************************************/

int		pci_detach_device (void *handle)

{
struct	_pci_detach	*det;
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	det = &msg.msg.unlock;
	det->handle = handle;
	if (_pci_send_message (IOM_PCI_DETACH_DEVICE, sizeof (struct _pci_detach), &msg) == -1)
		return (-1);
	return (msg.rep_hdr.reply_status);
}

/**************************************************************************/
/* Low level read configuration byte, word or double word from pci space  */
/* for cnt.                                                               */
/**************************************************************************/

int		pci_read_config_bus (unsigned busnum, unsigned devfuncnum,
							unsigned offset, unsigned cnt, size_t size, void *bufptr)

{
struct	_pci_config	*cfg;
int		status, func;
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	cfg = &msg.msg.config;
	cfg->bus = busnum;
	cfg->devfunc = devfuncnum;
	cfg->offset = offset;
	cfg->count = cnt;
	if (size == 1)
		func = IOM_PCI_READ_CONFIG_BYTE;
	else
		if (size == 2)
			func = IOM_PCI_READ_CONFIG_WORD;
		else
			func = IOM_PCI_READ_CONFIG_DWORD;
	if (_pci_send_message (func, sizeof (struct _pci_config), &msg) == -1)
		return (-1);
	if ((status = msg.rep_hdr.reply_status) != PCI_SUCCESS)
		return (status);
	memcpy ((char *) bufptr, &msg.msg.config.buffer, cnt * size);
	return (PCI_SUCCESS);
}

/**************************************************************************/
/* Low level write to configuration space for cnt bytes, words or double  */
/* words.                                                                 */
/**************************************************************************/

int		pci_write_config_bus (unsigned busnum, unsigned devfuncnum,
						  unsigned offset, unsigned cnt, size_t size, const void *bufptr)

{
struct	_pci_config	*cfg;
int		func;
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	cfg = &msg.msg.config;
	cfg->bus = busnum;
	cfg->devfunc = devfuncnum;
	cfg->offset = offset;
	cfg->count = cnt;
	memcpy (&cfg->buffer, (char *) bufptr, cnt * size);
	if (size == 1)
		func = IOM_PCI_WRITE_CONFIG_BYTE;
	else
		if (size == 2)
			func = IOM_PCI_WRITE_CONFIG_WORD;
		else
			func = IOM_PCI_WRITE_CONFIG_DWORD;
	if (_pci_send_message (func, sizeof (struct _pci_config), &msg) == -1)
		return (-1);
	return (msg.rep_hdr.reply_status);
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/

int		pci_generate_special_cycle (unsigned Busnum, unsigned long SpecialCycleData)

{
	return (PCI_SUCCESS);
}

/**************************************************************************/
/* Find a pci device based on its class code.                             */
/**************************************************************************/

int		pci_find_class (unsigned long ClassCode, unsigned index, unsigned *busnum,
						unsigned *devfuncnum)

{
struct	_pci_class	*class;
int		status;
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	class = &msg.msg.class;
	class->class = ClassCode;
	class->index = index;
	if (_pci_send_message (IOM_PCI_FIND_CLASS, sizeof (struct _pci_class), &msg) == -1)
		return (-1);
	if ((status = msg.rep_hdr.reply_status) != PCI_SUCCESS)
		return (status);
	*busnum = msg.msg.class.busnum;
	*devfuncnum = msg.msg.class.devfuncnum;
	return (status);
}

/**************************************************************************/
/* Get the last bus, version and hardware fields from the pci server.     */
/**************************************************************************/

int		pci_present (unsigned *lastbus, unsigned *version, unsigned *hardware)

{
struct	_pci_present	*pres;
int		status;
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	pres = &msg.msg.present;
	if (_pci_send_message (IOM_PCI_BIOS_PRESENT, sizeof (struct _pci_present), &msg) == -1)
		return (-1);
	if ((status = msg.rep_hdr.reply_status) != PCI_SUCCESS)
		return (status);
	if (lastbus)
		*lastbus = msg.msg.present.lastbus;
	if (version)
		*version = msg.msg.present.version;
	if (hardware)
		*hardware = msg.msg.present.hardware;
	return (status);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

int		pci_map_irq (unsigned busnum, unsigned devfuncnum, short intno, short intpin)

{
struct	_pci_map_irq	*map;
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	map = &msg.msg.map_irq;
	map->bus = busnum;
	map->devfunc = devfuncnum;
	map->intno = intno;
	map->intpin = intpin;
	if (_pci_send_message (IOM_PCI_MAP_IRQ, sizeof (struct _pci_map_irq), &msg) == -1)
		return (-1);
	return (msg.rep_hdr.reply_status);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

int		pci_rescan_bus (void)

{
MSG		msg;

	memset ((char *) &msg, 0, sizeof (msg));
	if (_pci_send_message (IOM_PCI_RESCAN_BUS, 0, &msg) == -1)
		return (-1);
	return (msg.rep_hdr.reply_status);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

int		pci_irq_routing_options (IRQRoutingOptionsBuffer *buf, uint32_t *irq)

{
MSG		msg;
struct	_pci_route_opt	*rte;
int		status;
char	*data;

	memset ((char *) &msg, 0, sizeof (msg));
	rte = &msg.msg.route_opt;
	rte->buf_size = buf->BufferSize;
	if (_pci_send_message (IOM_PCI_IRQ_ROUTING_OPTIONS, sizeof (struct _pci_route_opt), &msg) == -1)
		return (-1);
	if ((status = msg.rep_hdr.reply_status) != PCI_SUCCESS) {
		return (status);
		}
	buf->BufferSize = rte->buf_size;
	data = (char *) buf + sizeof (*buf);
	memcpy (data, rte->buffer, rte->buf_size);
	*irq = rte->irq_info;
	return (status);
}

__SRCVERSION("pci.c $Rev: 153052 $");
