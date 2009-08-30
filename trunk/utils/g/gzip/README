This is the file README for the gzip distribution.

gzip (GNU zip) is a compression utility designed to be a replacement
for 'compress'. Its main advantages over compress are much better
compression and freedom from patented algorithms.  The GNU Project
uses it as the standard compression program for its system.

gzip currently uses by default the LZ77 algorithm used in zip 1.9 (the
portable pkzip compatible archiver). The gzip format was however
designed to accommodate several compression algorithms. See below
for a comparison of zip and gzip.

gunzip can currently decompress files created by gzip, compress or
pack. The detection of the input format is automatic.  For the
gzip format, gunzip checks a 32 bit CRC. For pack, gunzip checks the
uncompressed length.  The 'compress' format was not designed to allow
consistency checks. However gunzip is sometimes able to detect a bad
.Z file because there is some redundancy in the .Z compression format.
If you get an error when uncompressing a .Z file, do not assume that
the .Z file is correct simply because the standard uncompress does not
complain.  This generally means that the standard uncompress does not
check its input, and happily generates garbage output.

gzip produces files with a .gz extension. Previous versions of gzip
used the .z extension, which was already used by the 'pack'
Huffman encoder. gunzip is able to decompress .z files (packed
or gzip'ed).

Several planned features are not yet supported (see the file TODO).
See the file NEWS for a summary of changes since 0.5.  See the file
INSTALL for installation instructions. Some answers to frequently
asked questions are given in the file INSTALL, please read it. (In
particular, please don't ask me once more for an /etc/magic entry.)

WARNING: gzip is sensitive to compiler bugs, particularly when
optimizing.  Use "make check" to check that gzip was compiled
correctly.  Try compiling gzip without any optimization if you have a
problem.

Please send all comments and bug reports by electronic mail to
<bug-gzip@gnu.org>.

Bug reports should ideally include:

    * The complete output of "gzip -V" (or the contents of revision.h
      if you can't get gzip to compile)
    * The hardware and operating system (try "uname -a")
    * The compiler used to compile (if it is gcc, use "gcc -v")
    * A description of the bug behavior
    * The input to gzip, that triggered the bug

If you send me patches for machines I don't have access to, please test them
very carefully. gzip is used for backups, it must be extremely reliable.

GNU tar 1.11.2 has a -z option to invoke directly gzip, so you don't have to
patch it. The package ftp.uu.net:/languages/emacs-lisp/misc/jka-compr19.el.Z
also supports gzip'ed files.

The znew and gzexe shell scripts provided with gzip benefit from
(but do not require) the cpmod utility to transfer file attributes.
It is available in
ftp://gatekeeper.dec.com/pub/usenet/comp.sources.unix/volume11/cpmod.Z.

The sample programs zread.c, sub.c and add.c in subdirectory sample
are provided as examples of useful complements to gzip. Read the
comments inside each source file.  The perl script ztouch is also
provided as example (not installed by default since it relies on perl).


gzip is free software, you can redistribute it and/or modify it under
the terms of the GNU General Public License, a copy of which is
provided under the name COPYING. The latest version of gzip are always
available from ftp://ftp.gnu.org/gnu/gzip or in any of the gnu
mirror sites.

- sources in gzip-*.tar (or .shar or .tar.gz).
- MSDOS lha self-extracting exe in gzip-msdos-*.exe. Once extracted,
  copy gzip.exe to gunzip.exe and zcat.exe, or use "gzip -d" to decompress.
  gzip386.exe runs much faster but only on 386 and above; it was compiled with
  djgpp 1.10 available in directory omnigate.clarkson.edu:/pub/msdos/djgpp.

A VMS executable is in ftp://ftp.spc.edu/[.macro32.savesets]gzip-1-*.zip
(use [.macro32]unzip.exe to extract). A PRIMOS executable is available
in ftp://ftp.lysator.liu.se/pub/primos/run/gzip.run.

Some ftp servers can automatically make a tar.Z from a tar file. If
you are getting gzip for the first time, you can ask for a tar.Z file
instead of the much larger tar file.

Many thanks to those who provided me with bug reports and feedback.
See the files THANKS and ChangeLog for more details.


		Note about zip vs. gzip:

The name 'gzip' was a very unfortunate choice, because zip and gzip
are two really different programs, although the actual compression and
decompression sources were written by the same persons. A different
name should have been used for gzip, but it is too late to change now.

zip is an archiver: it compresses several files into a single archive
file. gzip is a simple compressor: each file is compressed separately.
Both share the same compression and decompression code for the
'deflate' method.  unzip can also decompress old zip archives
(implode, shrink and reduce methods). gunzip can also decompress files
created by compress and pack. zip 1.9 and gzip do not support
compression methods other than deflation. (zip 1.0 supports shrink and
implode). Better compression methods may be added in future versions
of gzip. zip will always stick to absolute compatibility with pkzip,
it is thus constrained by PKWare, which is a commercial company.  The
gzip header format is deliberately different from that of pkzip to
avoid such a constraint.

On Unix, gzip is mostly useful in combination with tar. GNU tar
1.11.2 and later has a -z option to invoke gzip automatically.  "tar -z"
compresses better than zip, since gzip can then take advantage of
redundancy between distinct files. The drawback is that you must
scan the whole tar.gz file in order to extract a single file near
the end; unzip can directly seek to the end of the zip file. There
is no overhead when you extract the whole archive anyway.
If a member of a .zip archive is damaged, other files can still
be recovered. If a .tar.gz file is damaged, files beyond the failure
point cannot be recovered. (Future versions of gzip will have
error recovery features.)

gzip and gunzip are distributed as a single program. zip and unzip
are, for historical reasons, two separate programs, although the
authors of these two programs work closely together in the info-zip
team. zip and unzip are not associated with the GNU project.
See http://www.cdrom.com/pub/infozip/ for more about zip and unzip.


========================================================================

Copyright (C) 1999, 2001, 2002, 2006, 2007 Free Software Foundation, Inc.
Copyright (C) 1992, 1993 Jean-loup Gailly

Permission is granted to copy, distribute and/or modify this document
under the terms of the GNU Free Documentation License, Version 1.2 or
any later version published by the Free Software Foundation; with no
Invariant Sections, with no Front-Cover Texts, and with no Back-Cover
Texts.  A copy of the license is included in the ``GNU Free
Documentation License'' file as part of this distribution.
