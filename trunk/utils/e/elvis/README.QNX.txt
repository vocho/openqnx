1) On self hosted:
1.1) ./configure --verbose --with-x=no --with-gnome=no --prefix=/usr qnx
1.2) make all ( to know what's needed)
2) mv osunix/* .
3) get rid of  os* directories
4) get rid of original Makefile and Makefile.in and various scripts
5) get rid of alias.c program
6) reorg the files in:
   common/    files common to ctags and elvis
   ctags/     ctags program
   elvis/     elvis program
7) Create nto/*/* directory hierarchy with makefiles and common.mk
8) add module.tmpl


============================================================