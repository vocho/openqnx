" This file creates some aliases and maps that make elvis work more like vim

" Some variations of the :map and :unmap commands
alias nmap	map command
alias nm	map command
alias vmap	map select
alias vm	map select
alias omap	map motion
alias om	map motion
alias imap	map input
alias im	map input
alias cmap	map history
alias cm	map history
alias noremap	map!? noremap
alias nore	map!? noremap
alias nnoremap	map noremap command
alias nn	map noremap command
alias vnoremap	map noremap select
alias vn	map noremap select
alias inoremap	map noremap input
alias ino	map noremap input
alias cnoremap	map noremap history
alias cno	map noremap history
alias nunmap	unmap command
alias nun	unmap command
alias vunmap	unmap select
alias vu	unmap select
alias ounmap	unmap motion
alias ou	unmap motion
alias iunmap	unmap input
alias iu	unmap unput
alias cunmap	unmap history
alias cu	unmap history

" Some redundant commands that work on visibly selected text
map noremap select r	g=
map noremap select x	d
map noremap select U	noremap gU
map noremap select u	noremap gu
map noremap select ~	noremap g~
map noremap select J	:j
map noremap select 	y:ta 
map noremap select o	g%
map noremap select O	g
"map noremap select R	S     ... except that "vS" isn't implemented yet

" Some 'g' commands that aren't built in to elvis
map g#		yiw??w
map g*		yiw//w
map gf		:eval find (current(/[^[:space:]<>"]*/))
map gm		:eval normal (columns/2+1;char(124))
map go		

" Some '^W" commands that aren't built in to elvis
map f		:eval sfind (current(/[^[:space:]<>"]*/))

" A few miscellaneous vim commands
alias find {
 " Locate a file in 'includepath', and then edit it
 local elvispath f
 if "!*" == ""
 then error cursor not on file name
 let elvispath=includepath
 let f=elvispath("!*")
 if f == ""
 then error "!*" not found in includepath
 else e!? (f)
}
alias fin find
alias sfind {
 " Locate a file in 'includepath', and then split it
 local elvispath f
 if "!*" == ""
 then error cursor not on file name
 let elvispath=includepath
 let f=elvispath("!*")
 if f == ""
 then error "!*" not found in includepath
 else sp (f)
}
alias sf sfind
alias sview split +"se ro"
alias sv sview
alias update {
 " Write a file, but only if modified
 if modified
 then !%write!? !*
}
alias up !%update!?
