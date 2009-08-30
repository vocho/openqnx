
AC_DEFUN([PU_RMT],[
  # Set LIB_SETSOCKOPT to -lnsl -lsocket if necessary.
  pu_save_LIBS=$LIBS
  LIB_SETSOCKOPT=
  AC_SEARCH_LIBS(setsockopt, [socket], ,
    [AC_SEARCH_LIBS(setsockopt, [socket], , , [-lnsl])])
  AC_SEARCH_LIBS(setsockopt, [nsl])

  case "$ac_cv_search_setsockopt" in
    -l*) LIB_SETSOCKOPT=$ac_cv_search_setsockopt
  esac
  AC_SUBST(LIB_SETSOCKOPT)
  LIBS=$pu_save_LIBS

  AC_CHECK_FUNCS_ONCE([strerror])
  enable_rmt() {
    if test $ac_cv_header_sys_mtio_h = yes; then
      AC_CACHE_CHECK(for remote tape header files, pu_cv_header_rmt,
        [AC_TRY_CPP([
#if HAVE_SGTTY_H
# include <sgtty.h>
#endif
#include <sys/socket.h>],
      pu_cv_header_rmt=yes,
      pu_cv_header_rmt=no)])
      test $pu_cv_header_rmt = yes && PU_RMT_PROG='rmt$(EXEEXT)'
      AC_SUBST(PU_RMT_PROG)
    fi
  }

  AC_CHECK_HEADERS([sys/mtio.h])
  AC_CACHE_CHECK(which ioctl field to test for reversed bytes,
    pu_cv_header_mtio_check_field,
    [AC_EGREP_HEADER(mt_model, sys/mtio.h,
     pu_cv_header_mtio_check_field=mt_model,
     pu_cv_header_mtio_check_field=mt_type)])
  AC_DEFINE_UNQUOTED(MTIO_CHECK_FIELD,
                     $pu_cv_header_mtio_check_field,
                     [Define to mt_model (v.g., for DG/UX), else to mt_type.])


  AC_ARG_VAR([DEFAULT_RMT_DIR],
             [Define full file name of the directory where to install `rmt'. (default: $(libexecdir))])
  if test "x$DEFAULT_RMT_DIR" != x; then
	DEFAULT_RMT_COMMAND=$DEFAULT_RMT_DIR/rmt
  else
	DEFAULT_RMT_DIR='$(libexecdir)'
  fi

  AC_MSG_CHECKING([whether to build rmt])
  AC_ARG_WITH([rmt],
              AC_HELP_STRING([--with-rmt=FILE],
                             [Use FILE as the default `rmt' program. Do not build included copy of `rmt'.]),
              [case $withval in
	       yes|no) AC_MSG_ERROR([Invalid argument to --with-rmt]);;
	       /*)     DEFAULT_RMT_COMMAND=$withval
	               AC_MSG_RESULT([no, use $withval instead]);;
	       *)      AC_MSG_ERROR([Argument to --with-rmt must be an absolute file name]);;
               esac],
               [AC_MSG_RESULT([yes])
                enable_rmt
                if test "$PU_RMT_PROG" = ""; then
                  AC_MSG_WARN([not building rmt, required header files are missing])
                fi])

  AC_SUBST(DEFAULT_RMT_COMMAND)
  if test "x$DEFAULT_RMT_COMMAND" != x; then
    AC_DEFINE_UNQUOTED(DEFAULT_RMT_COMMAND, "$DEFAULT_RMT_COMMAND",
                       [Define full file name of rmt program.])
  fi
])


