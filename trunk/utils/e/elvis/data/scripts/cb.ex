" cut buffer operations: cbsave, cbload, cbshow
alias cbload {
 "Load cut-buffers from a file
 local b report=0 magic magicchar=^$.[* noignorecase
 let b=buffer
 if "!1" == ""
 then error cbload requires a file name
 e !1
 %s:^--CBS-- \([a-z]\)$:+;/^--CBS--/-1 y \1:x
 eval buffer (b)
}

alias cbsave {
 "Save cut-buffers to a file
 local a b report=0
 let b = buffer
 if "!1" == ""
 then error cbsave requires a file name
 e !1
 %d
 let a = 'a'
 while a <= 'z'
 do {
  if buffer("Elvis cut buffer ";char(a))
  then {
   eval $ a --CBS-- (char(a))
   eval $ put (char(a))
  }
  let a = a + 1
 }
 $ a --CBS--  
 w!?
 eval buffer (b)
}

alias cbshow {
 "Show contents of cut-buffers
 local b c i l="!*" q u s
 if l == ""
 then let l = "abcdefghijklmnopqrstuvwxyz123456789"
 echo Buf\| Size & Type \| Contents
 echo ---+-------------+----------------------------------------------------
 for i (1 .. strlen(l))
 do {
  let c = ((l;" ") << i) >> 1
  let b = "Elvis cut buffer ";c
  if buffer(b)
  then {
   (=b) let u = putstyle << 4;
   (=b) let q = u=="char" ? bufchars - 1 : buflines
   let s = q; " "; u; (q == 1 ? " " : "s")
   (=b) calc " "; c; " |"; s >> 12; " | "; line(b,1) << 52
  }
  let i = i + 1
 }
}
