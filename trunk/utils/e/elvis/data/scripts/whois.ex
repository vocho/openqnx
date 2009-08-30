" This implements a "whois:domainname_or_ipaddress" protocol, and a
" ":whois domainname_or_ipaddress" command that uses it.  These aliases
" depend on the whois server at arin.net

alias readWHOIS {
	" Implements the whois: protocol
	r http://ws.arin.net/cgi-bin/whois.pl?queryinput=!2
	try 1,/<pre>/-1 d
	then i <html><body>
	try %s/<\/\?blockquote>//g
	try %s/HREF="\/cgi-bin\/whois.pl?queryinput=/HREF="whois:/g
	set bufdisplay=html nomodified noedited locked
}
alias whois sp whois:!1
