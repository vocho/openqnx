#/usr/bin/perl

@simStrOps = 
(
	"''",
	"abc",
	0, -1, 1, 5
);

@simStrOps2 = 
(
	"''",
	"abc",
	0, 2
);

@operands = 
(
	"''", 
	"abc",
	"def", 
	"ABC", 
	"DEF",
	"'   '", 
	"'!@#'",
	"',.:'",
	"\\\"123\\\"", 
	"\\\"456\\\"", 
	0, -25, 317
);

@boolOps = 
(
	"|", 
	"&"
);

# simple relation operators
@simRelOps = 
(
	"=",
	"<"
);

@otherRelOps = 
(
	"!=",
	">",
	">=",
	"<="
);

@relOps =
(
	"=", 
	"<", 
	">", 
	"<=", 
	">=", 
	"!="
);

@nums = ( 0, 1, 10, -1, -3);
@arithOps =  ("+", "-", "*", "/", "%");

# simple tests
#print "expr\n";
#foreach $op1 (@operands)
#{	print "expr $op1\n";}


# bool tests
#foreach $op1 (@simStrOps)
#{	foreach $opr (@boolOps)
#	{	foreach $op2 (@simStrOps)
#		{	print "expr $op1 \'$opr\' $op2\n";}}}

# compare-simple tests
#foreach $op1 (@operands)
#{	foreach $opr (@simRelOps)
#	{	foreach $op2 (@operands)
#		{	print "expr $op1 \'$opr\' $op2\n";}}}

# compare-simple2 tests
#foreach $op1 (@simStrOps2)
#{	foreach $opr (@otherRelOps)
#	{	foreach $op2 (@simStrOps2)
#		{	print "expr $op1 \'$opr\' $op2\n";}}}

# arith tests
#foreach $op1 (@nums)
#{	foreach $opr (@arithOps)
#	{	foreach $op2 (@nums)
#		{	print "expr $op1 \'$opr\' $op2\n";}}}

# arith-string tests
#foreach $op1 (@simStrOps2)
#{	foreach $opr (@arithOps)
#	{	foreach $op2 (@simStrOps2)
#		{	print "expr $op1 \'$opr\' $op2\n";}}}

# parse-bignums tests
#@prefix = ("", "+", "-");
#@spaces = (0, 1, 2);
#@digits = (3, 4, 5);
#foreach $p (@prefix)
#{	foreach $s (@spaces)
#	{	foreach $d (@digits)
#		{	$o = "$p" . (" " x $s) . ("1" x $d);
#			print "expr $o + $o\n";}}}
