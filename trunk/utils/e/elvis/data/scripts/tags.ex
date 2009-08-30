" This file contains an alias and autocmd which, together, allow you to set
" the tags option to a traditional space-delimited value.  Elvis will then
" automatically convert it to the new colon-delimited format.

alias fixtags {
  " Convert spaces to colons in the tags option
  local i t
  let t = tags[1]
  for i (2 .. tags[0])
  do let t = t : tags[i]
  let tags = t
}
au optchanged tags fixtags
