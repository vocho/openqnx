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


/*
 * kevfile.h
 * 
 * kevfile operations
 * 
 */	
 
/*	
	int kevfile_open
		path
		flags
		maximum size of file
		number of buffers to allocate
		
		create kevfile
		writes header
		
		allocates buffers, populates buffer list
		
		optionally pre-grows in-memory file
		
	kevfile_close
		flushes file
		potentially truncates pre-grown file
		closes file
		
	kevfile_get_buffer
		gets the next free buffer from the free list
		
	kevfile_put_buffer
		if direct mapped
			advances data pointer to next free buffer slot in mapped file
			adds to free list
		else
			puts the filled buffer into the write list
		
	kevfile_write
		writes buffers from writelist to kevfile
		nop for direct mapped case
*/	

#ifndef __KEVFILE_H__
#define __KEVFILE_H__
#include <sys/types.h>
#include <sys/neutrino.h>
#include <inttypes.h>

#define _TRACE_MAKE_CODE(c,f,cl,e)  (((c)<<24)|(f)|(cl)|(e))
#if defined(__MIPS__) || defined(__SH__)
#define CLOCKCYCLES (ClockCycles()>>CLOCKCYCLES_INCR_BIT)
#else
#define CLOCKCYCLES (ClockCycles())
#endif

#define KEVFILE_MAP 0x00000001

typedef struct kevfile_buf kevfile_buf_t;
struct kevfile_buf {
	kevfile_buf_t	*next;
	size_t			nbytes;
	unsigned char	*data;
	unsigned		off;
};
typedef struct kevfile kevfile_t;
struct kevfile {
	const char	*path;
	int			fd;
	uint8_t		*addr; /* RUSH - check if Thomas is happy on drugs */
	off_t		current_offset;
	unsigned	flags;
	size_t		max_size;
	
	unsigned	buffers_logged;
	unsigned	buffers_dropped;
	unsigned	last_buffers_dropped;
	
	unsigned	num_buffers;
	intrspin_t		buf_spin;
	kevfile_buf_t	*free_buffers;
	kevfile_buf_t	*write_buffers;
	kevfile_buf_t	*write_tail;
	unsigned		write_depth;
	unsigned		max_write_depth;
};

__BEGIN_DECLS

extern kevfile_t		*kevfile_open( const char *path, unsigned flags, size_t max_size, unsigned initial_buffers );	
extern void				kevfile_close( kevfile_t *kevfile );
extern kevfile_buf_t	*kevfile_buffer_get( kevfile_t *kevfile );	
extern int				kevfile_buffer_put( kevfile_t *kevfile, kevfile_buf_t *buf );	
extern int				kevfile_flush( kevfile_t *kevfile );
extern unsigned			kevfile_space_left( kevfile_t *kevfile );

#define KEVFILE_FLUSH_NBUFS	1
#define kevfile_needs_flush( kev ) ( !((kev)->flags & KEVFILE_MAP) && (kev)->write_depth > KEVFILE_FLUSH_NBUFS )
	
__END_DECLS

#endif /* __KEVFILE_H__ */
	

/* __SRCVERSION("kevfile.h $Rev: 153052 $"); */
