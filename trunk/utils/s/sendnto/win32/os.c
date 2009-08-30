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





#include <windows.h>
#include "sendnto.h"

int
opendevice(const char *devicename, int baud) {
	HANDLE	fd;
	DCB		dcb;

	fd = CreateFile( (LPCTSTR) devicename, GENERIC_READ | GENERIC_WRITE,
			0, NULL, OPEN_EXISTING, 0, NULL );  /* synchronous */

	if(fd == INVALID_HANDLE_VALUE) return( -1 );

	GetCommState( fd, &dcb );
	if(baud != -1)
		dcb.BaudRate = baud;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDsrSensitivity = FALSE;
	dcb.XonLim = 512;
	dcb.XoffLim = 128;
	dcb.fBinary = TRUE;
	dcb.fNull = FALSE;
	dcb.fAbortOnError = FALSE;
	SetCommState( fd, &dcb );

    return( (int)fd );
}

void
flushdevice( int devicefd ) {
    FlushFileBuffers( (HANDLE)devicefd );
}

int
writedevice( int devicefd, const void *buff, int len ) {
	DWORD	nn;

	if( !WriteFile( (HANDLE) devicefd, buff, len, &nn, NULL ) ) return( -1 );
	return( nn );
}

int
getdevice(int fd, int timeout) {
	COMMTIMEOUTS	time;
	char			ch;
	DWORD			nn;

    GetCommTimeouts( (HANDLE)fd, &time );
	if( timeout == -1 ) {
		time.ReadIntervalTimeout = MAXDWORD;
		time.ReadTotalTimeoutMultiplier = 0;
		time.ReadTotalTimeoutConstant = 0;
	} else {
		time.ReadIntervalTimeout = 0;
	    time.ReadTotalTimeoutMultiplier = 0;
	    time.ReadTotalTimeoutConstant = timeout * 10;
	}
    SetCommTimeouts( (HANDLE)fd, &time );

	if( !ReadFile( (HANDLE) fd, &ch, sizeof( ch ), &nn, NULL ) ) return( -1 );
	if( nn != sizeof( ch ) ) return( -1 );
	return( ch );
}

int
checklapdevice( int devicefd ) {
    DWORD	mask;

	if( !GetCommMask( (HANDLE)devicefd, &mask ) ) return( -1 );
	if( mask & EV_PERR ) return( 1 );
	if( mask & EV_ERR ) return( 2 );
	return( 0 );
}
