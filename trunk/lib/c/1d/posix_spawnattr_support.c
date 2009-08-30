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

#include <unistd.h>		// determines whether _POSIX_SPAWN is defined or not
#ifdef _POSIX_SPAWN

#include "posix_spawnattr.h"


/*
 * default_posix_spawnattr_t
 * 
 * The default 'posix_spawnattr_t' object will be used in the following cases...
 * 	1.	to return default value for any _getxxx() for which a user 'posix_spawnattr_t'
 * 		object has not yet been created. A user object will only be created
 * 		(memory will only be allocated) if a call to one of the _setxxx() functions
 * 		is made. This avoids unnecessary allocations for objects which will only
 * 		have default values.
 * 	2.	to provide the 'posix_spawnattr_t' values in a posix_spawn() call if
 * 		the 'attrp' argument is NULL
 * 
 * Note re __attribute__((aligned(8))) for _default_posix_spawnattr_t
 * 
 * We need this variable to be 8 byte aligned for the same reason we expect
 * any allocated posix_spawnattr_t memory within libc to be 8 byte aligned
 * (see KEY_MASK). The WATCOM (x86 only) compiler will does not understand the
 * __attribute__ keyword and so it potentially will not align
 * _default_posix_spawnattr_t to 8 bytes. If this is the case, then posix_spawn()
 * will assert in posix_spawnattr_t_once_init(). In this case I will have to
 * either reduce the KEY_MASK to 4 bytes alignment for a WATCOM compile or use
 * the same technique used within procnto, again for a WATCOM compile only.
 * Until I actually see this problem, I am not going to code a specific WATCOM
 * solution.
*/
#if defined(__WATCOMC__)
const _posix_spawnattr_t _default_posix_spawnattr_t =
#else	/* defined(__WATCOMC__) */
const _posix_spawnattr_t _default_posix_spawnattr_t __attribute__((aligned(8))) =
#endif	/* defined(__WATCOMC__) */

{
	0,				// flags - POSIX says no flags set is the default
	0,				// process group - POSIX says default value is 0
	ND_LOCAL_NODE,	// node on which to spawn - non POSIX
	{	// struct {} stack;
		0,			// maximum stack size - non POSIX. default is no maximum
	},
	{	// struct {} sig;
		{{0, 0}},	// signals which should be forced to use default signal handling - POSIX says empty set
		{{0, 0}},	// signal mask of the new process - POSIX says unspecified. Leave as empty set
		{{0, 0}},	// signals to be ignored - non POSIX. Leave as empty set
	},
	{	// struct {} sched;
		{NULL},		// sched_param - POSIX says default is unspecified. Leave NULL so inherited
		0,			// scheduling policy - POSIX says default is unspecified. Leave 0 so inherited
		(~0),		// runmask - non POSIX. Default is any available CPU
	},
	NULL			// partitioning attributes - non POSIX. Default is no partitions specified
};

/* default_posix_spawnattr_t is available externally to posix_spawn() */
posix_spawnattr_t default_posix_spawnattr_t = ((unsigned)0);


/*
 * =============================================================================
 * 
 * 							Internal Support Routines
 * 
 * =============================================================================
*/

/*******************************************************************************
 * posix_spawnattr_t_once_init
 * 
 * Called from posix_spawn() only once, this routine will initialize the
 * default_posix_spawnattr_t object to a properly keyed value as this is not
 * possible to do at link time
 * 
 * All functions within this file will use _default_posix_spawnattr_t to return
 * default values. Only when posix_spawn() is first called with either an <attrp>
 * of NULL or an <attrp> which has never had any _setxxx() operations made on it
 * will the 'default_posix_spawnattr_t' object be used and hence initialized
*/
void posix_spawnattr_t_once_init(void)
{
	SET_ATTRP(&default_posix_spawnattr_t, (void *)&_default_posix_spawnattr_t);
}

