#! /bin/sh

# edit this to taste; note that you can also override via the environment:
case "$CC" in
  "") CC=cc
esac

test -f config.h || echo "Creating basic config.h..." && \
	cat >config.h <<'END_OF_CONFIG_H'
/* A bootstrap version of config.h, for systems which can't
   auto-configure due to a lack of a working sed.  If you are on
   a sufficiently odd machine you may need to hand-tweak this file.

   Regardless, once you get a working version of sed you really should
   re-build starting with a run of "configure", as the bootstrap
   version is almost certainly more crippled than it needs to be on
   your machine.
*/

#define PACKAGE "sed"
#define VERSION "``bootstrap''"
#define BOOTSTRAP 1

/* Undefine if your compiler/headers have a conflicting definition. */
#define const

/* Undefine if <stdio.h> or <sys/types.h> has conflicting definition.  */
#define size_t unsigned

/* If your antique compiler doesn't grok ``void *'', then #define VOID char */
#undef VOID

/* All other config.h.in options intentionally omitted.  Report as a
   bug if you need extra "#define"s in here. */
END_OF_CONFIG_H

# tell the user what we're doing from here on...
set -x

# the ``|| exit 1''s are for fail-stop; set -e doesn't work on some systems

rm -f lib/*.o sed/*.o sed/sed
cd lib || exit 1
${CC} -DHAVE_CONFIG_H -I.. -I. -c \
  alloca.c getopt.c getopt1.c memcmp.c memmove.c regex.c strerror.c || exit 1

cd ../sed || exit 1
${CC} -DHAVE_CONFIG_H -I.. -I. -I../lib -c \
  sed.c compile.c execute.c utils.c || exit 1

${CC} -o sed sed.o compile.o execute.o utils.o ../lib/*.o || exit 1
