#!/bin/sh

#
# Update the date in the version file (version.c).  If the existing
# date is todays date, a .number is apprended (or incremented) to
# make them distinct.
# Also update the version number and date in tests/version.t and ksh.Man.
#

# pattern that matches the date in the version string in version.c
#	\1 is preamble (@(#)PD KSH ),
#	\2 is the version (v1.2.3 ),
#	\3 is the date (99/03/21.3),
#	\4 is postamble (...)
# (use ? pattern delimiters).
DATEPAT='\(.*@(#).* \)\(v[.0-9]* \)\([0-9]*/[0-9]*/[.0-9]*\)\(.*\)'

vfile=version.c
vfiles="version.c tests/version.t ksh.Man"

version=`sed -n "s?$DATEPAT?\2?p" < $vfile`
odatev=`sed -n "s?$DATEPAT?\3?p" < $vfile`
odate=`echo "$odatev" | sed 's?\..*??'`
ov=`echo "$odatev" | sed 's?[^.]*\.*??'`

date=`date '+%y/%m/%d' 2> /dev/null`
case "$date" in
[0-9]*/[0-9]*/[0-9]*) ;;
*)
	# old system - try to compensate...
	date=`date | awk 'BEGIN {
		months["Jan"] = 1; months["Feb"] = 2; months["Mar"] = 3;
		months["Apr"] = 4; months["May"] = 5; months["Jun"] = 6;
		months["Jul"] = 7; months["Aug"] = 8; months["Sep"] = 9;
		months["Oct"] = 10; months["Nov"] = 11; months["Dec"] = 12;
	    } {
		if (months[$2])
			mon = sprintf("%02d", months[$2]);
		else
			mon = $2;
		printf "%02d/%s/%02d\n", $6 % 100, mon, $3;
	    }'`
esac

if test x"$odate" = x"$date"; then
	v=".$ov"
	if test -z "$ov" ; then
		v=1
	else
		v=`expr $ov + 1`
	fi
	date="$date.$v"
fi

for i in $vfiles; do
    bfile=$i.bak
    tfile=$i.new
    # try to save permissions/ownership/group
    cp -p $i $tfile 2> /dev/null
    if sed "s?$DATEPAT?\1$version$date\4?" < $i > $tfile; then
	    if cmp -s $i $tfile; then
		echo "$i not changed, not updating"
		rm -f $tfile
	    else
		rm -f $bfile
		ln $i $bfile || exit 1
		mv $tfile $i || exit 1
	    fi
    else
	    echo "$0: error creating new $i" 1>&2
	    exit 1
    fi
done

exit 0