/*******************************************************************************
 * get_attrp
 * set_attrp
 * 
 * These 2 routine translate between a real allocated '_posix_spawnattr_t *' and
 * the token stored in the 'posix_spawnattr_t *'.
 * 
 * A valid, initialized 'posix_spawnattr_t' object has 2 possible values.
 * 		- NULL
 * 		- a valid, allocated address
 * All other values are considered invalid. These 2 routines attempt to catch
 * uninitialized 'posix_spawnattr_t' objects from being used. The example is
 * an automatic 'posix_spawnattr_t' which contains stack garbage. It is non NULL
 * but does not represent valid, allocated memory.
 * 
 * Because we know that allocated memory is always aligned on given boundary
 * we use a key that will always turn a properly allocated memory address into
 * a token which always has the lower 3 bits set (8 byte alignment). This works
 * for all bogus values that do not have 7 as the LSB 
*/

_posix_spawnattr_t *get_attrp(unsigned int x, unsigned key_mask)
{
	if (x != 0) x ^= AKEY(key_mask);
	return (x & key_mask) ? NULL : (_posix_spawnattr_t *)x;
}

/*******************************************************************************
 * valid_attrp
 * 
 * Return true (!0) or FALSE (0) depending on whether <attrp> represents a valid
 * initialized 'posix_spawnattr_t' object.
 * 
 * A valid object is as follows
 * 	- <attrp> is non NULL
 * 	- <*attrp> is NULL or represents allocated memory
 * 
 * In order to differentiate between allocated memory and a non 0 value, any
 * memory actually allocated will be translated. The get_attrp() and set_attrp()
 * routines will be used to translate back and forth 
*/
unsigned valid_attrp(const posix_spawnattr_t *_Restrict attrp)
{
	if (attrp == NULL) return 0;				// not valid
	if (*(posix_spawnattr_t *)((unsigned int)attrp & ~MIN_KEY_MASK) == ~0U) return 0;			// not valid
	if (*(posix_spawnattr_t *)((unsigned int)attrp & ~MIN_KEY_MASK) == 0) return !0;			// valid
	if (GET_ATTRP(attrp) != NULL) return !0;	// valid
	return 0;
}


/*******************************************************************************
 * _posix_spawnattr_t_realloc
 * 
 * If <attrp> == NULL, this function will allocate and initialize to default
 * values, memory for an internal '_posix_spawnattr_t' structure of <size> bytes.
 * If <attrp> != NULL, this function will reallocate the memory pointed to by
 * <attrp> to <size> bytes.
 * If <size> == 0 and <attrp> == NULL, then <size> it will be set to the default
 * (ie. sizeof(*attrp))
 * 
 * Note:
 * The intent of this function is to grow the _posix_spawnattr_t and although it
 * is possible to release the memory for the <attrp> (or resize it to something
 * smaller than sizeof(_posix_spawnattr_t)) by passing in a non-null <attrp> and
 * a <size> < sizeof(_posix_spawnattr_t) (the API for this internal function does
 * not prevent this), it is currently considered an error to do so and the caller
 * is responsible for any side effects.
 * 
 * Returns a pointer to the _posix_spawnattr_t structure or NULL if memory could
 * not be allocated.
*/
_posix_spawnattr_t *_posix_spawnattr_t_realloc(_posix_spawnattr_t *attrp, size_t size)
{
	unsigned first_time = 0;

	if ((attrp == NULL) && (size == 0)) size = sizeof(*attrp);
	if (attrp == NULL) {
		attrp = calloc(1, size);	// ensure zeroed, aligned memory the first time
		first_time = 1;
	} else {
		attrp = realloc(attrp, size);
	}
	if (attrp != NULL) {
		/* if first time, set default values */
		if (first_time) {
			/* first time 'posix_spawnattr_t' allocation */
			memcpy(attrp, &_default_posix_spawnattr_t, sizeof(*attrp));
		}
	}
	return attrp;
}


#endif	/* _POSIX_SPAWN */

