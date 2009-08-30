"defines a :inregion alias
alias inregion {
  " execute a command for each line that happens to be in a given region
  if "!1" == ""
  then error usage: inregion facename excmdline...
  normal mz
  !%g/^/ {
    if current("region") == "!1"
    then !2*
  }
  normal `z
}
