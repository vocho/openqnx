/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */



#include <string.h>
#include <ctype.h>
#ifndef __MINGW32__
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif
#include "sendnto.h"

#ifdef __MINGW32__
static void herror (const char *s)
{
  printf ("Error: %s\n", s);
}
#endif

static void
initinet(void) {
}


static int
openinet(const char *name, int baud) {
	struct	servent		*service;
	struct	hostent		*h;
	char				*serv_name;
	struct sockaddr_in	addr;
	int					tos;
	int					fd;
	short				port;

	serv_name = strchr(name, ':');
	if(serv_name != NULL) {
		*serv_name = '\0';
		++serv_name;
	} else {
		serv_name = "telnet";
	}
	
	if(isdigit(*serv_name)) {
		port = htons(strtoul(serv_name, NULL, 0));
	} else {
		service = getservbyname(serv_name, "tcp");
		if(service == NULL) {
			fprintf(stderr, "unknown service name '%s'\n", serv_name);
			exit(1);
		}
		port = service->s_port;
	}
	
	h = gethostbyname(name);
	if(h == NULL) {
		herror("sendnto");
		exit(1);
	}
	memset(&addr, 0, sizeof(addr));
	memcpy(&addr.sin_addr, h->h_addr, h->h_length);
#if defined(__QNXNTO__)	
	addr.sin_len = h->h_length;
#endif	
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1) {
		return -1;
	}
	if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		return -1;
	}
	tos = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &tos, sizeof(int));

	return fd;
}


static int
writeinet(int fd, const void *buf, int n) {
	int	got;
	int	len = n;

	for( ;; ) {
		got = send(fd, buf, n, 0);
		if(got == -1) return -1;
		n -= got;
		if(n == 0) return len;
		buf = (char *)buf + got;
	}
}


static int
getinet(int fd, int timeout) {
	unsigned char	ch;
	fd_set			rd;
	struct timeval	to;
	int				r;

	FD_ZERO(&rd);
	FD_SET(fd, &rd);

	if(timeout == -1) timeout = 0;
	to.tv_usec = timeout * 1000;
	to.tv_sec = 0;
	r = select(fd+1, &rd, NULL, NULL, &to);
	if(r != 1) return -1;
	r = recv(fd, &ch, sizeof(ch), 0);
	if(r != 1) return -1;
	return ch;
}


static void
flushinet(int fd) {
	while(getinet(fd, -1) != -1) {
		// nothing to do
	}
}


static void
closeinet(int fd) {
	close(fd);
}


void
outputinet(void) {
	output.check_rate = 8;
	output.init = initinet;
	output.open = openinet;
	output.flush = flushinet;
	output.write = writeinet;
	output.get = getinet;
	output.close = closeinet;
}

static void
initpipe(void) {
}


static int
openpipe(const char *name, int baud) {
	return 0;
}


static int
writepipe(int fd, const void *buf, int n) {
	return writeinet(1, buf, n);
}


static int
getpipe(int fd, int timeout) {
	return getinet(0, timeout);
}


static void
flushpipe(int fd) {
	while(getinet(0, -1) != -1) {
		// nothing to do
	}
}


static void
closepipe(int fd) {
}


void
outputpipe(void) {
	output.check_rate = 8;
	output.init = initpipe;
	output.open = openpipe;
	output.flush = flushpipe;
	output.write = writepipe;
	output.get = getpipe;
	output.close = closepipe;
}
