"This file contains some maps that are useful when editing XML or HTML code.

" When a > is input at the end of a tag, automatically add the closing tag
map input > noremap >mmF<ye`mpa>bi/`mli

" Make the % command match XML tags.  This doesn't work as the destination of
" operator commands or during visual selections, though, so we only map it for
" commands.
map command % noremap :xmlmatch

" Add <> to matchchar
set matchchar="{}()[]<>"

alias xmlmatch {
    "Move the cursor to the matching HTML or XML tag name

    "If not on tag name, then do the normal % character match
    if current(/\i/) == ""
    then normal %
    else {
	" d is the direction to search
	" i counts nested tag pairs
	" n is the tag name without any punctuation
	" t is origin tag name without args or >
	local d i n t

	"Configure search parameters to be "normal"
	local nowrapscan magic magicchar=^$.[* magicname noincsearch ignorecase nosmartcase

	"HTML ignores case, but XML is case sensitive
	if (tolower(dirext(filename)) << 4) == ".htm"
	then set ignorecase
	else set noignorecase

	"This particular alias doesn't really change the file, but it uses
	"the :normal command which *can* change the file and hence is not
	"allowed on locked buffers.  Temporarily turn off locking.
	local nolocked

	"Get the current tag name
	let t=current(/<\/\i\+/)
	if t
	then {
	    let d="backward"
	    let n=t[,3...]
	}
	else {
	    let t=current(/<\i\+/)
	    if t
	    then {
		let d="forward"
		let n=t[,2...]
	    }
	    else error cursor isn't on a tag name
	}

	" move to the start of this tag, so we don't immediately find it in
	" the following search loop and mistake it for a nested tag.
	normal F<

	"search for the tag, for nested tags.  Stop on the matching one
	normal mx
	try {
	    set i=1
	    while i > 0
	    do {
		"find the next opening or closing tag, in the proper direction
		switch d
		case forward normal /<\/\?\=$n\>
		case backward normal ?</*\=$n\>

		"count nested tag levels
		if current(/<\/\?$n/) == t
		then let i = i + 1
		else let i = i - 1
	    }
	}
	else normal `x
    }
}
