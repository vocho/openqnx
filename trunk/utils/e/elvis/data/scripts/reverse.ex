"This script defines a :reverse alias, which reverses the order of the
"characters in a line.  By default it acts on the current line, but you
"can also give it a range of lines to alter.  Leading whitespace is
"unaffected.
alias reverse {
  "Reverse the characters in a line
  !(.)% g /\S/ {
    local t=true
    s/^\s*/&<@>/
    while t
    do {
      try s/\(<@>.*\)\(.\)/\2\1/
      else set t=false
    }
    s/<@>$/
  }
}
