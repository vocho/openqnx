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
 *  ppc/platform.h
 *

 */

#ifndef _PPC_PLATFORM_H_INCLUDED
#define _PPC_PLATFORM_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#error ppc/platform.h should not be included directly.
#endif

#if defined(__QNXNTO__)

/* GNU C varargs support for the PowerPC with V.4 calling sequence */

/*
   For gcc-3 and higher, varargs.h is deprecated, and we use the gcc
   builtin var arg support for stdarg.h, set up in sys/compiler_gnu.h.
 */
#if (defined(__GNUC__) && __GNUC__ < 3)
/* Define __gnuc_va_list.  */

#ifndef __PPC_STDARG_H_INCLUDED
#define __PPC_STDARG_H_INCLUDED

/* Note that the names in this structure are in the user's namespace, but
   that the V.4 abi explicitly states that these names should be used.  */
typedef struct __NTO_va_list_tag {
  char gpr;			/* index into the array of 8 GPRs stored in the
				   register save area gpr=0 corresponds to r3,
				   gpr=1 to r4, etc. */
  char fpr;			/* index into the array of 8 FPRs stored in the
				   register save area fpr=0 corresponds to f1,
				   fpr=1 to f2, etc. */
  char *overflow_arg_area;	/* location on stack that holds the next
				   overflow argument */
  char *reg_save_area;		/* where r3:r10 and f1:f8, if saved are stored */
} __NTO_va_list[1], __gnuc_va_list[1];
#endif /* not __GNUC_VA_LIST */

/* Different arg type should be pass differently in function calls */
#define _VALIST_IS_AN_ARRAY

/* Register save area located below the frame pointer */
typedef struct {
  long   __gp_save[8];		/* save area for GP registers */
  double __fp_save[8];		/* save area for FP registers */
} __NTO_va_regsave_t;

/* Macros to access the register save area */
/* We cast to void * and then to TYPE * because this avoids
   a warning about increasing the alignment requirement.  */

#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 9)

#define __VA_FP_REGSAVE(AP,TYPE)					\
  ((TYPE *) (void *) (&(((__NTO_va_regsave_t *)				\
			 (AP)->reg_save_area)->__fp_save[(int)(AP)->fpr])))

#define __VA_GP_REGSAVE(AP,TYPE)					\
  ((TYPE *) (void *) (&(((__NTO_va_regsave_t *)				\
			 (AP)->reg_save_area)->__gp_save[(int)(AP)->gpr])))

/* Common code for va_start for both varargs and stdarg.  This depends
   on the format of rs6000_args in rs6000.h.  The fields used are:

   #0	WORDS			# words used for GP regs/stack values
   #1	FREGNO			next available FP register
   #2	NARGS_PROTOTYPE		# args left in the current prototype
   #3	ORIG_NARGS		original value of NARGS_PROTOTYPE
   #4	VARARGS_OFFSET		offset from frame pointer of varargs area */

#define __NTO_va_words		__builtin_args_info (0)
#define __NTO_va_fregno		__builtin_args_info (1)
#define	__NTO_va_nargs		__builtin_args_info (2)
#define __NTO_va_orig_nargs		__builtin_args_info (3)
#define __NTO_va_varargs_offset	__builtin_args_info (4)

#define __NTO_va_start_common(AP, FAKE)					\
__extension__ ({							\
   register int __words = __NTO_va_words - FAKE;				\
									\
   (AP)->gpr = (__words < 8) ? __words : 8;				\
   (AP)->fpr = __NTO_va_fregno - 33;					\
   (AP)->reg_save_area = (((char *) __builtin_frame_address (0))	\
			  + __NTO_va_varargs_offset);			\
   (AP)->overflow_arg_area = ((char *)__builtin_saveregs ()		\
			      + (((__words >= 8) ? __words - 8 : 0)	\
				 * sizeof (long)));			\
   (void)0;								\
})

#else /* __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 9) */

#define __VA_FP_REGSAVE(AP,OFS,TYPE)					\
  ((TYPE *) (void *) (&(((__NTO_va_regsave_t *)				\
			 (AP)->reg_save_area)->__fp_save[OFS])))

#define __VA_GP_REGSAVE(AP,OFS,TYPE)					\
  ((TYPE *) (void *) (&(((__NTO_va_regsave_t *)				\
			 (AP)->reg_save_area)->__gp_save[OFS])))

