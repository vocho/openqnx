:
# NAME:
#	sys_config.sh - set system specific variables
#
# SYNOPSIS:
#	. /etc/sys_config.sh
#
# DESCRIPTION:
#	Source this script into shell scripts that want to handle
#	various system types.
#	You may well want to edit this on a particular system replacing 
#	`uname -s` etc with the result.  So that the facility will work 
#	even when in single user mode and uname et al are not available.
#
# SEE ALSO:
#	/etc/profile

# RCSid:
#	$Id: sys_config.sh,v 1.5 93/09/29 08:59:36 sjg Exp $
#
#	@(#)Copyright (c) 1991 Simon J. Gerraty
#
#	This file is provided in the hope that it will
#	be of use.  There is absolutely NO WARRANTY.
#	Permission to copy, redistribute or otherwise
#	use this file is hereby granted provided that 
#	the above copyright notice and this notice are
#	left intact. 
#

# determin machine type
if [ -f /386bsd ]; then		# doesn't have uname or arch
	ARCH=i386
	OS=386bsd
	HOSTNAME=`hostname`
elif [ -f /usr/bin/arch ]; then
	ARCH=`arch`
elif [ -f /usr/bin/uname -o -f /bin/uname ]; then
	ARCH=`uname -m`
fi
#
case "$ARCH" in
sun386)	uname=/usr/5bin/uname
	OS=SunOS
	;;
*)	uname=uname;;
esac

# set the operating system type
# you can't use `uname -s` with SCO UNIX
# it returns the same string as `uname -n`
# so set it manually
# OS=SCO-UNIX
# The eval below is a workaround for a bug in the PD ksh.
OS=${OS:-`eval $uname -s`}
HOSTNAME=${HOSTNAME:-`eval $uname -n`}

case `echo -n ""` in
-n*)	_C_=""; _N_="-n";;
*)	_C_="\c"; _N_="";;
esac
N="${_N_}"
C="${_C_}"
export OS ARCH HOSTNAME uname
