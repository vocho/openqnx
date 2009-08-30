:
# NAME:
#	profile - global initialization for sh,ksh
#
# DESCRIPTION:
#	This file is processed during login by /bin/sh
#	and /bin/ksh.  It is used to setup the default user
#	environment.
#
# SEE ALSO:
#	$HOME/.profile
#	/etc/ksh.kshrc

# RCSid:
#	$Id: profile,v 1.4 1992/08/10 12:00:11 sjg Exp $
#
#	@(#)Copyright (c) 1991 Simon J. Gerraty
#
#	This file is provided in the hope that it will
#	be of use.  There is absolutely NO WARRANTY.
#	Permission to copy, redistribute or otherwise
#	use this file is hereby granted provided that 
#	the above copyright notice and this notice are
#	left intact. 

sigs="2 3"
trap "" $sigs	# don't interrupt us

# simple versions. See ksh.kshrc for the clever ones
add_path () { [ -d $1 ] && eval ${2:-PATH}="\$${2:-PATH}:$1"; }
pre_path () { [ -d $1 ] && eval ${2:-PATH}="$1:\$${2:-PATH}"; }
del_path () { eval ${2:-PATH}=`eval echo :'$'${2:-PATH}: | 
	sed -e "s;:$1:;:;g" -e "s;^:;;" -e "s;:\$;;"`; }

case "$_INIT_" in
*env*) ;;
*)	# do these once
	_INIT_="$_INIT_"env
	export _INIT_
	case `echo -n ""` in
	-n*)
	  N=""; C="\c";;
	*)
	  N="-n"; C="";;
	esac

	if [ -f /unix ]; then
          # System V
	  [ -z "$TZ" -a -f /etc/TIMEZONE ] && . /etc/TIMEZONE

  	  set -- `who -r`
    	  case "$3" in
	      S|5|0)	SINGLE=y;;
	      *)	SINGLE=n;;
	  esac
        else
          SINGLE=n		# doesn't matter so much
	fi

	OS=${OS:-`uname -s`}
	ARCH=${ARCH:-`uname -m`}
	HOSTNAME=`hostname 2>/dev/null`
	HOSTNAME=${HOSTNAME:-`uname -n`}
	export OS ARCH HOSTNAME

	# pick one of the following for the default umask
	umask 002	# relaxed	-rwxrwxr-x
	# umask 022	# cautious	-rwxr-xr-x
	# umask 027	# uptight	-rwxr-x---
	# umask 077	# paranoid	-rwx------
	# you can override the default umask
	# for specific groups later...

	if [ -d /local ]; then
		LOCAL=/local
	else
		LOCAL=/usr/local
	fi

	# defaults (might be reset below)
	PATH=/bin:/usr/bin
	MANPATH=/usr/man
	SPOOL=/usr/spool
	defterm=vt220

	# set system specific things,
	# eg. set PATH,MANPATH 
	# override default ulimit if desired.
	# defult ulmit is unlimited on SunOS
	# and 4Mb for most System V
	case $OS in
	SunOS)
		# On sun's /bin -> /usr/bin so leave it out!
		PATH=/usr/bin:/usr/ucb:/usr/5bin:/usr/etc
		SPOOL=/var/spool
		LD_LIBRARY_PATH=/usr/lib
		add_path /usr/snm/lib LD_LIBRARY_PATH
		add_path /usr/X11R5/lib LD_LIBRARY_PATH
		add_path /usr/openwin/lib LD_LIBRARY_PATH
		export LD_LIBRARY_PATH
		;;
	SCO-UNIX)
		defterm=ansi
		;;
	B.O.S.)
		MANPATH=/usr/catman
		SRC_COMPAT=_SYSV
		export SRC_COMPAT
		;;
	NetBSD|386bsd)
		MACHINE_ARCH=`uname -m`
		MANPATH=/usr/share/man
		add_path /usr/X386/man MANPATH
		MAILDIR=/var/mail
		SPOOL=/var/spool
		export MACHINE_ARCH
		;;
	esac
	# add_path only adds them if they exist
	add_path /sbin
	add_path /usr/sbin
	add_path /usr/distbin
	add_path /usr/ucb
	add_path /usr/lbin
	add_path /usr/dbin
	add_path /usr/ldbin
	add_path ${LOCAL}/bin
	add_path /usr/bin/X11
	add_path /usr/X11R5/bin
	add_path /usr/openwin/bin
	# ensure . is at end
	PATH=$PATH:.

	case "$HOME" in
	/)	;;
	""|/tmp)
		echo "Using /tmp for HOME"
		HOME=/tmp; export HOME
		;;
	*)
		pre_path $HOME/bin
		;;
	esac
	add_path /usr/X11R5/man MANPATH
	add_path ${LOCAL}/man MANPATH

	# make sure these are set at least once
	LOGNAME=${LOGNAME:-`logname`}
	USER=${USER:-$LOGNAME}

	# NOTE: set up $GROUPDIR such that users cannot modify/install
	# their own $GROUPDIR/profile
        GROUPDIR=`dirname $HOME`
        [ "$GROUPDIR" != /etc -a -f $GROUPDIR/profile ] && . $GROUPDIR/profile

	export LOCAL TTY PATH LOGNAME USER

	if [ -t 1 ]; then
		# we are interactive
		TTY=`tty`
		TTY=`basename $TTY`
		if [ -f /etc/organization ]; then
			ORGANIZATION="`cat /etc/organization`"
			COPYRIGHT="Copyright (c) `date +19%y` $ORGANIZATION"
			export ORGANIZATION COPYRIGHT
		fi
		# set up some env variables
		MAIL=${MAILDIR:-$SPOOL/mail}/$USER
		MAILPATH=$MAIL:/etc/motd
		EMACSDIR=${LOCAL}/lib/emacs
		PAGER=${PAGER:-more}
		export MAIL EMACSDIR MANPATH MAILPATH PAGER

		CVSROOT=${LOCAL}/src/master
		EDITOR=vi
		VISUAL=vi
		FCEDIT=$EDITOR
		export CVSROOT FCEDIT EDITOR VISUAL
		case $UID in
		0) PS1S='# ';;
		esac
		PS1S=${PS1S:-'$ '}
		PROMPT="<$LOGNAME@$HOSTNAME>$PS1S"
		[ -f /etc/profile.TeX ] && . /etc/profile.TeX
	else
		TTY=none
	fi

	# test (and setup if we are Korn shell)
	if [ "$RANDOM" != "$RANDOM" ]; then
		# we are Korn shell
		SHELL=/bin/ksh
		ENV=${HOME%/}/.kshrc
		if [ ! -f $ENV ]; then
			ENV=/etc/ksh.kshrc
		fi
		HISTFILE=${HOME%/}/.ksh_hist
		PROMPT="<$LOGNAME@$HOSTNAME:!>$PS1S"
		export HISTSIZE HISTFILE ENV
		CDPATH=.:$HOME
		if [ "$TMOUT" ]; then
			typeset -r TMOUT
		fi
		set -o emacs	# biased :-)
	else
		SHELL=/bin/sh
	fi
	PS1=$PROMPT
	export SHELL PS1 EDITOR PATH PROMPT HOSTNAME CDPATH