#define __NTO_va_start_common(AP, FAKE)					\
  __builtin_memcpy ((AP), __builtin_saveregs(), sizeof(__gnuc_va_list))

#endif /* __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 9) */

/* Calling __builtin_next_arg gives the proper error message if LASTARG is
   not indeed the last argument.  */
#define __NTO_va_start_stdarg(AP,LASTARG) \
  (__builtin_next_arg (LASTARG), __NTO_va_start_common (AP, 0))

#ifdef _SOFT_FLOAT
#define __NTO_va_float_p(TYPE)	0
#else
#define __NTO_va_float_p(TYPE)	(__builtin_classify_type(*(TYPE *)0) == 8)
#endif

#define __NTO_va_aggregate_p(TYPE)	(__builtin_classify_type(*(TYPE *)0) >= 12)
#define __NTO_va_size(TYPE)		((sizeof(TYPE) + sizeof (long) - 1) / sizeof (long))

#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 9)

#define __NTO_va_arg(AP,TYPE)							\
__extension__ (*({							\
  register TYPE *__ptr;							\
									\
  if (__NTO_va_float_p (TYPE) && (AP)->fpr < 8)				\
    {									\
      __ptr = __VA_FP_REGSAVE (AP, TYPE);				\
      (AP)->fpr++;							\
    }									\
									\
  else if (__NTO_va_aggregate_p (TYPE) && (AP)->gpr < 8)			\
    {									\
      __ptr = * __VA_GP_REGSAVE (AP, TYPE *);				\
      (AP)->gpr++;							\
    }									\
									\
  else if (__builtin_classify_type( *(TYPE *)0) == 1		\
  	   && sizeof(TYPE) == 8				\
	   && (AP)->gpr <= (9-3))				\
    {									\
	  (AP)->gpr = ((AP)->gpr + 1) & ~1;					\
      __ptr = __VA_GP_REGSAVE (AP, TYPE);				\
      (AP)->gpr += __NTO_va_size (TYPE);					\
    }									\
									\
  else if (!__NTO_va_float_p (TYPE) && !__NTO_va_aggregate_p (TYPE)		\
	   && (AP)->gpr + __NTO_va_size(TYPE) <= 8)				\
    {									\
      __ptr = __VA_GP_REGSAVE (AP, TYPE);				\
      (AP)->gpr += __NTO_va_size (TYPE);					\
    }									\
									\
  else if (!__NTO_va_float_p (TYPE) && !__NTO_va_aggregate_p (TYPE)		\
	   && (AP)->gpr < 8)						\
    {									\
      (AP)->gpr = 8;							\
      __ptr = (TYPE *) (void *) ((AP)->overflow_arg_area);		\
      (AP)->overflow_arg_area += __NTO_va_size (TYPE) * sizeof (long);	\
    }									\
									\
  else if (__NTO_va_aggregate_p (TYPE))					\
    {									\
      __ptr = * (TYPE **) (void *) ((AP)->overflow_arg_area);		\
      (AP)->overflow_arg_area += sizeof (TYPE *);			\
    }									\
														\
  else if (__builtin_classify_type( *(TYPE *)0) == 1		\
  	   && sizeof(TYPE) == 8)			\
    {									\
      __ptr = (TYPE *) (void *) (((unsigned int)(AP)->overflow_arg_area + 7) & ~7 );		\
      (AP)->overflow_arg_area = (void *)__ptr + __NTO_va_size (TYPE) * sizeof (long);	\
    }									\
  else									\
    {									\
      __ptr = (TYPE *) (void *) ((AP)->overflow_arg_area);		\
      (AP)->overflow_arg_area += __NTO_va_size (TYPE) * sizeof (long);	\
    }									\
									\
  __ptr;								\
}))

#define __NTO_va_alist        void *__alist, ...
#define __NTO_va_dcl
#define __NTO_va_start_vararg(AP) \
  (__builtin_next_arg (__alist), __NTO_va_start_common (AP, 1))


#else /* __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 9) */

#define __NTO_va_overflow(AP) (AP)->overflow_arg_area
#define __NTO_va_vector_p(TYPE)	(__builtin_classify_type(*(TYPE *)0) == -2)

extern 
#if defined (__cplusplus) || defined(__CPLUSPLUS__)
    "C"
#endif
void __NTO_va_arg_type_violation(void) __attribute__((__noreturn__));

