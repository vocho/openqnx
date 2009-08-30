Copyright (c) 1990-2006 Info-ZIP.  All rights reserved.

See the accompanying file LICENSE, version 2005-Feb-10 or later
(the contents of which are also included in zip.h) for terms of use.
If, for some reason, all of these are missing, the Info-ZIP license
also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html

This is Zip 2.32, a maintenance update for Zip 2.3.  Changes include
fixes and speedups to pattern matching, other bug fixes, addition
of a C-Sharp example, and a new -MM option where input patterns must
match at least one file and input files must be readable.

Previous updates in Zip 2.31 included some bug fixes for Debian and
SourceForge bugs, inclusion of the standard zip encryption code in the
main code, some VMS updates, some changes to the Win32 projects, and
some other bug fixes.

We are also working on the new Zip 3.0 and companion UnZip 6.0 that
finally support files and archives larger than 4 GB on systems that support
large files and include other new features.  See the latest betas for
details and the new releases when available.


Zip 2.32 is a compression and file packaging utility.  It is compatible with
PKZIP 2.04g (Phil Katz ZIP) for MSDOS systems.  There is a companion to zip
called unzip (of course) which you should be able to find the same place
you got zip. See the file 'WHERE' for details on ftp sites and mail
servers.

This version of zip has been ported to a wide array of Unix and other
mainframes, minis, and micros including VMS, OS/2, Minix, MSDOS, Windows NT,
Atari, Amiga, BeOS and VM/CMS. Although highly compatible with PKware's
PKZIP and PKUNZIP utilities of MSDOS fame, our primary objective has been
one of portability and other-than-MSDOS functionality.  Features not found
in the PKWare version include creation of zip files in a pipe or on a
device, VMS, BeOS and OS/2 extended file attributes, conversion from Unix to
MSDOS text file format; and, of course, the ability to run on most of your
favorite operating systems.  And it's free.

See the file WhatsNew for a summary of new features and the file 'CHANGES' for
a detailed list of all changes.

Notes:

Multi-volume support.  This version does not support multi-volume archives
as in pkzip 2.04g.  This is expected to be in Zip 3.0 in the form of
split archive support but the original PKZip spanning that only works on
floppy disks will not be supported.  Where spanned archives use floppy
volume labels for marking volumes, split archives use the names of the
split files.  Otherwise these forms are identical and there is a
procedure for converting between spanned and split archives.

Installation.  Please read the file INSTALL for information on how to
compile and install zip, zipsplit, zipcloak, and zipnote.  Please read
the file MANUAL for information on how to use them.  The file "contents"
is a complete list of the files you should have in this distribution.  Also,
if you are using MSDOS, you should read the note on file formats at the end
of the contents file.  (The contents file somehow got misplaced apparently
in the Zip 2.3 distribution.  If time permits we will create a new one.)

Encryption.  This version supports standard zip encryption.  The encryption
code was distributed separately before this version because of US export
regulations but recent relaxing of export restrictions now allow the code
to be included.  Note that standard zip encryption is considered weak by
today's standards.  We are looking at adding strong encryption to a later
release.  Decryption can be made with unzip 5.0p1 or later, or with zipcloak.

Bug reports.  All bug reports should go to zip-bugs using the web form at
www.info-zip.org, and suggestions for new features can be sent using the
same form (although we don't promise to use all suggestions).  Contact us to
send patches as we currently don't have a standard way to do that.  Patches
should be sent as context diffs only (diff -c).

Ports.  If you're considering a port, please check in with zip-bugs FIRST,
since the code is constantly being updated behind the scenes.  We'll
arrange to give you access to the latest source.

Discussion group.  If you'd like to keep up to date with our Zip (and
companion UnZip utility) development, join the ranks of BETA testers, add
your own thoughts and contributions, etc., send a two-line mail message
containing the commands HELP and LIST (on separate lines in the body of the
message, not on the subject line) to mxserver@lists.wku.edu.  You'll receive
two messages listing the various Info-ZIP mailing-list formats which are
available (and also various unrelated lists) and instructions on how to
subscribe to one or more of them (courtesy of Hunter Goatley).  (Currently
these are all discontinued but we are considering implementing new versions.
However, a discussion group for Info-ZIP issues is at
http://www.quicktopic.com/27/H/V6ZQZ54uKNL and can be used to discuss issues,
request features, and is one place new betas and releases are announced.)

Frequently asked questions on zip and unzip:

Q. When unzipping I get an error message about "compression method 8".

A. Please get the latest version of unzip. See the file 'WHERE' for details.


Q. I can't extract this zip file that I just downloaded. I get
   "zipfile is part of multi-disk archive" or some other message.

A. Please make sure that you made the transfer in binary mode. Check
   in particular that your copy has exactly the same size as the original.


Q. When running unzip, I get a message about "End-of-central-directory
   signature not found".

A. This usually means that your zip archive is damaged, or that you
   have an uncompressed file with the same name in the same directory.
   In the first case, it makes more sense to contact the person you
   obtained the zip file from rather than the Info-ZIP software
   developers, and to make sure that your copy is strictly identical to
   the original.  In the second case, use "unzip zipfile.zip" instead
   of "unzip zipfile", to let unzip know which file is the zip archive
   you want to extract.


Q. Why doesn't zip do <something> just like PKZIP does?

A. Zip is not a PKZIP clone and is not intended to be one.  In some
   cases we feel PKZIP does not do the right thing (e.g., not
   including pathnames by default); in some cases the operating system
   itself is responsible (e.g., under Unix it is the shell which
   expands wildcards, not zip).  Info-ZIP's and PKWARE's zipfiles
   are interchangeable, not the programs.

   For example, if you are used to the following PKZIP command:
               pkzip -rP foo *.c
   you must use instead on Unix:
               zip -R foo '*.c'
   (the singled quotes are needed to let the shell know that it should
    not expand the *.c argument but instead pass it on to the program)


Q. Can I distribute zip and unzip sources and/or executables?

A. You may redistribute the latest official distributions without any
   modification, without even asking us for permission. You can charge
   for the cost of the media (CDROM, diskettes, etc...) and a small copying
   fee.  If you want to distribute modified versions please contact us at
   zip-bugs at www.info-zip.org first. You must not distribute beta versions.
   The latest official distributions are on ftp.info-zip.org in directory
   /pub/archiving/zip and subdirectories as well as on mirror sites.


Q. Can I use the executables of zip and unzip to distribute my software?

A. Yes, so long as it is made clear in the product documentation that
   zip or unzip are not being sold, that the source code is freely
   available, and that there are no extra or hidden charges resulting
   from its use by or inclusion with the commercial product. Here is
   an example of a suitable notice:

     NOTE:  <Product> is packaged on this CD using Info-ZIP's compression
     utility.  The installation program uses UnZip to read zip files from
     the CD.  Info-ZIP's software (Zip, UnZip and related utilities) is
     free and can be obtained as source code or executables from various
     anonymous-ftp sites, including ftp.info-zip.org:/pub/archiving/zip/*.


Q. Can I use the source code of zip and unzip in my commercial application?

A. Yes, so long as you include in your product an acknowledgment and an
   offer of the original compression sources for free or for a small
   copying fee, and make clear that there are no extra or hidden charges
   resulting from the use of the compression code by your product. In other
   words, you are allowed to sell only your own work, not ours. If you have
   special requirements contact us at zip-bugs at www.info-zip.org.
