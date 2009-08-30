#! /bin/sh
# This file is part of GNU tar testsuite.
# Copyright (C) 2004, 2005 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

PWD=`pwd`
P=`expr $0 : '\(.*\)/.*'`
DIR=`cd $P; pwd`/../../src
if [ -d $DIR ]; then
	PATH=`cd $DIR;pwd`:$PATH
fi

# Usage: quicktest FILELIST ARCHIVE-NAME
quicktest() {
	DIR=quicktest.$$
	mkdir $DIR
	cd $DIR

	TAR_OPTIONS=""
	export TAR_OPTIONS

	tar xf $2
	tar -cf ../archive -H ustar -T $1
	cd ..

	${TARTEST:-tartest} -v < $2 > $DIR/old.out
	${TARTEST:-tartest} -v < archive > $DIR/new.out

	if cmp $DIR/old.out $DIR/new.out; then
		echo "PASS"
		rm -r $DIR
		exit 0
	else
		echo "FAIL. Examine $DIR for details"
		exit 1
	fi
}

test_access() {
	if [ -r $1 ]; then
		:
	else
		echo "$1 does not exist or is unreadable"
		echo 77
	fi
}		

check_environ() {
	if [ "$STAR_TESTSCRIPTS" = "" ]; then
		echo "STAR_TESTSCRIPTS not set"
		exit 77
	fi

	if [ -d $STAR_TESTSCRIPTS ]; then
		:
	else
		echo "STAR_TESTSCRIPTS is not a directory"
		exit 77
	fi		

	ARCHIVE=$STAR_TESTSCRIPTS/ustar-all-quicktest.tar
	test_access $ARCHIVE
	FILELIST=$STAR_TESTSCRIPTS/quicktest.filelist
	test_access $FILELIST				

	${TARTEST:-tartest} < /dev/null > /dev/null 2>&1
	if [ $? -eq 127 ]; then
		echo "tartest not in your path"
		exit 77
	fi
	tar --version
}

getargs() {
	for option
	do
		case $option in
		*=*)  eval $option;;
		*)    echo "Unknown option: $option" >&2
		      exit 77;;
		esac
	done	      
}

if [ -w / ]; then
	getargs $*
	check_environ
	quicktest $FILELIST $ARCHIVE
else
	echo "You need to be root to run this test"
	exit 77
fi

# End of quicktest.sh
