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





#ifndef _IO_H_
#define _IO_H_

#include <stdio.h>

/*
 * Device types (_dev_type)
 */

#define TYPE_FILE	0
#define TYPE_DEVICE	1

/*
 * Open Modes and File Attributes/Permissions
 */

#define _READ			0x01
#define _WRITE			0x02
#define _APPEND			0x04
#define _CREATE			0x04
#define _EXECUTE		0x08
#define _BLOCK			0x08
#define _MODIFY			0x10
#define _DIRECTORY		0x20

#define _NEW			0x01
#define _OLD			0x02
#define _FILE			0x04
#define _VERBOSE		0x08
#define _QUIT			0x10
#define _CACHE			0x20

/*
 * IOCB flag bits (_flags)
 * _READ and _WRITE are also used in _flags
 */

#define _EOF			0x80

#define _UNBUF			0x40
#define _TRUNCATED		0x40
#define _UNGOT			0x20
#define _DIRTY			0x10
#define _FILTHY			0x08
#define _EOF_PENDING	0x08
#define _SLIMY			0x04
#define _NOWAIT			0x04

/*
 * This is the basic IO control block. It is the minimum
 * required information for IO transfers.
 * Device Buffers are 80 characters, File buffers are 512.
 */

struct iocb {
	struct iocb *_link;
	char _unchar;
	char _dev_type;
	char _cmd_stat;
	unsigned _arg[2];
	char _flags;
	char _dev_no;
	int _index;
	int _length;
	char _drive_number;
	char _iocb_spare;
	unsigned _admin_tid;
	char _buffer[0];
	};

struct msg_hdr {
	char mtype;
	unsigned marg[2];
	char mflags;
	char mdev_no;
	int mindex;
	int mlen;
	char mdrive_number;
	char miocb_spare;
	unsigned mtid;
	char mbuf[0];
	} ;

#define FMSG_HDR sizeof(struct msg_hdr)

/*
 * Messages to either FSYS or DEV
 */

#define TASK_DIED			0
#define FILL				1
#define EMPTY				2
#define OPEN				3
#define CLOSE				4
#define SEEK				5
#define REWIND				6
#define FORWARD				7
#define GET_ATTR			8
#define SET_ATTR			9
#define CLOSE_ALL			10
#define ABORT_MSG			11
#define CHAR_WAITING		12
#define GET_OPTION			13
#define READ_BLK			13
#define SET_OPTION			14
#define WRITE_BLK			14
#define DISK_PROMPT			15
#define LOAD				15
#define NEW_OPTION			16
#define CD					16
#define NEW_WINDOW			17
#define PASS_CD				17
#define ADOPT_DEVICE		18
#define DEFINE_DRIVER		18
#define INHERIT_DEVICE		19
#define GET_SEARCH			19
#define UNADOPT_DEVICE		20
#define SET_SEARCH			20
#define QUERY_DEVICE		21
#define GET_DIR_ENTRY		21
#define SET_DIR_ENTRY		22
#define NACC_GET_TTY		22
#define LSEEK				23
#define NACC_SET_TTY		23
#define MOUNT_XHDR_CACHE	24
#define DEV_TTY_TID			24
#define MOUNT_CACHE			25
#define DEV_TTY_ROOT_TID	25
#define PWD					26
#define MULTI_CHAR_WAITING  26
#define GET_SEARCH_ORDER	27
#define MULTI_CHAR_WAITING_NW	27
#define FTELL				28
#define __WINDOW_SIZE		28
#define FORMAT_TRACK		29
#define FLUSH_INPUT			29
#define STTY_MESSAGE		30
#define PURGE_CACHE			30
#define INSERT_CHAR			31
#define ADOPT_DRIVE			31
#define GET_RSEARCH			32
#define GET_TTY_TIME		32
#define SET_RSEARCH			33
#define DEV_SET_KEYBOARD	33
#define LREAD_BLK			34
#define __WINDOW_CREATE		34
#define LWRITE_BLK			35
#define LGET_ATTR			36
#define NACC_GET_DISK		37
#define NACC_SET_DISK		38
#define MULTI_SECTOR		39
#define FILE_STATUS			40
#define FSYS_SEARCH_ORDER	41
#define START_FSYS			42
#define TRUNCATE_FILE		43
#define MOUNT_BITMAP_CACHE	44
#define MOVE_FILE			45
#define FILE_DOTS			46
#define DISK_CTRL			47
#define DISK_USER			48

/*
 * Replies
 */

#define IO_ERROR			0x80
#define RETRY_OPEN			0x40
#ifndef	OK
#define OK					0
#endif
#define RETRY				1
#define READ_ERROR			2
#define WRITE_ERROR			3
#define FULL_DISK			4
#define BAD_XTNT			5
#define FILE_WRITE_ERROR	6
#define BAD_FILE_NUMBER     7
#define BAD_IOCB			8
#define COMMAND_ABORTED		9
#define NO_ACCESS			10
#define NO_DATA				11

#endif _IO_H_
