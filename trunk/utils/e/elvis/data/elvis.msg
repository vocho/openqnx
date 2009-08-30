# This is a sample "elvis.msg" file, for translating messages.  Each line
# contains a terse message followed by a colon, some optional whitespace, and
# whatever text you want to use as the verbose form of that message.  Lines
# that start with '#' are ignored simply because no terse messages happen to
# start with '#'.  Blank lines are also allowed.  The terse forms of most
# messages are listed in the "elvistrs.msg" file.
#------------------------------------------------------------------------------
no match above:		     Hit top of file without finding a match
no match below:		     Hit bottom of file without finding a match
$1 is a boolean option:	     Use "set $1" or "set no$1" to enable or disable $1
termcap needs $1:	     This termcap entry lacks the :$1: capability
read $1, $2 lines, $3 chars: Read $1(readonly?" [READONLY]"), $2 lines, $3 chars
nothing to fold there:       To create a new folded region, you must give a name

# Some words that are stored as options: True False Submit Cancel Help
Submit:Okay

# This demonstrates how to cause specific messages to beep, regardless of the
# settings of the "errorbells" and "warningbells" options.  This example is
# commented out; delete the initial '#' to activate it.
#------------------------------------------------------------------------------
#wrapped:		     Wrapped

# The following shows how to silence a typical error message -- just don't
# have any text after the colon character.
#------------------------------------------------------------------------------
not while in open mode:

# The following line is used to control the output of the :file command and
# visual ^G command.  This is by far the most complex message used by elvis.
#------------------------------------------------------------------------------
(filename)(readonly?" [READONLY]")(modified?" [MODIFIED]")(!edited?" [NOT EDITED]")(newfile?" [NEW FILE]") ($1 * 100 / $2)%:(filename)(readonly?" [READONLY]")(modified?" [MODIFIED]")(!edited?" [NOT EDITED]")(newfile?" [NEW FILE]") line $1 of $2 \(($1*100 / $2)%\)
