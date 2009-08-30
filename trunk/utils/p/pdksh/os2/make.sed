s/@SHELL_PROG@/ksh/
s!/bin/sh!sh!
s/@configure_input@/OS2_Makefile/
s/@srcdir@/./
s/@CC@/gcc/
s/@CPP@/cpp/
s/@CPPFLAGS@//
s/@INSTALL@/\/bin\/install -c/
s/@INSTALL_PROGRAM@/$(INSTALL)/
s/@INSTALL_DATA@/$(INSTALL) -m 644/
s/@DEFS@/-DHAVE_CONFIG_H/
s/@LIBS@/-los2/
s/@LDSTATIC@//
s/@LDFLAGS@/-O -s $(LDSTATIC)/
s/^\(CFLAGS[	 ]*=\).*/\1 -O -DOS2/
s/^\(prefix[	 ]*=\).*/\1 c:\/usr/
s/^\(exec_prefix[	 ]*=\).*/\1 c:\/usr/
s/^\(exe_suffix[	 ]*=\).*/\1.exe/
s/^\(OBJS[	 ]*=\)/\1 os2.o/
s/`echo $(SHELL_PROG)|sed '$(transform)'`/$(SHELL_PROG)/
s#$(srcdir)/tests/th.sh#th.cmd#
/^configure:/,/^$/d
/^config.h.in:/,/^$/d
/^config.h:/,/^$/c\
config.h:\
	cmd /c copy $(srcdir)\\os2\\config.h config.h > nul\
	touch config.h\

/^Makefile:/,/^$/c\
Makefile:\
	cmd /c copy $(srcdir)\\os2\\Makefile Makefile > nul\
	touch Makefile\

/^config.status:/,/^$/d
/^siglist.out:/,/^$/c\
siglist.out:\
	cmd /c copy $(srcdir)\\os2\\os2siglist.out siglist.out > nul\
	touch siglist.out\

/^emacs.out:/,/^$/c\
emacs.out:\
	cmd /c copy $(srcdir)\\os2\\emacs.out emacs.out > nul\
	touch emacs.out\

