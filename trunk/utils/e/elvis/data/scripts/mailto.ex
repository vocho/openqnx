" This script attempts to define the mailto: and man: protocols.  This
" feature of elvis is still changing rapidly, so this might not work.
" Also, the mailto: alias only works on Unix-like systems.

alias readMAILTO {
  "initialize a mailto: message
  se noro reol=text
  if exists($HOME/".signature")
  then {
    read ~/.signature
    1 i --
  }
}
alias writeMAILTO {
  "send a mailto: message

  " This is a lot more complex that one would think.  The mail program forks
  " off a spooler, and the spooler inherits the stdout/stderr file descriptors.
  " This has the unfortunate side-effect of making elvis wait until the mail
  " queue is emptied, after this message and any other pending messages have
  " been uploaded to the mailserver.  Yuck!  To avoid that, we redirect the
  " mail program's stdout/stderr to /dev/null.
  w !!mail -s"!(no subject)subject=" !2 >/dev/null 2>&1
  se nomod
}

alias readMAN {
 local report=0 nosaveregexp
 local magic magicchar=^$.[* noignorecase
 r !!man !2
 set bd=man
 try 1 s/^Reformatting.*ait\.\.\.$//
 try % s/\\/\\\\/g
 try % s/_\(.\)/\\fI\1\\fR/g
 try % s/.\(.\)/\\fB\1\\fR/g
 try % s/\\fR\\fB//g
 try % s/\\fR\\fI//g
 1 i .nf
 set nomod
}
