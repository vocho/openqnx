"This script defines a :decode alias, which handles simple letter<->number
"ciphers.  You tell it a starting letter and number, and it fills in the
"rest of the alphabet.
alias decode {
 "secret decoder ring
 local i=1 l=-1 n=!2
 try let l='!1'
 if l < 'a' || l > 'z' || n < 1 || n > 26
  then error usage: [lines] decode letter number
 while i <= 26
 do {
  try eval !% s/\<(n)\>/(char(l))/g
  let i=i+1
  let n=n+1
  if n > 26
   then set n=1
  if l == 'z'
   then let l='a'
   else let l=l+1
 }
}
