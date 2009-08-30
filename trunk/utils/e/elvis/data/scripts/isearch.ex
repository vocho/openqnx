" This script defines :ilist and :isearch aliases, and some vim-compatible
" maps that use them.  They search for the first usage of an identifier name
" in the current file.

alias ilist {
" list all lines containing a given word
push
!%g/\<!*\>/
pop
}

alias isearch {
" search for a word within a range (default whole file)
local l=""
push
!%g/\<!*\>/ {
 if l == ""
 then let l = current("line")
}
eval (l)p
pop
}

map [i yiw:isearch 
map ]i yiw:+,$isearch 
map [I yiw:ilist 
map ]I yiw:+,$ilist 
