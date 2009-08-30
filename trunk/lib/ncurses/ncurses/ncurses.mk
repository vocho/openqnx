EXCLUDE_OBJS=make_keys.o
POST_HINSTALL=$(LN_HOST) curses.h $(INSTALL_ROOT_HDR)/ncurses.h
include ../../../../common.mk
