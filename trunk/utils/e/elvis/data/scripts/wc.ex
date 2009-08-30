"This script defines a :wc alias, which counts words

alias wc {
 "Count chars, words, and lines in the current file, or a given range of lines
 local c=0 w=0 l=0 nolocked
 !%g/^/ {
  let l=l+1
  let c=c+strlen(line()) + 1
  try s/\w\+/let w=w+1/xg
 }
 calc c "chars," w "words," l "lines"
}
