# kshrc.ksh startup file for OS/2 version of ksh

set -o trackall
set -o ignoreeof

alias a:='cd a:.'
alias b:='cd b:.'
alias c:='cd c:.'
alias d:='cd d:.'
alias e:='cd e:.'
alias f:='cd f:.'
alias g:='cd g:.'
alias h:='cd h:.'
alias i:='cd i:.'
alias j:='cd j:.'
alias k:='cd k:.'
alias l:='cd l:.'
alias m:='cd m:.'

alias h='fc -l'
alias j='jobs'
#alias which='type'
alias back='cd -'
alias cls='print -n "\033[H\033[2J"'

alias dir='cmd /c dir'
alias del='cmd /c del'
alias erase='cmd/c erase'
alias copy='cmd /c copy'
alias start='cmd /c start /f'
alias path='print -r $PATH'

alias ll='ls -lsAFk'
alias lf='ls -CAFk'
alias cp='cp -p'
alias ls='ls -F'

clock_p () {
PS1='${__[(H=SECONDS/3600%24)==(M=SECONDS/60%60)==(S=SECONDS%60)]-$H:$M:$S}>'
typeset -Z2 H M S; let SECONDS=`date '+(%H*60+%M)*60+%S'`
}
#function needed by add_path, pre_path, and del_path
no_path () {
  eval _v="\$${2:-PATH}"
  case \;$_v\; in
    *\;$1\;*) return 1 ;;           # no we have it
  esac
  return 0
}
#if $1 exists and is not in path, append it, or prepend it
add_path () {
  [ -d ${1:-.} ] && no_path $* && eval ${2:-PATH}="\$${2:-PATH}\;$1"
}
pre_path () {
  [ -d ${1:-.} ] && no_path $* && eval ${2:-PATH}="$1\;\$${2:-PATH}"
}
#if $1 is in path then remove it
del_path () {
  no_path $* || eval ${2:-PATH}=`eval print -f '\;$'${2:-PATH}'\;' | sed -e "s!;$1;!;!g" -e "s!^;!!" -e "s!;\\$!!" -e "s!;!\\\\\;!g"`
}

unalias login newgrp

if [ "$KSH_VERSION" = "" ]
then PS1='$PWD>'
     return               #bail out for sh which doesn't have edit modes
fi

set -o emacs
bind ^Q=quote
bind ^I=complete
#bind ^[^[=list-file

#The next four have been preprogrammed
bind ^0H=up-history
bind ^0P=down-history
bind ^0K=backward-char
bind ^0M=forward-char

bind ^0s=backward-word
bind ^0t=forward-word
bind ^0G=beginning-of-line
bind ^0O=end-of-line
bind ^0w=beginning-of-history
bind ^0u=end-of-history
bind ^0S=eot-or-delete


FCEDIT=t2
PS1='[!]$PWD: '
function pushd { 
        if [ $# -eq 0 ]
        then    d=~
                set -A dirstk ${dirstk[*]} $PWD
                cd $d
        else    for d in $* 
                do      if [ -d $d ] && [ -r $d ] && [ -x $d ]
                        then    set -A dirstk ${dirstk[*]} $PWD
                                cd $d
                        else    echo "$d: Cannot change directory"
                                break
                        fi
                done
        fi
        echo ${dirstk[*]} $PWD
        unset d ;
}
 
function popd { 
        if [ ${#dirstk[*]} -gt 0 ]
        then    let n=${#dirstk[*]}-1
                cd ${dirstk[$n]}
                unset dirstk[$n]
                echo ${dirstk[*]} $PWD
        else    echo "popd: Directory stack empty"
        fi
        unset n ; 
}
