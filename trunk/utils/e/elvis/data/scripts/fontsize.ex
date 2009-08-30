"For the 'x11' user interface only, this script adds a FontSize button to the
"toolbar which allows you to choose the font size used by elvis.
gui FontSize;"fontsize"(oneof tiny small medium large huge current)s=s?s:"current"
gui FontSize:eval fontsize (s)
alias fontsize {
 local n
 switch "!(large)1"
  case huge
  case 20 set n=20 s=huge
  case large
  case 15 set n=15 s=large
  case medium
  case 13 set n=13 s=medium
  case small
  case 10 set n=10 s=small
  case tiny
  case 7 set n=7 s=tiny
  case current set n=0 s=current
  default {
   if isnumber("!1")
   then set n="!1" s=current
   else error Invalid size
  }
  if n > 0
  then let font="*-fixed-medium-r-normal--";n;"-*-iso8859-1"
}
