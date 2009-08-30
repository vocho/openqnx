"This script defines an :ellipse alias.  If invoked with two arguments,
"they are assumed to be width & height.  If invoked with one argument,
"it is assumed to be width, and the height is computed as half the width
"(since in most fonts, characters are twice as high as they are wide).
"With no arguments, it uses either textwidth or columns as the width.
alias ellipse {
  " draw an ellipse using * characters, of a given width and height.
  local h w x y r s
  let w = !(textwidth ? textwidth : columns)1
  let h = !(w / 2)2
  let y = h / 2
  let r = (h * h * w * w) / 4
  let x = 0
  while y > 0
  do {
    while x * x * h * h + y * y * w * w <= r
    do let x = x + 1
    let s = (" " * (w/2 - x)); ("*" * (x * 2))
    "let s = "i\n";s;"\n";s;"\n.\n"
    "eval (s)
    eval a ,(s)
    eval i ,(s)
    let y = y - 1
  }
  %s/^,
}
