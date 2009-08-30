#! /bin/sh

#
# this uses the -r command line option to sed (--rx-test) which changes
# the regex syntax used to that of POSIX egrep for the purposes of these 
# tests.
#

sed=${1-sed}

$sed -e "\
  /^0:/s,0:\(.*\):\(.*\),echo '\2' | $sed -re '/\1/d' | $sed -e,
  /^1:/s,1:\(.*\):\(.*\),echo '\2' | $sed -re '/\1/!d' | $sed -e,
  /^2:/s,2:\(.*\):\(.*\),echo '\2' | $sed -re '/\1/p' | $sed -e '/bad regexp:/d' -e," | \
  awk '{print $0 " s\"/.*/Test\ #" NR "\ failed!/\""}'
