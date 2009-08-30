# The VMS POSIX `c89' command writes any/all diagnostic info to stdout
# rather than stderr, confusing configure tests which capture error output.
#
# Also, the VMS linker issues a warning for any undefined symbol, but that
# does not inhibit creation of the final executable file, again confusing
# configure.  As an added complication, there's not enough control of the
# linker to put the map file with chosen name into the current directory.
#
if [ -f ~/_posix-cc.map ] ; then  rm -f ~/_posix-cc.map* ; fi
c89 -Wc,nowarn -Wl,nodebug -Wl,map=_posix-cc.map $* ; x=$?
if [ -f ~/_posix-cc.map ] ; then
  if [ -n "`fgrep LINK-W-USEUNDEF ~/_posix-cc.map`" ] ; then  x=1 ; fi
  rm -f ~/_posix-cc.map*
fi
if [ x -ne 0 ] ; then  echo "c89 reports failure" 1>&2 && exit 1 ; fi
exit 0
