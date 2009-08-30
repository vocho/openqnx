" This script defines :piglatin, an English-to-pig-latin translator.
" It translates the current line by default, or you can give it a range of
" lines to translate.
alias piglatin {
 "Translate English to pig latin
 local magic magicchar=^$.[* noignorecase
 try !% s/\<[aeiouAEIOU][a-zA-Z]*\>/&way/g
 try !% s/\<\([b-dfghj-np-tv-z]\+\)\([a-zA-Z]*\)\>/\2\1ay/g
 try !% s/\<\([B-DFGHJ-NP-TV-Z][b-dfghj-np-tv-z]*\)\([a-zA-Z]*\)\>/\u\2\l\1ay/g
}
