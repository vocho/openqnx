# use any precedenceN file as input on the command line to generate
# the precedenceN+1 test case file

if (@ARGV == 0)
{
	@lev = ("12 \\| 34", "34 \\& 13", "12 = 34", "12 + 34", "4 \\* 6");
}
else
{
	@lev = <>;
	foreach (@lev)
	{
		chop;
		s/expr//g;
	}
}

@levOp =
(
        "\\|", "\\&", "!=", "-", "/"
);

foreach $e (@lev)
{
	for ($i = 0; $i < @levOp; $i++)
	{
        	for ($j = $i; $j <= $i + 2 && $levOp[$j]; $j++)
	        {
        	        for ($n = 0; $n <= 3; $n++)
                	{
	                	if ($n == 0)
       		        	{print "expr $e $levOp[$j] $e\n";}
               		        elsif ($n == 1)
	                        {print "expr $e $levOp[$j] \\( $e \\)\n";}
	       	                elsif ($n == 2)
        	      	        {print "expr \\( $e \\) $levOp[$j] $e\n";}
                	       	elsif ($n == 3)
	                        {print "expr \\( $e \\) $levOp[$j] \\( $e \\)\n";}
			}
                }
        }
}
