/* Print the version number.  */

/* $Id: version.c,v 1.13 2003/05/18 08:25:17 eggert Exp $ */

#define XTERN extern
#include <common.h>
#undef XTERN
#define XTERN
#include <version.h>

static char const copyright_string[] = "\
Copyright (C) 1988 Larry Wall\n\
Copyright (C) 2003 Free Software Foundation, Inc.";

static char const free_software_msgid[] = "\
This program comes with NO WARRANTY, to the extent permitted by law.\n\
You may redistribute copies of this program\n\
under the terms of the GNU General Public License.\n\
For more information about these matters, see the file named COPYING.";

static char const authorship_msgid[] = "\
written by Larry Wall and Paul Eggert";

void
version (void)
{
  printf ("%s %s\n%s\n\n%s\n\n%s\n", PACKAGE_NAME, PACKAGE_VERSION,
	  copyright_string, free_software_msgid, authorship_msgid);
}
