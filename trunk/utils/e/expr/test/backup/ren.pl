foreach (@ARGV)
{
	$n = $_ . ".tst";
	print "$_ -> $n\n";
	system "mv $_ $n";
}
