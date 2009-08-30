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



#ifndef _LIB43_PROTO_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif
extern int      dev_insert_chars( int __fd, int __n, const char *__buf );
extern unsigned dev_mode( int __fd, unsigned __mask, unsigned __mode );
extern int      dev_read( int __fd, void *__buf, unsigned __nbytes,
                  unsigned __minimum, unsigned __time, unsigned __timeout,
                  pid_t __proxy, int *__triggered );
extern int      dev_size( int __fd, int __set_rows, int __set_columns,
                  int *__rows, int *__cols );
#ifdef __cplusplus
};
#endif

#define _LIB43_PROTO_H_INCLUDED
#endif

