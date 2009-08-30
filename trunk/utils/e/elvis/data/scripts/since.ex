"Defines some aliases and autocmds for highlighting lines that have changed.

alias since {
  " Highlight the differences between current buffer and its file
  "local a s=since f t=/tmp/diff report=0 u=/tmp/diff.ex
  set s=since t=/tmp/diff report=0 u=/tmp/diff.ex
  let f = "!*" || filename
  if !exists(filename)
  then error usage: since [filename]
  eval unr (s)
  w! (t)
  let a = "diff" f t
  let a = a;"| sed -n 's/^[0-9,]*a\\([0-9,]*\\)$/\\1reg diff added/p;s/^[0-9,]*c\\([0-9,]*\\)$/\\1reg diff changed/p' >";u
  eval !!(a)
  safely source (u)
  eval !!rm (t)
}

alias rcssince {
  " Highlight the differences between the current buffer and an RCS version
  local s=rcssince t=/tmp/diff report=0
  eval unr (s)
  if exists("RCS"/filename;",v") || exists(filename;",v")
  then {
    w! (t)
    let a = "co -p !1 2>/dev/null" filename "| diff - " t
    let a = a;"| sed -n 's/^[0-9,]*a\\([0-9,]*\\)$/\\1reg" s "added/p;s/^[0-9,]*c\\([0-9,]*\\)$/\\1reg" s "changed/p'"
    let a = shell(a)
    eval (a)
    eval !!rm (t)
  }
}

aug since
  au!
  au Edit * {
    if filename
    then '[,']reg unsaved
  }
  au BufWritePost * %unr unsaved
  au BufReadPost * rcssince
aug END
color since on orange
color rcssince on orange
color unsaved on tan
