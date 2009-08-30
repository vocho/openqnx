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
 *  sh/platform.h
 *

 */

#ifndef _SH_PLATFORM_H_INCLUDED
#define _SH_PLATFORM_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#error sh/platform.h should not be included directly.
#endif

#if defined(__QNXNTO__)

/*
   For gcc-3 and higher, varargs.h is deprecated, and we use the gcc
   builtin var arg support for stdarg.h, set up in sys/compiler_gnu.h.
 */
#if (defined(__GNUC__) && __GNUC__ < 3)
#define	__NTO_va_list	__gnuc_va_list

/* This is just like the default gvarargs.h
   except for differences described below.  */

/* Define __gnuc_va_list.  */

#if defined (__SH3E__) || defined (__SH4_SINGLE__) || defined (__SH4__) || defined (__SH4_SINGLE_ONLY__)

typedef long __NTO_va_greg;
typedef float __NTO_va_freg;

typedef struct {
  __NTO_va_greg * __NTO_va_next_o;		/* next available register */
  __NTO_va_greg * __NTO_va_next_o_limit;	/* past last available register */
  __NTO_va_freg * __NTO_va_next_fp;		/* next available fp register */
  __NTO_va_freg * __NTO_va_next_fp_limit;	/* last available fp register */
  __NTO_va_greg * __NTO_va_next_stack;		/* next extended word on stack */
} __gnuc_va_list;
#else /* ! SH3E */

typedef void *__gnuc_va_list;

#endif /* ! SH3E */

#if defined (__SH3E__) || defined (__SH4_SINGLE__) || defined (__SH4__) || defined (__SH4_SINGLE_ONLY__)

#define __NTO_va_start_stdarg(AP, LASTARG) \
__extension__ \
  ({ \
     (AP).__NTO_va_next_fp = (__NTO_va_freg *) __builtin_saveregs (); \
     (AP).__NTO_va_next_fp_limit = ((AP).__NTO_va_next_fp + \
			      (__builtin_args_info (1) < 8 ? 8 - __builtin_args_info (1) : 0)); \
     (AP).__NTO_va_next_o = (__NTO_va_greg *) (AP).__NTO_va_next_fp_limit; \
     (AP).__NTO_va_next_o_limit = ((AP).__NTO_va_next_o + \
			     (__builtin_args_info (0) < 4 ? 4 - __builtin_args_info (0) : 0)); \
     (AP).__NTO_va_next_stack = (__NTO_va_greg *) __builtin_next_arg (LASTARG); \
  })

#else /* ! SH3E */

#define __NTO_va_start_stdarg(AP, LASTARG) 						\
 ((AP) = ((__gnuc_va_list) __builtin_next_arg (LASTARG)))

#endif /* ! SH3E */

/*
#define __NTO_va_alist  void* __builtin_va_alist,...
#define __NTO_va_dcl    
*/
/* This is a work-around until the compiler is fixed */
#define __NTO_va_alist  __builtin_va_alist
#define __NTO_va_dcl    int __builtin_va_alist;...

#if defined (__SH3E__) || defined (__SH4_SINGLE__) || defined (__SH4__) || defined (__SH4_SINGLE_ONLY__)

#define __NTO_va_start_vararg(AP) \
__extension__ \
  ({ \
     (AP).__NTO_va_next_fp = ((__NTO_va_freg *) __builtin_saveregs ()); \
     (AP).__NTO_va_next_fp_limit = ((AP).__NTO_va_next_fp + \
			      (__builtin_args_info (1) < 8 ? 8 - __builtin_args_info (1) : 0)); \
     (AP).__NTO_va_next_o = (__NTO_va_greg *) (AP).__NTO_va_next_fp_limit; \
     (AP).__NTO_va_next_o_limit = ((AP).__NTO_va_next_o + \
			     (__builtin_args_info (0) < 4 ? 4 - __builtin_args_info (0) : 0)); \
     (AP).__NTO_va_next_stack \
       = ((__NTO_va_greg *) __builtin_next_arg (__builtin_va_alist) \
	  - (__builtin_args_info (0) >= 4 || __builtin_args_info (1) >= 8 \
	     ? 1 : 0)); \
  })

#else /* ! SH3E */

#define __NTO_va_start_vararg(AP)  ((AP) = (char *) &__builtin_va_alist)

#endif /* ! SH3E */

#ifndef va_end
void va_end (__gnuc_va_list);		/* Defined in libgcc.a */

/* Values returned by __builtin_classify_type.  */

enum __NTO_va_type_classes {
  __no_type_class = -1,
  __void_type_class,
  __integer_type_class,
  __char_type_class,
  __enumeral_type_class,
  __boolean_type_class,
  __pointer_type_class,
  __reference_type_class,
  __offset_type_class,
  __real_type_class,
  __complex_type_class,
  __function_type_class,
  __method_type_class,
  __record_type_class,
  __union_type_class,
  __array_type_class,
  __string_type_class,
  __set_type_class,
  __file_type_class,
  __lang_type_class
};

#endif
#define __NTO_va_end(pvar)	((void)0)

#ifdef __LITTLE_ENDIAN__
#define __LITTLE_ENDIAN_P 1
#else
#define __LITTLE_ENDIAN_P 0
#endif

#define __SCALAR_TYPE(TYPE)					\
  ((TYPE) == __integer_type_class				\
   || (TYPE) == __char_type_class				\
   || (TYPE) == __enumeral_type_class)

