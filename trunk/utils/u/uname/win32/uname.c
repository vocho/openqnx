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





#ifdef __USAGE		/* uname.c */
%C - return system name (POSIX)

%C	[-amnprsv]  [-S name]
Options:
 -a       All options (as though all the options (-rsv) were specified).
 -m       Machine name (write to standard output).
 -n       Node name (write to standard output).
 -p       Processor Name.
 -r       Release level (write to standard output).
 -s       System (OS) name (write to standard output).
 -v       Version (write to standard output).
 -S name  Set host name
Note:
 If no options are specified, the uname utility writes the System name,
 as if the -s option had been specified.
#endif
	
/*
 * Include declarations:
 */
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Define declarations:
 */
#define TRUE	1
#define FALSE	0

#ifdef __MINGW32__
#define ENOTSUP 48
#endif

#ifndef PROCESSOR_AMD_X8664
#define PROCESSOR_AMD_X8664 8664
#endif

#ifndef PROCESSOR_ARCHITECTURE_AMD64
#define PROCESSOR_ARCHITECTURE_AMD64 9
#endif

#define MACHINE 1
#define NODE	2
#define RELEASE 4
#define OSFLAG  8
#define VERSION 16
#define PROCESSOR 32
#define ALL (MACHINE|NODE|RELEASE|OSFLAG|VERSION|PROCESSOR)

#define  ENV_VAR_STRING_COUNT  (sizeof(envVarStrings)/sizeof(TCHAR*))
#define INFO_BUFFER_SIZE 32767

BOOL DisplaySystemVersion(int flags)
{
	OSVERSIONINFO osvi;
	SYSTEM_INFO si;
	
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx ((OSVERSIONINFO *) &osvi);
	GetSystemInfo(&si);

	switch (osvi.dwPlatformId)
	{
		case VER_PLATFORM_WIN32_NT:

		// Test for the product.
			if(flags & OSFLAG){
				HKEY hKey;
				char szProductType[80];
				DWORD dwBufLen;

				if ( osvi.dwMajorVersion <= 4 )
					printf("Microsoft Windows NT ");

				if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
					printf ("Microsoft Windows 2000 ");
	
				if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
					printf ("Microsoft Windows XP ");

				if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
					printf ("Microsoft Windows Server 2003 ");
				
				if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
					printf ("Microsoft Windows Vista ");

		// Test for product type.
				RegOpenKeyEx( HKEY_LOCAL_MACHINE,
					"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
					0, KEY_QUERY_VALUE, &hKey );
				RegQueryValueEx( hKey, "ProductType", NULL, NULL,
					(LPBYTE) szProductType, &dwBufLen);
				RegCloseKey( hKey );
				if ( lstrcmpi( "WINNT", szProductType) == 0 )
					printf( "Workstation " );
				if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
					printf( "Server " );

			}

		// Display version, service pack (if any), and build number.

			if(flags & RELEASE){
				printf ("version %d.%d %s (Build %d) ",
					osvi.dwMajorVersion,
					osvi.dwMinorVersion,
					osvi.szCSDVersion,
					osvi.dwBuildNumber & 0xFFFF);
			}
			break;
		case VER_PLATFORM_WIN32_WINDOWS:

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
			{
				 printf ("Microsoft Windows 95 ");
				 if ( osvi.szCSDVersion[1] == 'C' )
					 printf("OSR2 " );
			} 

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
			{
				 printf ("Microsoft Windows 98 ");
				 if ( osvi.szCSDVersion[1] == 'A' )
					 printf("SE " );
			} 

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
			{
				 printf ("Microsoft Windows Me ");
			} 
			break;

		case VER_PLATFORM_WIN32s:

			printf ("Microsoft Win32s ");
			break;
	}

	// Display node name
	if(flags & NODE) 
	{
		TCHAR  infoBuf[INFO_BUFFER_SIZE];
		DWORD  bufCharCount = INFO_BUFFER_SIZE;
 
		if(GetComputerName( infoBuf, &bufCharCount)) 
  			printf("%s ", infoBuf);
	}

	// Display version
	if(flags & VERSION) 
	{
		printf("unknown ");
	}

	// Display name of CPU
	if(flags & MACHINE) 
	{
		switch (si.dwProcessorType)
		{
			case PROCESSOR_INTEL_386:
				printf("Intel 386 ");
				break;
			case PROCESSOR_INTEL_486:
				printf("Intel 486 ");
				break;
			case PROCESSOR_INTEL_PENTIUM:
				printf("Intel Pentium ");
				break;
			case PROCESSOR_INTEL_IA64:
				printf("Intel IA64 ");
				break;
			case PROCESSOR_AMD_X8664:
				printf("x64 (AMD64/EM64T) ");
				break;
			default:
				printf("unknown machine name ");
				break;
		}
	}

	// Display processor type
	if(flags & PROCESSOR)
	{
		switch (si.wProcessorArchitecture)
		{
			case PROCESSOR_ARCHITECTURE_AMD64:
				printf("x64 ");
				break;
			case PROCESSOR_ARCHITECTURE_IA64:
				printf("Intel IPF ");
				break;
			case PROCESSOR_ARCHITECTURE_INTEL:
				printf("x86 ");
				break;
			default:
				printf("unknown processor ");
				break;
		}
	}

	printf("\n");
	return TRUE; 
}

void setname(char *name) 
{
	HKEY hKey;

	if(!SetComputerName(name)) {
		fprintf(stderr, "Error setting computer name\n");
	}

	// This registry entry is only valid for NT/XP/Vista
	RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
			0, KEY_SET_VALUE, &hKey );
	
	if(RegSetValueEx(hKey, "Hostname", 0, REG_SZ, name, strlen(name)+1))
	{
		fprintf(stderr, "Error setting hostname\n");
	}
	if(RegSetValueEx(hKey, "NV Hostname", 0, REG_SZ, name, strlen(name)+1))
	{
		fprintf(stderr, "Error setting NV hostname\n");
	}
}

/*	Uname writes the current system name to standard output.  When options
 *	are specified, symbols representing one or more system characteristics are 
 *	written to the stdandard output.  
 */
int
main( int argc, char *argv[] )
{
	/* static struct utsname name; */
	int  result, i, error;
	int	set = 0, flags = 0;
	extern char *optarg;

	result=error= FALSE;

	while(( i= getopt( argc, argv, "apmnrsvS:")) != -1)
		switch ( i ){
			case 'a':	flags |= ALL;
						break;
			case 'm':	flags |= MACHINE;   /* hardware machine name */
						break;
			case 'n':	flags |= NODE;	    /* system node name */
						break;
			case 'r':	flags |= RELEASE;   /* OS release level */
						break;
			case 'p':	flags |= PROCESSOR; /* Processor type */
						break;
			case 's':	flags |= OSFLAG;    /* OS name - also default */
						break;            
			case 'v':	flags |= VERSION;   /* OS version */
						break;
			case 'S':	setname(optarg);
						set = 1;
						break;	
			case '?':	error = TRUE;       /* error */
						break;
			}

	if (flags == 0 && !set) 
		flags |= OSFLAG;

	if ( error )
		exit(EXIT_FAILURE);

	DisplaySystemVersion(flags);
	return(EXIT_SUCCESS);
}
