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





#include <malloc_g/malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void
duplicate_free( void )
{
	char *p = malloc( 10 );
	free( p );
	free( p );
}

void
overrun( void )
{
	int *foo, *f, i;
#ifdef MALLOC_DEBUG
	mallopt(MALLOC_FILLAREA, 1);
#endif
	foo = malloc( 10*4 );
	for ( f = foo, i = 12; i > 0; f++, i-- )
	*f = 89;
	free( foo );
}

void
free_uninitialized( void )
{
	char *p = (void *)40000;
	free( p );
}

void
free_then_use( void )
{
	char *p;
#ifdef MALLOC_DEBUG
	mallopt(MALLOC_CKACCESS, 1);
#endif
	p = malloc( 30 );
	free( p );
	strcpy( p, "Hello There!" );
	p = malloc( 30 );
	free( p );
}

void
free_array( void )
{
	char p[10], *c = (char *)&p;
	free( c );
}

void
free_midrange( void )
{
	char *p = malloc( 30 );
	strcpy( p, "This is a Test");
	p = (void *)strchr( p, 's' );
	free( p );
}

void
dump_garbage()
{
#ifdef MALLOC_DEBUG
    malloc_mark(0);
    malloc_dump_unreferenced(STDERR_FILENO, 0);
#endif
}

void
just_leak( void )
{
	char *memarray[10];
	int i;
	atexit(dump_garbage);
	for ( i = 0; i < 10; i++ )
		memarray[i] = (void *)strdup( "string" );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (void *)strdup( "string" );

	for ( i = 0; i < 10; i++ )
		memarray[i] = malloc( 45 );

	for ( i = 0; i < 10; i++ )
		memarray[i] = calloc( 55, 1 );

	for ( i = 0; i < 10; i++ )
		memarray[i] = calloc( 55, 1 );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (void *)realloc( memarray[i], 75 );
}

void
malloc_free_cycle( void )
{
	char *memarray[10];
	int i;
	for ( i = 0; i < 10; i++ )
		memarray[i] = (void *)strdup( "string" );
	for ( i = 0; i < 10; i++ )
		free( memarray[i] );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (void *)strdup( "string" );
	for ( i = 0; i < 10; i++ )
		free( memarray[i] );

	for ( i = 0; i < 10; i++ )
		memarray[i] = malloc( 45 );
	for ( i = 0; i < 10; i++ )
		free( memarray[i] );

	for ( i = 0; i < 10; i++ )
		memarray[i] = calloc( 55, 1 );
	for ( i = 0; i < 10; i++ )
		free( memarray[i] );

	for ( i = 0; i < 10; i++ )
		memarray[i] = calloc( 55, 1 );
	for ( i = 0; i < 10; i++ )
		memarray[i] = (void *)realloc( memarray[i], 75 );
	for ( i = 0; i < 10; i++ )
		free( memarray[i] );
}

void
underrun()
{
	char *p;
#ifdef MALLOC_DEBUG
	mallopt(MALLOC_FILLAREA, 1);
#endif
	p = (char *)(void *)strdup( "hello" );
	p -= 4;
	*p = 88;
	p += 4;
	free( p );
}

void
node_corrupt()
{
	char *p, *c;
	p = malloc( 20 );
	//c = (char *)(void *)mm_find_node( 0 );
	c = (char *)((Dhead *)p - 1);
	strcpy( c, "hello" );
	free( p );
}

void
dohelp( void )
{
	printf( "\nUsage:\n\t1\tduplicate_free\n\t2\toverrun\n\t3\tfree_uninitialized\n\t4\tfree_then_use\n\t5\tfree_array\n\t6\tfree_midrange\n");
	printf( "\t7\tunderrun\n\t8\tnode corruption\n\t9\tjust_leak\n\t10\tmalloc/free cycling\n\n");
}
	
int
main( int argc, char **argv )
{
	int num;

	if( argc < 2 ) {
		dohelp();
		exit( 0 );
	}
	switch( atoi( argv[1] ) ) {
		case 1:
			duplicate_free();
			break;
		case 2:
			overrun();
			break;
		case 3:
			free_uninitialized();
			break;
		case 4:
			free_then_use();
			break;
		case 5:
			free_array();
			break;
		case 6:
			free_midrange();
			break;
		case 7:
			underrun();
			break;
		case 8:
			node_corrupt();
			break;
		case 9:
			just_leak();
			break;
		case 10:
			malloc_free_cycle();
			break;
		default:;
	}
	return 0;
}