/* RECORD_TYPE args passed using the C calling convention are
   passed by invisible reference.  ??? RECORD_TYPE args passed
   in the stack are made to be word-aligned; for an aggregate that is
   not word-aligned, we advance the pointer to the first non-reg slot.  */

  /* When this is a smaller-than-int integer, using
     auto-increment in the promoted (SImode) is fastest;
     however, there is no way to express that is C.  Therefore,
     we use an asm.
     We want the MEM_IN_STRUCT_P bit set in the emitted RTL, therefore we
     use unions even when it would otherwise be unnecessary.  */

/* gcc has an extension that allows to use a casted lvalue as an lvalue,
   But it doesn't work in C++ with -pedantic - even in the presence of
   __extension__ .  We work around this problem by using a reference type.  */
#ifdef __cplusplus
#define __VA_REF &
#else
#define __VA_REF
#endif

#define __NTO_va_arg_sh1(AP, TYPE) __extension__ 				\
({(sizeof (TYPE) == 1							\
   ? ({union {TYPE t; char c;} __t;					\
       __asm(""								\
	     : "=r" (__t.c)						\
	     : "0" ((((union { int i, j; } *__VA_REF) (AP))++)->i));	\
       __t.t;})								\
   : sizeof (TYPE) == 2							\
   ? ({union {TYPE t; short s;} __t;					\
       __asm(""								\
	     : "=r" (__t.s)						\
	     : "0" ((((union { int i, j; } *__VA_REF) (AP))++)->i));	\
       __t.t;})								\
   : sizeof (TYPE) >= 4 || __LITTLE_ENDIAN_P				\
   ? (((union { TYPE t; int i;} *__VA_REF) (AP))++)->t			\
   : ((union {TYPE t;TYPE u;}*) ((char *)++(int *__VA_REF)(AP) - sizeof (TYPE)))->t);})

#if defined (__SH3E__) || defined (__SH4_SINGLE__) || defined (__SH4__) || defined (__SH4_SINGLE_ONLY__)

#define __PASS_AS_FLOAT(TYPE_CLASS,SIZE) \
  (TYPE_CLASS == __real_type_class && SIZE == 4)

#define __TARGET_SH4_P 0

#if defined(__SH4__) || defined(__SH4_SINGLE__)
#undef __PASS_AS_FLOAT
#define __PASS_AS_FLOAT(TYPE_CLASS,SIZE) \
  ((TYPE_CLASS == __real_type_class && SIZE <= 8) \
   || (TYPE_CLASS == __complex_type_class && SIZE <= 16))
#undef __TARGET_SH4_P
#define __TARGET_SH4_P 1
#endif

#define __NTO_va_arg(pvar,TYPE)					\
__extension__							\
({int __type = __builtin_classify_type (* (TYPE *) 0);		\
  void * __result_p;						\
  if (__PASS_AS_FLOAT (__type, sizeof(TYPE)))			\
    {								\
      if ((pvar).__NTO_va_next_fp < (pvar).__NTO_va_next_fp_limit)	\
	{							\
	  if (((__type == __real_type_class && sizeof (TYPE) > 4)\
	       || sizeof (TYPE) > 8)				\
	      && (((int) (pvar).__NTO_va_next_fp ^ (int) (pvar).__NTO_va_next_fp_limit)\
		  & 4))						\
	    (pvar).__NTO_va_next_fp++;				\
	  __result_p = &(pvar).__NTO_va_next_fp;			\
	}							\
      else							\
	__result_p = &(pvar).__NTO_va_next_stack;			\
    }								\
  else								\
    {								\
      if ((pvar).__NTO_va_next_o + ((sizeof (TYPE) + 3) / 4)	\
	  <= (pvar).__NTO_va_next_o_limit) 				\
	__result_p = &(pvar).__NTO_va_next_o;			\
      else							\
	{							\
	  if (sizeof (TYPE) > 4)				\
	   if (! __TARGET_SH4_P)				\
	    (pvar).__NTO_va_next_o = (pvar).__NTO_va_next_o_limit;	\
								\
	  __result_p = &(pvar).__NTO_va_next_stack;			\
	}							\
    } 								\
  __NTO_va_arg_sh1(*(void **)__result_p, TYPE);})

#else /* ! SH3E */

#define __NTO_va_arg(AP, TYPE) __NTO_va_arg_sh1((AP), TYPE)

#endif /* SH3E */

/* Copy __gnuc_va_list into another variable of this type.  */
#define __NTO_va_copy(dest, src) ((dest) = (src))

#endif /* __GNUC__ < 3 */

#define __JMPBUFSIZE	16
typedef	double			__jmpbufalign;

#else
#error Not configured for target
#endif


/* The sh4a math optimizations are only in gcc-3.4 or better */

#if defined __GNUC__ && ( __GNUC__ >= 4 || (__GNUC__ >= 3 && __GNUC_MINOR__ >= 4))

#if defined __SH4A__ && defined __FAST_MATH__

/* use gccs built-ins */

#define _FAST_SIN(val) __builtin_sin(val)
#define _FAST_COS(val) __builtin_cos(val)
#define _FAST_SINL(val) __builtin_sinl(val)
#define _FAST_COSL(val) __builtin_cosl(val)
#define _FAST_SINF(val) __builtin_sinf(val)
#define _FAST_COSF(val) __builtin_cosf(val)

#endif
#endif


#endif

/* __SRCVERSION("platform.h $Rev: 164949 $"); */
