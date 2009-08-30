#include <stdio.h>

char *
fgetln(FILE *stream, size_t *size)
{
int c, n;
static char buf[1024];

	n = 0;
	while ((c = fgetc(stream)) != '\n' && c != EOF && c != -1) {
		buf[n++] = c;
	}
	if (c == -1)
		return NULL;
	*size = n;
	return buf;
}
