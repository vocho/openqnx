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
 *  _packpop.h: Restore structure packing to previous value
 *

 */

#if defined(__WATCOMC__)
 #pragma pack(__pop);
#elif defined(__HIGHC__)
 #pragma pop_align_members();
#elif defined(__GNUC__)
 #if defined(__PRAGMA_PACK_PUSH_POP__)
  #pragma pack(pop)
 #else 
  #pragma pack()
 #endif
#elif defined(__INTEL_COMPILER)
 #pragma pack(pop)
#elif defined(__MWERKS__)
#else
 #error Compiler not supported
#endif

/* __SRCVERSION("_packpop.h $Rev: 167337 $"); */
