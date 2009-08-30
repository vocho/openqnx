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
 *  seginfo.h   Segment information data structures
 *

 */
#ifndef __SEGINFO_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#if __WATCOMC__ > 1000
#pragma pack(push,1);
#else
#pragma pack(1);
#endif

struct _seginfo {
    short unsigned  selector,
        flags;
    long    addr,
        nbytes;
    } ;

/*
 *  Segment flag definitions        _PROC_SEGMENT --> msg.segment.bits
 *  The top 8 bits are read only.
 */

#define _PMF_DATA_RW     0x0000  /*  Data RW                               */
#define _PMF_DATA_R      0x0001  /*  Data R                                */
#define _PMF_CODE_RX     0x0002  /*  Code RX                               */
#define _PMF_CODE_X      0x0003  /*  Code Execute only                     */
#define _PMF_DMA_SAFE    0x0004  /*  Will not cross a 64K segmnt boundry   */
#define _PMF_VOVERLAY    0x0004  /*  Treat addr as virtual not physical    */
#define _PMF_MODIFY      0x0008  /*  Can modify access rights              */
#define _PMF_HUGE        0x0010  /*  Next segment is linked to this one    */
#define _PMF_GLOBAL_OWN  0x0020  /*  If global, make segment owned by me   */
#define _PMF_ALIGN       0x0040  /*  Align segment on a GET/PUT operatn    */
#define _PMF_GLOBAL      0x0080  /*  Make segment global                   */
#define _PMF_NOCACHE     0x0100  /*  Disable caching if possible           */
#define _PMF_DMA_HIGH    0x0200  /*  Allow dma requests above 16 meg       */
#define _PMF_LINKED      0x0400  /*  Segment is a link to an existing seg  */
#define _PMF_DBBIT       0x0800  /*  Set code/stack segment to USE32 deflt */
#define _PMF_SHARED      0x1000  /*  More than one process owns segment    */
#define _PMF_BORROWED    0x2000  /*  Segment which was taken from free lst */
#define _PMF_LOADING     0x4000  /*  Segment which has a cmd loading in it */
#define _PMF_INUSE       0x8000  /*  This segment entry is in use          */

#define _PMF_ALLOCMASK   0x03af  /*  Bits for qnx_segment_alloc_flags      */
#define _PMF_OVERLAYMASK 0x03af  /*  Bits for qnx_segment_overlay_flags    */
#define _PMF_GETMASK     0x004b  /*  Bits for qnx_segment_get              */
#define _PMF_PUTMASK     0x00cb  /*  Bits for qnx_segment_put              */
#define _PMF_FLAGSMASK   0x080b  /*  Bits for qnx_segment_flags            */

#ifdef __cplusplus
extern "C" {
#endif
extern unsigned  qnx_segment_alloc( long );
extern unsigned  qnx_segment_alloc_flags( long, unsigned );
extern unsigned  qnx_segment_realloc( unsigned, long );
extern unsigned  qnx_segment_huge( long );
extern unsigned  qnx_segment_overlay( long, long );
extern unsigned  qnx_segment_overlay_flags( long, long, unsigned );
extern int       qnx_segment_free( unsigned );
extern int       qnx_segment_arm( pid_t, unsigned, unsigned );
extern unsigned  qnx_segment_get( pid_t, unsigned, unsigned );
extern unsigned  qnx_segment_put( pid_t, unsigned, unsigned );
extern unsigned  qnx_segment_flags( unsigned, unsigned );
extern unsigned  qnx_segment_info( pid_t, pid_t, unsigned, struct _seginfo * );
extern unsigned  qnx_segment_index( pid_t, pid_t, unsigned );
extern unsigned  qnx_segment_raw_alloc( long, struct _seginfo * );
extern unsigned  qnx_segment_raw_free( struct _seginfo * );
extern unsigned  qnx_segment_priv( unsigned, unsigned );
#ifdef __cplusplus
};
#endif

#if __WATCOMC__ > 1000
#pragma pack(pop);
#else
#pragma pack();
#endif

#define __SEGINFO_H_INCLUDED
#endif
