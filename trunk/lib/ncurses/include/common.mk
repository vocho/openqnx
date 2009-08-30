#
# Copyright 2004, QNX Software Systems Ltd. All Rights Reserved.
#
# This source code may contain confidential information of QNX Software
# Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
# modification, disclosure, distribution or transfer of this software,
# or any software which includes or is based upon any of this code, is
# prohibited unless expressly authorized by QSSL by written agreement. For
# more information (including whether this source code file has been
# published) please email licensing@qnx.com.
#
# $Id: common.mk 153052 2008-08-13 01:17:50Z coreos $

CHDRS= curses.h form.h menu.h panel.h eti.h unctrl.h term.h
CXXHDRS= cursesapp.h cursesf.h cursesm.h cursesp.h cursesw.h cursslk.h etip.h

ifndef HDRS
HDRS= $(CHDRS)
endif

include common/mkfiles/install_in.mk

all: $(HDRS)

install: $(HDRS)
	for i in $(HDRS); do \
		cp $(CPOPT) -f $$i $(INSTALL_IN); \
	done
	ln -sf curses.h $(INSTALL_IN)/ncurses.h
