
" dark background default
alias colordbg {
	" get a dark random background: helps tell different elvis session
	local d n m
	let n="0,1,2"
	let m="0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f"
	eval let d="(rand(n))(rand(m))(rand(n))(rand(m))(rand(n))(rand(m))"

	" let's go
  	eval color normal cornsilk on #(d)
	eval color toolbar cornsilk3 on #(d)
	eval color tool #(d) on grey75
	eval color scrollbar grey75 on #(d)
	color idle like normal
	color lnum pink
 	color selection on #715d4b
	color hlsearch black on lightblue

	color cursor cornsilk on cornsilk
	color hexcursor bold black on gold
	color hexheading bold gold

	color italic italic sandybrown
	color graphic graphic sandybrown
	color underlined lightblue
	color emphasized darkolivegreen2
	color link lightblue
	color definition pink
	color bold bold gold
	color boxed sandybrown
	color fixed fixed orange
	color markup pink
	
	color comment italic orange
	color keyword gold
	color prep darkolivegreen2
	color prepquote darkolivegreen2
	color string mediumturquoise
	color char mediumturquoise
	color function cornsilk
	color variable cornsilk
	color number cornsilk
	color other darkolivegreen2

	color fold italic bold on SpringGreen4
	color hlobject1 on MidnightBlue
	color guide lightsteelblue1
}

" dark background 2 for bold type fonts
alias colordbg2 {
	local d n m
	let n="0,1,2"
	let m="0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f"
	eval let d="(rand(n))(rand(m))(rand(n))(rand(m))(rand(n))(rand(m))"
  	eval color normal cornsilk3 on #(d)
	eval color toolbar cornsilk3 on #(d)
	eval color tool #(d) on grey75

	color idle like normal
	color lnum pink2
 	color selection on grey40
	color hlsearch black on lightblue3

	color cursor cornsilk2 on cornsilk2
	color hexcursor bold black on gold
	color hexheading bold gold

	color italic italic sandybrown
	color graphic graphic sandybrown
	color underlined lightblue3
	color emphasized darkolivegreen3
	color link lightblue3
	color definition pink2
	color bold bold gold2
	color boxed sandybrown
	color fixed fixed orange2
	
	color comment italic orange2
	color keyword gold2
	color prep darkolivegreen3
	color prepquote darkolivegreen3
	color string #00aab4
	color char #00aab4
	color function cornsilk3
	color variable cornsilk3
	color number cornsilk3
	color other darkolivegreen3

	color fold on SpringGreen4
	color hlobject1 on MidnightBlue
	color guide lightsteelblue1
}

" bright background
alias colorbbg {
	local d n m
	let n="d,e,f"
	let m="0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f"
	eval let d="(rand(n))(rand(m))(rand(n))(rand(m))(rand(n))(rand(m))"
  	eval color normal black on #(d)
	eval color toolbar black on #(d)
	eval color tool #(d) on grey75

	color idle like normal
	color lnum pink
	color selection on PeachPuff1
	color hlsearch lightblue on black

	color cursor firebrick on firebrick
	color hexcursor bold gold4 on black
	color hexheading bold on gold4

	color italic italic darkorange4
	color graphic graphic brown
	color underlined blue
	color emphasized green4
	color link blue
	color definition pink4
	color bold bold gold4
	color boxed brown
	color fixed fixed darkorange3
	
	color comment darkorange3
	color keyword darkgoldenrod4
	color prep green4
	color prepquote green4
	color string turquoise3
	color char turquoise3
	color function black
	color variable black
	color number black
	color other darkolivegreen4
	color fold black on NavajoWhite1
	color hlobject1 on DarkSlateBlue
	color hlobject2 on DarkGreen
}

alias ft {
	set font=!^
}
" try
" fixed
" corona-bold14, courier14, neep-14
" corona16, screen16, serif16
" large
" interesting fonts : m, mr, t  

alias x11def {
	switch background
	case dark colordbg
	case light colorbbg
}
alias x11sm {
	colordbg2
}


" toolbar related : generate a tab for each buffer
" bug : cannot show the name properly when the filename has :?="~.
alias removesc {
	local c i g
	let g=f
	let f=""
	for i (1 .. strlen(g))
	do {
		let c = ((g;" ") << i) >> 1
 		if c=="?" || c=="=" || c==":" || c=="\""  || c=="~"
		then let c=" "
		let f=f;c
	}
}

alias rescanbuffers {
  local d f x
  gui newtoolbar
  gui Rescan:rescanbuffers
  gui Rescan"Redcan current buffers
  all {
    let x=file
    if x==""
    then let x=buffer
    let d=dirdir(x)
    let f=dirfile(x);" (";bufferid;")"
    removesc
    if d!="."
    then let f="..."/f
    eval gui (f) :openbuffer (bufferid)
    eval gui (f) ?bufferid != (bufferid)
    eval gui (f) "(x) \((bufferid)\)
  }
}
alias openbuffer {
	local i
	let i="#!^"
	if buffer(i)
	then {
		"keep track of the old buffer for "goback" alias
		let b=(bufferid)	
		eval b (i)
	}
	else {
		message buffer (i) is already gone.
		rescanbuffers
	}
}
alias toggletoolbar {
	local d
	eval let d=(toolbar)
	if !d
	then rescanbuffers
	set negtoolbar
	eval message show toolbar (toolbar)
}

gui Rescan:rescanbuffers
gui Rescan"Rescan current buffers
au BufCreate,BufDelete * rescanbuffers
map  :toggletoolbar

x11def
"set noicon
set nostatusbar
set nofocusnew
" for now no toolbar, it will show once you have more than one buffer
" and you can toogle it with 
set notoolbar 

