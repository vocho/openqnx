" Use the "dict" program to fetch the definition of a word
alias readDICT {
	local nolock
	r !dict --html "!2"
	try %s/{\([^,}]*\), \([^}]*\)}/{\1}, {\2}/g
	try %s/{\([^,}]*\), \([^}]*\)}/{\1}, {\2}/g
	try %s/{\([^,}]*\), \([^}]*\)}/{\1}, {\2}/g
	try %s/{\(\w[[:alpha:] .]*\)}/<a href="dict:\1">\1<\/a>/g
	try %s/[Hh][Tt][Tt][Pp]:\/\/[^ ,>")&]*/<a href="&">&<\/a>/g
	set bufdisplay=html
}
alias dict {
	" Show the definition of a term in a new window
	local w="!*"
	let w =~ s/ /\\ /g
	eval sp dict:(w)
}
