*****************************************
************ vms/00readme.txt ***********
*****************************************

Additional information for compiling Zip for VMS:

A) Support for storing VMS specific file attributes
   ================================================

The current version of Zip comes with two different types of support
to store VMS file attributes in extra blocks:

  -- the traditional Info-ZIP format in vms/vms_im.c
and

  -- a new PKware (ASI) type extra field structure in vms/vms_pk.c

Both versions should supply exactly the same functionality.
Up to Zip 2.1, the default configuration was to use the traditional IM style,
since it was well tested and known to be stable.
==> NEW <==
As of Zip 2.2, this default has been changed to use PK style extra field
format. This change is needed to support indexed VMS files. The
IM style code in UnZip (!!) has a known problem that prevents the correct
restoring operation for some (but not all) indexed VMS files.

IMPORTANT: To extract new PK style extra fields, Info-ZIP's
           UnZip version 5.2 or newer is required, previous
           versions will crash with an access violation !!!!

If you want to use the old IM style support (to achieve compatibility
with older versions of UnZip), the preprocessor symbol VMS_IM_EXTRA
has to be defined at compile time.

MMS (MMK) users have to edit vms/descrip.mms and add this symbol to
the definition of the COMMON_DEFS macro; for example:

COMMON_DEFS = VMS_IM_EXTRA,

if VMS_IM_EXTRA is the only option. (NOTE the trailing comma!)

Users of the DCL make procedure can select the PK style support by defining
the DCL symbol LOCAL_ZIP as a list of user specific compilation options
(do not forget the trailing comma!!). Example:
$ LOCAL_ZIP == "VMS_IM_EXTRA,"


B) Notes on the compiler switches used on VMS:
   ===========================================

The source has been successfully compiled on VMS 6.1 (VMS 6.2 for AXP), using
 - DEC C 5.2 and 5.6 for Alpha AXP
 - DEC C 4.0 for VMS VAX
 - VAX C 3.2

1. Discussion of the /STANDARD switch:

With the exception of some few rough spots in the VMS specific sources,
the code is fully compatible with the "RELAXED_ANSI" mode of the DEC C
compilers. The problems found in vmsmunch.c and vms_pk.c are caused
by incompatibles between the system include headers supplied for DEC C
(AXP) and DEC C (VAX) which cannot get worked around. (Some system
service structure members have type "unsigned int"  in the VAX version,
but "pointer to [miscellanous]" in the AXP headers.)
I consider the AXP headers to show the direction of `future developement'
and have adapted the sources to match the AXP's header files.
This means:
On Alpha AXP, we can equally well use "/STANDARD=RELAXED" instead of
"/STANDARD=VAXC" without getting any warnings.
With the current release of DEC C on VAX, the /STANDARD=VAXC switch is
required to suppress the "assignment to incompatible type" warnings.
Beginning with the Zip 2.1 release, the compiler mode for Alpha AXP has
been changed to "/STANDARD=RELAX", since the "ANSI mode" executables are
slightly smaller.

2. The /PREFIX_LIBRARY_ENTRIES switch:

In (strict and relaxed) ANSI mode on Alpha AXP, only the standard ANSI
RTL function names get prefixed with "DECC$" by the compiler per default.
This results in unresolved references to such functions as "read()", "open()"
"lseek()" at link step. (The same might be true for earlier releases of DEC C
on VAX.) To resolve this problem, one has to explicitely request prefixing
of all DEC C RTL function by applying the "/PREFIX=ALL" switch.
Although this switch is not needed in "VAXC" mode, it does not hurt either.
Therefore, "/PREFIX=ALL" is applied regardless of the compilation mode,
to avoid any problems when switching over to ANSI standard mode in the future.

C) Support for UT extra field UTC time stamps
   ==========================================
Beginning with Zip 2.1 and UnZip 5.2, the Info-ZIP compression utilities
do principally support saving and restoring the modification time of
Zipfile entries as UTC (GMT) universal time. This new information is
stored in an "extra field" labeled "UT" (Unix style GMT modification/access
times, ...).
Previous version of Zip and UnZip used local time, stored in MSDOS compatible
format (as specified by PKware for the Zip file format). This practice caused
a lot of "time synchronization" trouble when transporting Zip archives world
wide between largely different time zones.

Unfortunately, VMS (and the VMS C runtime environment) up to VMS 6.x does not
contain support for timezone handling and assumes "local time == UTC time".
This has changed with the release of VMS 7.0, which does (finally) support
the concept of "universal world time" that is required for time synchronization
in intercontinental networks...

For this reason, the UTC time stamp support is disabled in VMS Zip by default,
otherwise users would experience annoying time stamp deviations when
locally transfering Zip archives between VMS nodes and other (UNIX, OS/2,
WinNT/Win95, MSDOS) systems.
But when compiled on a VMS 7.x system, the UTC "UT extra field" support is
automatically enabled.

For users located in the GMT time zone (or a nearby timezone, like CET),
it might be worthwhile to enable UTC support by hand.

The default configuration can be overridden by defining one of the
following preprocessor macro:

  USE_EF_UT_TIME        includes "UT" time stamp support
  NO_EF_UT_TIME         disables "UT" time stamp support

When using MMS/MMK, you should add the appropiate symbol to the "COMMON_DEFS"
list in vms/descrip.mms; if the command procedure is used for compiling,
you can add the macro to the "LOCAL_ZIP" DCL symbol.

14-Oct-1997 Christian Spieler
