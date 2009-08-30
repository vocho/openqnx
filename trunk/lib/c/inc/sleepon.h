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





/* Forward declare this */
struct _sleepon_entry;

/*
 * Since we export _sleepon_default to users, the size of
 * this structure can not change within minor shared lib changes.
 * That is why we but some reserved's in...
 */
struct _sleepon_handle {
	pthread_mutex_t			mutex;
	struct _sleepon_entry	*list;
	struct _sleepon_entry	*free;
	unsigned				flags;
	unsigned				inuse;
	int						reserved[6];
}; 

/* __SRCVERSION("sleepon.h $Rev: 153052 $"); */
