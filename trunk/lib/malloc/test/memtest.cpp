#include <malloc_g/malloc.h>
#include <malloc_g/malloc>	// C++ Version
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef DEBUG
typedef CheckedPtr<int> intp_t;
#else
typedef int * intp_t;
#endif

#ifdef DEBUG
#define malloc(__s)		debug_malloc(__FILE__, __LINE__, __s)
#define realloc(__p,__s)	debug_realloc(__FILE__, __LINE__, __p, __s)
#define calloc(__n,__s)		debug_calloc(__FILE__, __LINE__, __n, __s)
#define free(__p)		debug_free(__FILE__, __LINE__, __p)
#endif

void
duplicate_free( void )
{
	char *p = (char *)malloc( 10 );
	free( p );
	/* malloc_g and mmalloc catch it here */
	free( p );
}

void
overrun( void )
{
	intp_t foo, f;
	int i;
	mallopt(MALLOC_FILLAREA, 1);
	foo = (int *)malloc( 10*4 );
	/*
	 * malloc_g with smart pointers catches it here!
	 */
	mallopt(MALLOC_WARN, M_HANDLE_ABORT); /* set malloc warning handling	*/
	for ( f = foo, i = 12; i > 0; f++, i-- )
	*f = 89;
	mallopt(MALLOC_WARN, M_HANDLE_IGNORE);
	/*
	 * malloc_g and mmalloc catch it here
	 */
	free( foo );
}

void
free_uninitialized( void )
{
	char *p = (char *)40000;
	free( p );
}

void
free_then_use( void )
{
	char *p;
	mallopt(MALLOC_CKACCESS, 1);
	p = (char *)malloc( 30 );
	free( p );
	/*
	 * malloc_g catches it here with CKACCESS
	 */
	strcpy( p, "Hello There!" );
	/*
	 * mmalloc catches it here?
	 */
	p = (char *)malloc( 30 );
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
	char *p = (char *)malloc( 30 );
	strcpy( p, "This is a Test");
	p = (char *)strchr( p, 's' );
	/* malloc_g and mmalloc catch it here */
	free( p );
}

void
dump_garbage()
{
    malloc_mark(0);
    malloc_dump_unreferenced(STDERR_FILENO, 0);
}

void
just_leak( void )
{
	char *memarray[10];
	/* malloc_g will automatically catch the leaks at exit */
	atexit(dump_garbage);
	int i;

	/* mmalloc would have you dump the heap stats here in the debugger */
	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)strdup( "string" );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)strdup( "string" );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)malloc( 45 );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)calloc( 55, 1 );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)calloc( 55, 1 );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)realloc( memarray[i], 75 );
	/*
	 * mmalloc would have you dump the heap stats here in the debugger 
	 * and compare with the stats from above.
	 * Doesn't help much with multi-threaded execution.
	 */
}

void
malloc_free_cycle( void )
{
	char *memarray[10];
	int i;
	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)strdup( "string" );
	for ( i = 0; i < 10; i++ )
		free( memarray[i] );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)strdup( "string" );
	for ( i = 0; i < 10; i++ )
		free( memarray[i] );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)malloc( 45 );
	for ( i = 0; i < 10; i++ )
		free( memarray[i] );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)calloc( 55, 1 );
	for ( i = 0; i < 10; i++ )
		free( memarray[i] );

	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)calloc( 55, 1 );
	for ( i = 0; i < 10; i++ )
		memarray[i] = (char *)realloc( memarray[i], 75 );
	for ( i = 0; i < 10; i++ )
		free( memarray[i] );
}

void
underrun()
{
	char *p;
	mallopt(MALLOC_FILLAREA, 1);
	p = (char *)(void *)strdup( "hello" );
	p -= 4;
	*p = 88;
	p += 4;
	/*
	 * malloc_g and mmalloc catch it in the free()
	 * malloc_g may have problems detecting when you underran
	 * into the previous block without trashing your own header
	 */ 
	free( p );
}

void
node_corrupt()
{
	char *p, *c;
	p = (char *)malloc( 20 );
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

