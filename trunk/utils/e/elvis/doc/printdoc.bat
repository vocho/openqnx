REM () { :; }
REM This little script will print the documentation.  It does this by invoking
REM elvis on each documentation file in turn, telling elvis to print the file
REM via its :lpr command and then quit.  The complete manual should be about
REM 200 pages long.
REM
REM     THIS ASSUMES YOU HAVE ALREADY SET UP THE PRINTING OPTIONS!
REM
REM This script should work under DOS, Windows/NT, and the UNIX "ksh" shell or
REM clones such as "bash".  The first line of this file allows "sh" to accept
REM these REM lines without complaint, by defining it as a do-nothing function.

elvis -Gquit -clp elvis.html
elvis -Gquit -clp elvisvi.html
elvis -Gquit -clp elvisinp.html
elvis -Gquit -clp elvisex.html
elvis -Gquit -clp elvisre.html
elvis -Gquit -clp elvisopt.html
elvis -Gquit -clp elvisdm.html
elvis -Gquit -clp elvisgui.html
elvis -Gquit -clp elvisos.html
elvis -Gquit -clp elvisses.html
elvis -Gquit -clp elviscut.html
elvis -Gquit -clp elvismsg.html
elvis -Gquit -clp elvisexp.html
elvis -Gquit -clp elvistag.html
elvis -Gquit -clp elvisnet.html
elvis -Gquit -clp elvistip.html
elvis -Gquit -clp elvistrs.msg
elvis -Gquit -clp elvisqr.html
elvis -Gquit -clp elvis.man
elvis -Gquit -clp ctags.man
elvis -Gquit -clp ref.man
elvis -Gquit -clp fmt.man
