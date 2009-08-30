#!/usr/bin/perl

while (<>)
{
	chop;
	$_ .= " > /dev/null; echo Retval = \$\?\n";
	print;
}
