#include <limits.h>
#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>

static struct utsname u;

char *nto_conf_path( char *path, char *nodename, int executable )
{
static char buf[_POSIX_PATH_MAX];

	if ( nodename == NULL && u.nodename[0] == '\0' ) {
		uname( &u );
	}
	if ( nodename == NULL )
		nodename = u.nodename;

	sprintf( buf, "%s.%s", path, nodename );
	if ( access( buf, executable ? X_OK:F_OK ) != -1 )
		return buf;
	return path;
}
