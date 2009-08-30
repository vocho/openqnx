HORIZONTAL SCROLLING
====================

I have migrated my 3.2 ksh edit.c mods into the 3.3 ksh
This file describes the mods in general and really only applies
(at this stage) to emacs-mode.  I have not touched vi-mode apart
from making it use pprompt() so that '!' is correctly expanded
as in emacs-mode.

I would prefer to see both vi.c and emacs.c use a common set of
primatives in edit.c - but that looks like a lot of work.

Basically my mods affect the functions that move the cursor
about and redraw the edit line.  

The input buffer "buf" is pointed to by "xbuf" and its end is
pointed to by "xend".  The current position in "xbuf" and end of
the edit line are pointed to by "xcp" and "xep" respectively.
I have added "xbp" which points to the start of a display window
within "xbuf".

[A] starting position

buf
|<-------- $COLUMNS --------->|
|    |<---- x_displen ------->|
| PS1|
     +==========+==============--------+.......+
     |\          \                      \       \
  xbuf xbp        xcp                    xep     xend

[B] scrolled

buf
|           |<----- COLUMNS -------->|
|           |<----- x_displen ------>|
|      
+-----------+==========+==============--------+.......+
 \           \          \                      \       \
  xbuf        xbp        xcp                    xep     xend

In the above -------- represents the current edit line while
===== represents that portion which is visible on the screen.
Note that initially xbp == xbuf and PS1 is displayed.
PS1 uses some of the screen width and thus "x_displen" is less
than $COLUMNS.

Any time that "xcp" moves outside the region bounded by "xbp"
and "xbp" + "x_displen", the function x_adjust() is called to
relocate "xbp" appropriately and redraw the line.

Excessive I/O is avoided where possible.  x_goto() for instance
calculates whether the destination is outside the visible
region, and if so simply adjusts "xcp" and calls x_adjust()
directly.  Normally though x_adjust() is called from x_putc().

The above mechanism has be updated to use a function x_lastcp()
that returns a pointer that accurately  reflects the last
visible char in the edit buffer.  That is a more accurate
version of xbp + x_displen which does not account for TABS.

Other changes
=============

I have also added some emacs mode functions.
M-[0-9]
	Set a numeric arg (positive only).
	Used by all word related commands.
M-_
M-.
	Retrieve word from previous command (default is last
	word).  Use M-[1-9] to select word.
M-u
M-l
M-c
	UPPER,lower,Capitalize word.


Commands like delete-word now push the deleted text so that it
may be yanked back into place.

BUGS?
=====

There are bound to be some.  Please report same to me:

Simon J. Gerraty	<sjg@zen.void.oz.au>

