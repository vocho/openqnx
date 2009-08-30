" Defines makehtml alias to convert plain text to HTLM source

alias makehtml {
 "Convert plain text to HTML source
 local report=0 m=text n=text b=false magic magicchar=^$.[* noignorecase
 "
 " m is current line mode -- one of "text", "pre", "ol", or "ul"
 " n is next line mode
 " b is a flag for detecting series of blank lines.
 " 
 " For each line...
 !%g /^/ {
  " Protect characters which are special to HTML
  try s/&/\&amp;/g
  try s/</\&lt;/g
  try s/>/\&gt;/g
  "
  " Convert uppercase lines into headings
  try s/^[A-Z0-9][A-Z0-9-.) 	]*$/<h1>&<\/h1>/
  then set n=text
  "
  " Convert horizontal lines into <hr> tags
  try s/^\s*[-=]\{10,}\s*$/<hr>/
  then set n=text
  "
  " Try to be clever about finding links
  try s/http:[^">, 	)]\+/<a href="&">&<\/a>/g
  try s/ftp:[^">, 	)]\+/<a href="&">&<\/a>/g
  try s/[a-zA-Z]\w*@[a-zA-Z][[:alnum:].-]\+/<a href="mailto:&">&<\/a>/g
  "
  " Convert asterisked lines into "ul" list items.
  try s/^\s*\* */<li>
  then set n=ul
  else {
   if m=="ul"
   then try s/^[^* 	]/set n=text/x
  }
  "
  " Convert numbered lines (other than headings) into "ol" list items.
  try s/^\s*\d\+[.)] */<li>
  then set n=ol
  else {
   if m=="ol"
   then try s/^[^0-9 	]/set n=text/x
  }
  "
  " if in normal text, then assume indented text is preformatted.
  if n=="text"
  then try s/^\s/set n=pre/x
  "
  " if in preformatted mode, then unindented lines revert to text mode
  if m=="pre" && n=="pre"
  then try s/^\S/set n=text/x
  "
  " Any non-blank line turns off the "b" flag.
  try s/./set b=false/x
  "
  " if not in preformatted text, then blank lines are paragraph breaks.
  " Avoid consecutive <p> tags, though.
  if m!="pre" && b=="false"
  then {
   try s/^$/<p>/
   then set b=true
  }
  "
  " if mode switched, then add tags for that.
  if m!=n
  then {
   if m!="text"
   then eval i </(m)>
   if n!="text"
   then eval i <(n)>
   let m=n
   set b=false
  }
 }
 "
 " if not in text mode, then terminate the mode
 if m != "text"
 then eval !> a </(m)>
 "
 " If converting the whole file, then add <html>...</html>
 if "!%" == ""
 then {
  $a </body></html>
  eval 1i <html><head><title>(htmlsafe(filename))</title></head><body>
  "
  " minor conveniences...
  set bufdisplay=html mapmode=html
  display html
  if filename != "" && tolower(dirext(filename) << 4) != ".htm"
  then eval file (dirdir(filename)/basename(filename)).html
 }
}
