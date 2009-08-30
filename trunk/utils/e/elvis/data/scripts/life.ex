" This script adjusts some options for speed, and then loads the "life"
" macros from a file named 'macros/life1.5.mac' which isn't included here.

se noru nosmd report=100 blkcache=200 blkhash=300 blkgrow=32 animation=30
if gui == "x11"
then set notoolbar nostatusbar noscrollbar
if feature("incsearch")
then set nois
so macros/life1.5.mac
