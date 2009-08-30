/*
 * $Id: keynames.c 153052 2008-08-13 01:17:50Z coreos $
 */

#include <test.priv.h>

int main(int argc, char *argv[])
{
	int n;
	for (n = -1; n < 512; n++) {
		printf("%d(%5o):%s\n", n, n, keyname(n));
	}
	return EXIT_SUCCESS;
}