;;
esac

# login time initialization
case "$_INIT_" in
*log*) ;;
*)	_INIT_="$_INIT_"log
	case "$SINGLE" in
	y)	;;
	*)
	if [ TTY != none -a "$0" != "-su" -a "$LOGNAME" = "`logname`" ]
	then
		case $TTY in
		tty0*)
			echo "`date '+%b %d %H:%M:%S'` $LOGNAME logged in on $TTY" > /dev/console;;
		esac
		stty sane		# usually a good idea :-)
	  if [ ! -f ~/.hushlogin ]; then
		# ensure known state
		case $OS in
		SunOS|*BSD)	;;
		*)
			stty isig icanon intr '^c' erase '^h' kill '^u' eof '^d' 
			mesg y
			;;
		esac

		case $TERM in
		network|unknown|dialup|"") 
		  echo ${N} "Enter terminal type [$defterm]: ${C}" 1>&2
		  read tmpterm
		  TERM=${tmpterm:-$defterm}
		  ;;
		esac
		case "$TERM" in
		pc3|xterm)
			stty erase ^?
			;;
		esac
		# not all of the following are appropriate at all sites
		# Sun's don't need to cat /etc/motd for instance
		case "$OS" in
		SunOS)	;;
		SCO-UNIX)	
			[ -s /etc/motd ] && cat /etc/motd
			[ -x /usr/bin/mail -a -s "$MAIL" ] && 
				echo "You have mail."
			[ -x /usr/bin/news ] && /usr/bin/news -n
			;;
		NetBSD|386bsd)
			# hardware flow control works so use it
			case $TTY in
			tty0*)	# dialups
				stty  -ixon -ixany
				stty crtscts
				;;
			esac
			;;
		*)
			[ -s /etc/motd ] && cat /etc/motd
			if [ -x /usr/bin/mailx ]; then
		 	  if mailx -e; then
			    echo "You have mail."
			    # show the the headers, this might
			    # be better done in .profile so they
			    # can override it.
#			    mailx -H
			  fi
			fi
			[ -x /usr/bin/news ] && /usr/bin/news -n
			;;
		esac
		if [ -f $LOCAL/etc/1stlogin.ann ]; then
			[ -f $HOME/... ] || sh $LOCAL/etc/1stlogin.ann
		fi
#		[ -x /usr/games/fortune ] && /usr/games/fortune -a
		# remind folk who turned on reply.pl to turn it off.
		if [ -f $HOME/.forward ]; then
			echo "Your mail is being forwarded to:"
			cat $HOME/.forward
			if [ -f $HOME/.recording ]; then
				echo "Perhaps you should run \"reply.pl off\""
			fi
		fi
		[ -x /usr/ucb/msgs ] && /usr/ucb/msgs -fq
	  fi
	fi
	unset tmpterm defterm C N
	esac
	case "$TERM" in
	network|unknown|"")	TERM=$defterm;;
	esac
	export TERM TTY
;;
esac
# Handle X-terminals if necessary
[ "$SINGLE" = n -a -f /etc/profile.X11 ] && . /etc/profile.X11

# make sure you have this bit last
trap $sigs	# restore signals
unset sigs
