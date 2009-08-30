/* http.c */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_http[] = "$Id: http.c,v 2.11 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef PROTOCOL_HTTP

static sockbuf_t *sb;

/* Open an HTTP resource for reading.  Returns ElvTrue if successful, or False
 * for error (after giving an error message).
 */
ELVBOOL httpopen(site_port, resource)
	char	*site_port;	/* host name and optional port number */
	char	*resource;	/* name of file at remote host */
{
	char	*line, *p;

	/* Open a connection to the server */
	sb = netconnect(site_port, 80);
	if (!sb)
	{
		/* error message already given */
		return ElvFalse;
	}

	/* Send the "GET" request to the HTTP server.  Note that netputline()
	 * will add a second CRLF pair after the "HTTP/1.0" string.  This is
	 * important, because when an HTTP server sees "HTTP/1.0" it expects
	 * supplemental information to appear on following lines, up to the
	 * next blank line.  It won't process the request until it sees that
	 * blank line.
	 */
	msg(MSG_STATUS, "[s]requesting $1", resource);
	if (!netputline(sb, "GET", resource, "HTTP/1.0")
	 || !netputline(sb, "Host:", site_port, NULL)
	 || !netputline(sb, "", NULL, NULL))
	{
		/* error message already given */
		return ElvFalse;
	}

	/* Success, so far.  Now ready to begin reading data. */
	msg(MSG_STATUS, "receiving...");

	/* Fill the receive buffer. */
	if (!netread(sb))
		return ElvFalse;

	/* If the response starts with "HTTP", then assume it is in MIME
	 * format.  Skip the header, and watch for a "Content-Length:" line.
	 */
	if (netbytes(sb) >= 4 && !strncmp(netbuffer(sb), "HTTP", 4))
	{
		while ((line = netgetline(sb)) != NULL && *line)
		{
			/* Convert the header label to lowercase */
			for (p = line; *p && *p != ':'; p++)
			{
				*p = elvtolower(*p);
			}

			/* If "Content-Length:" then allow url.c to display
			 * the size.
			 */
			if (!strncmp(line, "content-length: ", 16))
			{
				urlbytes(atol(&line[16]));
			}
		}
	}

	return ElvTrue;
}


/* Read some data from the remote server */
int httpread(buf, nbytes)
	CHAR	*buf;
	int	nbytes;
{
	/* try to fill sb's buffer */
	if (sb->left > 0 && !netread(sb))
		return -1;

	/* never read more data than there is in sb's buffer */
	if (netbytes(sb) < nbytes)
		nbytes = netbytes(sb);

	/* copy bytes from sb's buffer to buf */
	if (nbytes > 0)
	{
		memcpy(buf, netbuffer(sb), nbytes);
		netconsume(sb, nbytes);
	}
	return nbytes;
}

/* close the socket */
void httpclose()
{
	netdisconnect(sb);
}


#endif /* PROTOCOL_HTTP */
