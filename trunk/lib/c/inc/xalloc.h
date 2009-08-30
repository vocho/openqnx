/* xalloc.h internal header */
#ifndef _XALLOC
#define _XALLOC
#include <stddef.h>
#include <stdlib.h>
#ifndef _YVALS
 #include <yvals.h>
#endif
_C_STD_BEGIN

		/* macros */
#define M_MASK	((1 << _MEMBND) - 1)	/* rounds all sizes */
#define CELL_OFF	(offsetof(_Cell, _Next) + M_MASK & ~M_MASK)
#define SIZE_BLOCK	512	/* minimum block size, power of 2 */
#define SIZE_CELL	(sizeof (_Cell) + M_MASK & ~M_MASK)

		/* type definitions */
typedef struct _Cell
	{	/* heap item */
	size_t _Size;	/* CELL_OFF <= SIZE_CELL <= _Size */
	struct _Cell *_Next;	/* reused if CELL_OFF < SIZE_CELL */
	} _Cell;

typedef struct
	{	/* heap control information */
	_Cell **_Plast;	/* null, or where to resume malloc scan */
	_Cell *_Head;	/* null, or lowest addressed free cell */
	} _Altab;

		/* declarations */
_C_LIB_DECL
void *_Getmem(size_t);
extern _Altab _Aldata;	/* free list initially empty */
_END_C_LIB_DECL

 #if _INTELx86
/* #define _PTR_NORM(p) (void __huge *)(p) should have worked */
  #define _PTR_NORM(p) \
	( (((unsigned long)(p) & 0xFFFF0000L)>>12) \
	+ ((unsigned long)(p) & 0xFFFFL) )
 #else
  #define _PTR_NORM(p) (p)
 #endif

#ifdef DEBUG
 #include <assert.h>
 #define ASSERT(e) assert(e)
_C_LIB_DECL
int _OK_Cell(_Cell *p);
int _OK_Altab(_Altab *p);
void _UPD_Altab(size_t d_heap, size_t d_alloc, size_t d_free);
_END_C_LIB_DECL
#else /* is NDEBUG */
 #define ASSERT(e) (void)0
 #define _OK_Cell(p) (void)0
 #define _OK_Altab(p) (void)0
 #define _UPD_Altab(d_heap, d_alloc, d_free) (void)0
#endif /*DEBUG*/
_C_STD_END
#endif /* _XALLOC */

/*
 * Copyright (c) 1994-2000 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
V3.05:1296 */

/* __SRCVERSION("xalloc.h $Rev: 153052 $"); */
