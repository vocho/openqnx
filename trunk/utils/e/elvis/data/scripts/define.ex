"A macro for accessing the TechEncyclopaedia easily, without clutter.

alias define {
 "fetch a definition from TechEncyclopaedia
 local u="define:!1"
 if "!1" == ""
 then error usage: define TERM
 if "!2" != ""
 then let u=u;"+!2"
 if "!3" != ""
 then let t=u;"+!3"
 split (u)
}

alias readDEFINE {
 local report=0
 r http://www.techweb.com/encyclopedia/defineterm?term=!2
 set nolocked
 try 1,/termDefined/-1d
 try /^<tr>/,$d
 try g/<img .*>/d
 try %s/href="\/encyclopedia\/defineterm?term=/href="define:/g
 set bufdisplay=html
 set nomod locked
 try set nospell
}
