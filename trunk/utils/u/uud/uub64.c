/*-
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* based on NetBSD uudecode */

#include <uub64.h>
#include <err.h>
#include <resolv.h>
#include <string.h>

int
base64_decode(FILE *in, FILE *out, char *ifname)
{
	int n;
	char inbuf[MAXPATHLEN];
	unsigned char outbuf[MAXPATHLEN * 4];

	for (;;) {
		if (!fgets(inbuf, sizeof(inbuf), in)) {
			warnx("%s: short file.", ifname);
			return (1);
		}
		n = b64_pton(inbuf, outbuf, sizeof(outbuf));
		if (n < 0)
			break;
		fwrite(outbuf, 1, n, out);
	}
	return (checkend(inbuf, "====",
			"error decoding base64 input stream"));
}

int
checkend(const char *ptr, const char *end, const char *msg)
{
	size_t n;

	n = strlen(end);
	/*
	 * Note '\t' and '\r' below aren't
	 * strictly posix for uudecode.
	 * posix says '\n' only with no 
	 * trailing or leading space.
	 */
	if (strncmp(ptr, end, n) != 0 ||
	    strspn(ptr + n, "\n\t\r") != strlen(ptr + n)) {
		if (msg != NULL)
			warnx("%s", msg); 
		return (1);
	}
	return (0);
}
