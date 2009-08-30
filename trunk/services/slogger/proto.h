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



 


void options(int argc, char *argv[]);
void slogger_init(struct slogdev *trp);
void *logger(void *dummy);
void check_overrun(struct slogdev *trp, int *ptr, int cnt);
int wait_add(struct slogdev *trp, int rcvid, int priority);

extern resmgr_io_funcs_t io_funcs;
extern resmgr_connect_funcs_t connect_funcs;

int io_read(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb);
int io_unblock(resmgr_context_t *ctp, io_pulse_t *msg, iofunc_ocb_t *ocb);
int io_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra);
int io_console_write(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb);

/* __SRCVERSION("proto.h $Rev: 153052 $"); */
