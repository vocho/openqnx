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



#ifndef QKCP_IF_H
#define QKCP_IF_H

struct qkcp_shmem
{
	uint64_t	job_size;      // in kilobytes
	uint64_t	job_size_done; 
	uint32_t	job_files;
	uint32_t	job_files_done;
	uint64_t	job_time;      // in seconds
	uint64_t	job_time_left;
	int     	percent;
	int     	phase;
	int     	status;
};

#endif

/* __SRCVERSION("qkcp_if.h $Rev: 157947 $"); */








