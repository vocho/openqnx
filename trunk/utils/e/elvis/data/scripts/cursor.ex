" This file defines a :cursor alias, which moves the cursor to a given line
" of a given file.  This is handy if you want some external program to move
" elvis' cursor -- you can (under X-windows at least) say...
"
"	elvis -client -c "cursor $line $file"

alias cursor {
   "Move the cursor to a given line, in a given file
   if !isnumber("!1") || !exists("!2")
   then error Usage: cursor linenumber filename
   if filename != "!2"
   then {
      if buffer("!2")
      then (!2)!1
      else e +!1 !2
   }
   else !1
}
