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





// We need basename and setenv, which do not exist equally on all
// platforms.  

#define gen_mkdir mkdir

#if defined(__QNXNTO__)
// setenv() included with stdlib.h
#include <libgen.h>

#elif defined(__QNX__) || defined(__NT__)
#include <env.h>
#undef gen_mkdir
#define gen_mkdir(path,perm) mkdir(path)

#elif defined(__MINGW32__)
#undef gen_mkdir
#define gen_mkdir(path,perm) mkdir(path)

#elif defined(linux)
// setenv() is in stdlib.h
#include <libgen.h>

#elif defined(__SOLARIS__)
// setenv() in libcompat.a for solaris, included in lib/compat.h above
#include <libgen.h>

#else

#error What host are you trying to compile this for?

#endif
