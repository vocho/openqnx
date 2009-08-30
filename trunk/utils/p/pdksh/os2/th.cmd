extproc ksh
#
# Simple script to find perl and run it
# for os2

IFS=\;$IFS
perl=
for i in $PATH; do
    [ X"$i" = X ] && i=.
    for j in perl perl5x perl5 ; do
	[ -x $i/$j.exe ] && perl=$i/$j.exe && break 2
    done
done

[ X"$perl" = X ] && {
	echo "$0: can't find perl - bye\n" 1>&2
	exit 1
    }

perlpath=`dirname $perl`
if [ `basename $perlpath` = bin ]
then perlpath=`dirname $perlpath`
fi
if [ "$PERL5LIB" = "" ]
then PERL5LIB=$perlpath/lib
fi
export PERL5LIB
exec $perl "$@"
