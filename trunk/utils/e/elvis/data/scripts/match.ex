" Defines a "match" alias, which moves the cursor to a matching word.  The
" lists of matching words can be easily changed.
" Contributed by Ian Utley (iu@apertus.uk.com)

alias match {
    "Move to the line where the pairing keyword is located
    local a b d x y i nowrapscan magic magicchar=^$.[* noignorecase
    "x and y are lists of matching words -- we'll seek forward from x words
    "and backward from y words.  The words listed here are appropriate for
    "Unix shell scripts.
    set x="if/then/case/do/begin/repeat" y="fi/else/esac/done/end/until"
    let a=current("word")
    if (a == "")
    then error Cursor is not on a word
    set b=""
    while b=="" && x!="."
    do {
	if a==dirfile(x)
	then {
	    let b=dirfile(y)
	    set d=forward
	}
	if a==dirfile(y)
	then {
	    let b=dirfile(x)
	    set d=backward
	}
	let x=dirdir(x)
	let y=dirdir(y)
    }
    if ( b=="" )
    then error (a) is not a matchable keyword 
    mark c
    set i=1
    if (d=="forward")
    then {
        while ( i != 0 )
        do {
            set i=0
            try eval /\<(b)\>
            else {
                " Failed to locate a match
                'c
                error No matching (b) located
            }
            mark d
            eval 'c,'d global /\<(a)\>/ let i=i+1
            eval 'c,'d global /\<(b)\>/ let i=i-1
        }
    }
    if (d=="backward") 
    then {
        while ( i != 0 )
        do {
            set i=0
            try eval ?\<(b)\>
            else {
                " Failed to find a match
                'c
                error No matching (b) located
            }
            mark d
            eval 'd,'c global /\<(a)\>/ let i=i+1
            eval 'd,'c global /\<(b)\>/ let i=i-1
            " global command has moved cursor back to 'c
            'd
        }
    }
}
