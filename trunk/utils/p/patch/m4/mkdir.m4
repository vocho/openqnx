#serial 1

dnl From Mumit Khan and Paul Eggert
dnl Determine whether mkdir accepts only one argument instead of the usual two.

AC_DEFUN([PATCH_FUNC_MKDIR_TAKES_ONE_ARG],
  [AC_CHECK_FUNCS(mkdir)
   AC_CACHE_CHECK([whether mkdir takes only one argument],
     patch_cv_mkdir_takes_one_arg,
     [patch_cv_mkdir_takes_one_arg=no
      if test $ac_cv_func_mkdir = yes; then
        AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/stat.h>
	  ],
	  [mkdir (".", 0);],
	  ,
	  [AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/stat.h>
	     ],
	     [mkdir (".");],
	     patch_cv_mkdir_takes_one_arg=yes
	  )]
	)
      fi
     ]
   )
   if test $patch_cv_mkdir_takes_one_arg = yes; then
     AC_DEFINE([MKDIR_TAKES_ONE_ARG], 1,
       [Define if mkdir takes only one argument.])
   fi
  ]
)
