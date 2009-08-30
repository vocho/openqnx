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
 *  hw/inout.h
 *

 *
 */
#ifndef _HW_INOUT_INCLUDED
#define _HW_INOUT_INCLUDED

#ifndef _GULLIVER_H_INCLUDED
#include <gulliver.h>
#endif

#ifndef __CPUINLINE_H_INCLUDED
#include <sys/cpuinline.h>
#endif

#include _NTO_CPU_HDR_(inout.h)

#ifndef mem_barrier
#define mem_barrier()				__cpu_membarrier()
#endif

#ifndef inle16
#define inle16(__port)             (ENDIAN_LE16(in16((__port))))
#endif
#ifndef inle32
#define inle32(__port)             (ENDIAN_LE32(in32((__port))))
#endif
#ifndef inbe16
#define inbe16(__port)             (ENDIAN_BE16(in16((__port))))
#endif
#ifndef inbe32
#define inbe32(__port)             (ENDIAN_BE32(in32((__port))))
#endif
#ifndef outle16
#define outle16(__port, __val)     (out16((__port), ENDIAN_LE16((__val))))
#endif
#ifndef outle32
#define outle32(__port, __val)     (out32((__port), ENDIAN_LE32((__val))))
#endif
#ifndef outbe16
#define outbe16(__port, __val)     (out16((__port), ENDIAN_BE16((__val))))
#endif
#ifndef outbe32
#define outbe32(__port, __val)     (out32((__port), ENDIAN_BE32((__val))))
#endif

#endif

/* __SRCVERSION("inout.h $Rev: 153052 $"); */
