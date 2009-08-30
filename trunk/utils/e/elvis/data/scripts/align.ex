" Defines an :align alias for aligning = signs, or some other delimiter.  This
" can be handy when you're trying to make a Makefile look pretty.
" Contributed by Ian Utley (iu@apertus.uk.com)

alias align {
    "Align any = signs (or other given text) in selected line
    local f=0 i=0 k report=0 nosaveregexp magic magicchar=^$.[* noignorecase
    "
    " The following if tests to see if we have visually highlighted lines.
    "
    if ( !> !!= "" )
    then {
        !< mark a
        !> mark b
        let f=1
    }
    if ( f == 1)
    then {
        "
        " Initialise i which will store the alignment column.
        " Mark the current line to return the cursor at the end.
        "
        set i=0
        mark z
        "
        " Remove any whitespace before the alignment character.
        "
        'a,'b s/[ 	]*!(=)\$/!(=)\$/
        "
        " We could be aligning != <= or >= so we want to keep this letter
        " near. Of course we may not be aligning an equals but we commonly do.
        "
        if ( "!(=)\$" == "=" )
        then {
            'a,'b s/[	 ]*\([!!<>]*\)!(=)\$[	 ]*/ \1!(=)\$ /
        } 
        "
        "
        let f=0
        'a,'bglobal /!(=)\$/ {
            " 
            " Special case for the top line as -1 will not work.
            "
            if ( current("line") == 1 )
            then {
                1 insert ""
                let f=1
            }
            -1
            /!(=)\$
            "
            "
            " Remember the largest column number for alignment.
            "
            if (current("column")>i)
            then let i=current("column")
            "
            " Special case removal
            "
            if ( current("line") > 1 && f == 1)
            then {
                1 delete
                let f=0
            }
        }
        "
        " Do the alignment.
        "
        let f=0
        'a,'bglobal /!(=)\$/ {
            " 
            " Special case for the top line as -1 will not work.
            "
            if ( current("line") == 1 )
            then {
                1i ""
                let f=1
            }
            -1
            /!(=)\$
            "
            " Not sure why I need to add +1
            "
            let k=i-current("column")+1
            s/\([!!<>]*\)!(=)\$/                                                                                \1!(=)\$/
            eval s/ *\\\( \{(k)\}[!!<>]*!(=)\$\\\)/\1
            "
            " Special case removal
            "
            if ( current("line") > 1 && f == 1 )
            then {
                1 delete
                let f=0
            }
        }
        "
        " Return the cursor to the line it was previously on.
        "
        'z
    }
}
