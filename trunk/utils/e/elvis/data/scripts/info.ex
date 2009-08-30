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
  let m=display
  display normal
  if "!(/)2" == "/"
  then {
    eval r !!cd (d); ls *.info.gz
    try {
      %s/\(.*\)\.info\.gz/<a href="info:\1">\1<\/a>/
      se bufdisplay=html noinitialsyntax
    }
    se nomod
  }
  else {
    eval r !!gzip -d -c (d)/!2.info.gz (d)/!2.info-*.gz
    try {
      1,/^File: .* Node: !(Top)3,/-1 d
      try %s/&/\&amp;/g
      try %s/</\&lt;/g
      try %s/>/\&gt;/g
      %s/^$/<hr>/
      %s/^\(File: .* Node: \)\([^:,]*\)/+2s,.*,<a name="\2">\&<\/a>,/x
      g/^File: /s/\(Prev\|Next\|Up\): \([^:,]*\)/\1: <a href="#\2">\2<\/a>/g
      try %s/<a href="#(dir)">/<a href="info:">/g
      %s/^\* \(.*\)::/* <a href="info:!2#\1">\1<\/a>::/
      1i <pre>
      $a </pre>
      se bufdisplay=html noinitialsyntax
    }
    se nomod
  }
  eval display (m)
}

alias info sp info:!1
