while (<>)
{
	s/\(/\\(/g;
	s/\)/\\)/g;
	print
}
