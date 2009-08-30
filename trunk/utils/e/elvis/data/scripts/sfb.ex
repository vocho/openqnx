"Visual directory display a.k.a split file browse
"Written by Dan Fandrich <dan@fch.wimsey.bc.ca>
switch os
case unix  alias sfbrowse split +"fbrowsetweak !*" !ls !*
case win32 alias sfbrowse split +"fbrowsetweak !*" !dir /b /a-d !*
"What are the equivalent commands for OS/2 and whatever other OSes are valid?
default    alias sfbrowse split +"fbrowsetweak !*" !dir !*

alias sfb sfbrowse
alias fbrowsetweak {
 "Turn a directory listing into a browsable HTML page
 local report=0 nosaveregexp magicchar=.*
 1,$s/.*/<LI><A HREF="&">&<\/A>/
 local d="!*"
 if d=="" || d=="."
 then eval set d=(getcwd())
 1
 eval insert <HTML><HEAD><TITLE>(d)</TITLE></HEAD><H1>(d)</H1><BODY><MENU>
 $a </MENU></HTML>
 if !userprotocol
 then eval file (d)/
 else 1i <!DOCTYPE user protocol>
 "Move to the first file so that <tab> will move to the second
 1
 /HREF
 set bufdisplay=html nomod locked
 display html
}
