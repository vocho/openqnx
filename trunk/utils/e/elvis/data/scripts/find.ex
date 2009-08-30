"This script defines a :find alias.  It repeats the previous search in the
"current file, or if that fails then it tries the next file.
alias find {
 " repeat previous search in this file, or in :next file if that fails
 try /
 else {
  next
  /
}