#define __NTO_va_arg(AP,TYPE)							   \
__extension__ (*({							   \
  register TYPE *__ptr;							   \
									   \
  if (__NTO_va_vector_p (TYPE))						   \
    {									   \
      __NTO_va_overflow(AP) = (char *)(((long)(__NTO_va_overflow(AP)) + 15) & ~15); \
      __ptr = (TYPE *) (void *) (__NTO_va_overflow(AP));			   \
      __NTO_va_overflow(AP) += __NTO_va_size (TYPE) * sizeof (long);		   \
    }									   \
									   \
  else if (__NTO_va_float_p (TYPE) && sizeof (TYPE) < 16)			   \
    {									   \
      unsigned char __fpr = (AP)->fpr;					   \
      if (__fpr < 8)							   \
	{								   \
	  __ptr = __VA_FP_REGSAVE (AP, __fpr, TYPE);			   \
	  (AP)->fpr = __fpr + 1;					   \
	}								   \
      else if (sizeof (TYPE) == 8)					   \
	{								   \
	  unsigned long __addr = (unsigned long) (__NTO_va_overflow (AP));	   \
	  __ptr = (TYPE *)((__addr + 7) & -8);				   \
	  __NTO_va_overflow (AP) = (char *)(__ptr + 1);			   \
	}								   \
      else								   \
	{								   \
	  /* float is promoted to double.  */				   \
	  __NTO_va_arg_type_violation ();					   \
	}								   \
    }									   \
									   \
  /* Aggregates and long doubles are passed by reference.  */		   \
  else if (__NTO_va_aggregate_p (TYPE) || __NTO_va_float_p (TYPE))		   \
    {									   \
      unsigned char __gpr = (AP)->gpr;					   \
      if (__gpr < 8)							   \
	{								   \
	  __ptr = * __VA_GP_REGSAVE (AP, __gpr, TYPE *);		   \
	  (AP)->gpr = __gpr + 1;					   \
	}								   \
      else								   \
	{								   \
	  TYPE **__pptr = (TYPE **) (__NTO_va_overflow (AP));		   \
	  __ptr = * __pptr;						   \
	  __NTO_va_overflow (AP) = (char *) (__pptr + 1);			   \
	}								   \
    }									   \
									   \
  /* Only integrals remaining.  */					   \
  else									   \
    {									   \
      /* longlong is aligned.  */					   \
      if (sizeof (TYPE) == 8)						   \
	{								   \
	  unsigned char __gpr = (AP)->gpr;				   \
	  if (__gpr < 7)						   \
	    {								   \
	      __gpr += __gpr & 1;					   \
	      __ptr = __VA_GP_REGSAVE (AP, __gpr, TYPE);		   \
	      (AP)->gpr = __gpr + 2;					   \
	    }								   \
	  else								   \
	    {								   \
	      unsigned long __addr = (unsigned long) (__NTO_va_overflow (AP)); \
	      __ptr = (TYPE *)((__addr + 7) & -8);			   \
	      (AP)->gpr = 8;						   \
	      __NTO_va_overflow (AP) = (char *)(__ptr + 1);			   \
	    }								   \
	}								   \
      else if (sizeof (TYPE) == 4)					   \
	{								   \
	  unsigned char __gpr = (AP)->gpr;				   \
	  if (__gpr < 8)						   \
	    {								   \
	      __ptr = __VA_GP_REGSAVE (AP, __gpr, TYPE);		   \
	      (AP)->gpr = __gpr + 1;					   \
	    }								   \
	  else								   \
	    {								   \
	      __ptr = (TYPE *) __NTO_va_overflow (AP);			   \
	      __NTO_va_overflow (AP) = (char *)(__ptr + 1);			   \
	    }								   \
	}								   \
      else								   \
	{								   \
	  /* Everything else was promoted to int.  */			   \
	  __NTO_va_arg_type_violation ();					   \
	}								   \
    }									   \
  __ptr;								   \
}))

#define __NTO_va_alist        __NTO_va_1st_arg
#define __NTO_va_dcl	  void *__NTO_va_1st_arg; ...
#define __NTO_va_start_vararg(AP) __NTO_va_start_common (AP, 1)

#endif

/* Copy __gnuc_va_list into another variable of this type.  */
#define __NTO_va_copy(dest, src) *(dest) = *(src)
#define __NTO_va_end(AP)	((void)0)

#endif /* __GNUC__ < 3 */

#define __JMPBUFSIZE	64
typedef	double			__jmpbufalign;

#else
#error Not configured for target
#endif

#endif

/* __SRCVERSION("platform.h $Rev: 164949 $"); */
