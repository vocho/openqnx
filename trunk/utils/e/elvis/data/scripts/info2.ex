" This file tries to implement an "info:" protocol, for viewing info pages
" inside elvis.  It doesn't quite succeed, but it comes close.
" 
" This assumes your info pages are stored in /usr/share/info.  If they're
" stored someplace else, then you'll need to edit the "local d=..." line below.

alias readINFO {
  local d=/usr/share/info
  local report=0
  local magic magicchar=^$.[* noignorecase
  local m
  if "!(/)2" == "/"
  then {
    r (d/"dir")
    try %s/&/\&amp;/g
    try %s/</\&lt;/g
    try %s/>/\&gt;/g
    try %s/^\* \([^:]*\): (\([^)]*\))\(.*\)\.$/* <a href="info:\2#\3">\1<\/a>: (\2)\3/
    try %s/^\* \([^:]*\): (\([^)]*\))\(.*\)/* <a href="info:\2#\3">\1<\/a>: (\2)\3/
  }
  else {
    if exists(d/"!2.info")
    then r !!cat (d)/!2.info (d)/!2.info-* 2>/dev/null
    eval r !!gzip -d -c (d)/!2.info*.gz (d)/!2.info-*.gz 2>/dev/null
    try 1,/^File: .* Node: !(Top)3,/-1 d
    try %s/&/\&amp;/g
    try %s/</\&lt;/g
    try %s/>/\&gt;/g
  }
  try %s/^$/<hr>/
  try %s/^\(File: .* Node: \)\([^:,]*\)/+2s,.*,<a name="\2">\&<\/a>,/x
  try g/^File: /s/\(Prev\|Next\|Up\): \([^:,]*\)/\1: <a href="#\2">\2<\/a>/g
  try %s/<a href="#(dir)">/<a href="info:">/g
  try %s/^\* \(.*\)::/* <a href="info:!2#\1">\1<\/a>::/
  1i <pre>
  $a </pre>
  se bufdisplay=html noinitialsyntax nomod
}

alias info sp info:!1
