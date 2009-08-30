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





/***********
	*
	*	A V L   T r e e   M a n a g e r
	*
	*
	*	This module provides basic AVL tree management functions.
	*
	*		AllocAvl allocates an AVL entry (external)
	*
	*		AllocAvlTree allocates an AVL tree (external)
	*
	*		FindAvlEntry finds an entry (local)
	*
	*		ForAllAvl walks the AVL tree, executes function (local)
	*
	*		ForFullAvlTree walks the AVL tree, executes function (external)
	*
	*		FreeAvlEntry free an entry (local)
	*
	*		FreeAvlTree frees the entire tree (external)
	*
	*		InsertAvl inserts a new entry into the AVL tree (external)
	*
	*		LookupAvlEntry locates the entry (external)
	*
	*
	*	These functions use the AVL Balanced Tree Algorithm described in
	*	Knuth:	The Art of Computer Programming Vol. 3 - Sorting and Selection
	*			pp. 451 - 458
	*
***********/
#include	<stdlib.h>

#include	"avl.h"


#define	INSERT_LEFT		-1		/* AVL entry to be inserted on LHS of tree	*/
#define	INSERT_RIGHT	 1		/* AVL entry to be inserted on RHS of tree	*/
#define	AVL_ENTRY_FOUND	 0		/* AVL entry was found						*/


/*
**	Local Function Prototypes
*/
static AVL_ENTRY_STRUCT		*AllocAvl( void *key, void *data );
static int					 FindAvlEntry( void *k ); 
static void					 FreeAvlEntry( AVL_ENTRY_STRUCT *k );
static void					 ForAllAvl( AVL_ENTRY_STRUCT *s );


/*
**	Node pointers
*/
static AVL_ROOT_STRUCT	*avl_root;

static AVL_ENTRY_STRUCT *sm_sa;					/* -> Rebalancing point		*/
static AVL_ENTRY_STRUCT *sm_ta;					/* -> Father of S			*/
static AVL_ENTRY_STRUCT *sm_pa; 				/* -> Current entry			*/
static AVL_ENTRY_STRUCT *sm_qa;					/* -> New node to inspect	*/
static AVL_ENTRY_STRUCT *sm_ra; 				/* -> Rebalancing node		*/


/*
**	Local data
**
**	Function to call while walking the tree with ForAllAvl (avoids
**	recursively passing the function on the stack).  Same for
**	free data
*/
static void 			(*ForAllAvlFunction)( AVL_ENTRY_STRUCT *s );
static int				  ForAllAvlFreeData;


/***********
	*
	*	A l l o c A v l
	*
	*	Allocate a AVL tree entry
	*
***********/
static AVL_ENTRY_STRUCT		*AllocAvl( void *Key, void *Data )
{
	AVL_ENTRY_STRUCT *s;

	s		= calloc( 1, sizeof *s );
	s->key	= Key;
	s->data	= Data;

	return s;
}


/***********
	*
	*	A l l o c A v l T r e e
	*
	*	Allocate the root for an AVL tree
	*
***********/
AVL_ROOT_STRUCT				*AllocAvlTree( int (*CompFn)( void *, void * ) )
{
	AVL_ROOT_STRUCT		*Root;

	Root		= calloc( 1, sizeof *Root );
	Root->comp	= CompFn;
	Root->root	= AllocAvl( NULL, NULL );		/* Set root entry */

	return Root;
}


/***********
	*
	*	F i n d A v l E n t r y
	*
	*	Find an entry in an AVL tree
	*
	*	See Knuth for details of the operation of the algorithm
	*
***********/
static int					FindAvlEntry( void *k )
{
	AVL_ENTRY_STRUCT	*t;
	AVL_ENTRY_STRUCT	*p;
	AVL_ENTRY_STRUCT	*q;
	int					 i;

	/*
	**	Setup search pointers
	*/
	t = sm_ta;
	sm_pa = sm_sa = t->rlink;

	/*
	**	Loop looking for requested entry (or a slot to put it)
	*/
	if ( t->rlink == NULL )
		return INSERT_RIGHT;

	for (; ;)
	{
		p = sm_pa;
		i = avl_root->comp( k, p->key );
		if ( i == 0 )
			return AVL_ENTRY_FOUND;

		sm_qa = (i < 0) ? p->llink : p->rlink;

		if ( sm_qa == NULL )
			return (i < 0) ? INSERT_LEFT : INSERT_RIGHT;

		q = sm_qa;
		if ( q->balance_factor )
			{
			sm_ta = sm_pa;			/* Rebalancing will be needed here	*/
			sm_sa = sm_qa; 			/* Set new T and S					*/
			}

		sm_pa = sm_qa;
	}
}


/***********
	*
	*	F o r A l l A v l
	*
	*	Internal recursive function to walk AVL tree
	*
***********/
static void					ForAllAvl( AVL_ENTRY_STRUCT *s )
{
	AVL_ENTRY_STRUCT	*q;


	if ( s == NULL )
		return;

	ForAllAvl( s->llink );

	q = s->rlink;					/* Save here in event function deletes */

	ForAllAvlFunction( s );			/* Invoke function */

	ForAllAvl( q );
}


/***********
	*
	*	F o r F u l l A v l T r e e
	*
	*	Walk the AVL tree executing a specified function for each entry
	*
***********/
void						ForFullAvlTree( AVL_ROOT_STRUCT *Root,
									void (*Function)( AVL_ENTRY_STRUCT *s ) )
{
	ForAllAvlFunction = Function;

	ForAllAvl( Root->root->rlink );

}


/***********
	*
	*	F r e e A v l E n t r y
	*
	*	Internal function to free an AVL entry
	*
	*
***********/
static void					FreeAvlEntry( AVL_ENTRY_STRUCT *s )
{
	if ( ForAllAvlFreeData )
		{
		if ( s->key )
			free( s->key );

		if ( s->data )
			free( s->data );
		}

	free( s );
}


/***********
	*
	*	F r e e A v l T r e e
	*
	*	Free an AVL tree
	*
***********/
void						FreeAvlTree( AVL_ROOT_STRUCT *Root, int FreeData )
{
	ForAllAvlFreeData = FreeData;
	ForAllAvlFunction = FreeAvlEntry;

	ForAllAvl( Root->root->rlink );

	free( Root );
}


/***********
	*
	*	I n s e r t A v L
	*
	*	Insert an entry into an AVL tree
	*
	*	See Knuth for a full description of this algorithm
	*
***********/
int						InsertAvl( void *key, void *data, AVL_ROOT_STRUCT *root, AVL_ENTRY_STRUCT **node )
{
	AVL_ENTRY_STRUCT	*q;
	AVL_ENTRY_STRUCT	*t;
	AVL_ENTRY_STRUCT	*p;
	AVL_ENTRY_STRUCT	*r;
	AVL_ENTRY_STRUCT	*s;
	AVL_ENTRY_STRUCT	*pl, *pr;
	int					 i, a, pbf;


	/*
	 *	Insert node in empty tree
	 */
	avl_root	= root;
	sm_ta		= root->root;			/* Set start of search */
	t 			= sm_ta;
	if ( t->rlink == NULL )
	{	*node = t->rlink = AllocAvl( key, data );
		++root->entries;
		return TRUE;					/* Set first node (balance 0) */
	}

	/*
	 *	Find point to insert this entry - Steps A1 - A4
	 */
	if ( (i = FindAvlEntry( key )) == AVL_ENTRY_FOUND )
	{	*node = sm_pa;
		return FALSE;
	}
	else
		*node = q = AllocAvl( key, data );

	/*
	 *	Insert node in tree - Step A5
	 */
	p = sm_pa;
	if ( i == INSERT_LEFT )
		p->llink = q;
	else
		p->rlink = q;

	/*
	 *	Loop adjusting balance factors - Step A6
	 */
	s = sm_sa;
	a = avl_root->comp( key, s->key );
	if ( a < 0 )						/* Which side to start? */
		sm_ra = s->llink;				/* Look left */
	else
		sm_ra = s->rlink; 				/* Looking right */

	sm_pa = sm_ra;
	if ( sm_pa != NULL )
		for ( ; ; )
			{
			p = sm_pa;
			i = avl_root->comp( key, p->key );

			if ( i == 0 )
				break;					/* Balance adjustment complete */

			if ( i < 0 )
				{
				--(p->balance_factor);	/* Left */
				sm_pa = p->llink;
				}
			else
				{
				++(p->balance_factor);	/* Right */
				sm_pa = p->rlink;
				}
			}

	/*
	 *	Determine type of balancing required  - Step A7
	 */
	a = (a < 0) ? -1 : 1;
	s = sm_sa;
	if ( s->balance_factor == 0 )
		s->balance_factor = a;			/* Tree has grown in height */
	else if ( s->balance_factor == -a )
		s->balance_factor = 0;			/* Tree has grown more balanced */
	else
		{								/* Tree is out of balance - Step A8 */
		r = sm_ra;
		if ( r->balance_factor == a )	/* Do single rotation */
		{
			sm_pa = sm_ra;
			if ( a < 0 )
				{
				sm_qa = r->rlink;
				r->rlink = sm_sa;
				}
			else
				{
				sm_qa = r->llink;
				r->llink = sm_sa;
				}

			r->balance_factor = 0;
			s = sm_sa;
			s->balance_factor = 0;

			if ( a < 0 )
				s->llink = sm_qa;
			else
				s->rlink = sm_qa;
		}							/* End of Step 8							*/
		else						/* Do double rotation - Step A9				*/
			{
			if ( a < 0 )
				{
				sm_pa = r->rlink;
				p = sm_pa;
				pl = p->llink;
				pr = p->rlink;
				p->llink = sm_ra;
				}
			else
				{
				sm_pa = r->llink;
				p = sm_pa;
				pl = p->llink;
				pr = p->rlink;
				p->rlink = sm_ra;
				}

			pbf = p->balance_factor;

			r = sm_ra;
			if ( a < 0 )
				r->rlink = pl;
			else
				r->llink = pr;

			p = sm_pa;
			s = sm_sa;
			if ( a < 0 )
				s->llink = pr;
			else
				s->rlink = pl;

			s->balance_factor = (pbf == a) ? -a : 0;
			r = sm_ra;
			r->balance_factor = (pbf == -a)  ? a : 0;
			p = sm_pa;

			if ( a < 0 )
				p->rlink = sm_sa;
			else
				p->llink = sm_sa;

			p->balance_factor = 0;
			}							/* End of Step 9							*/
	
		/*
		 *	Set new root for sub-tree - Step 10
		 */
		t = sm_ta;

		if ( sm_sa == t->rlink )
			t->rlink = sm_pa;
		else
			t->llink = sm_pa;
		}

	sm_ta = root->root;
	++root->entries;


	return TRUE;
}


/***********
	*
	*	L o o k u p A v l E n t r y
	*
	*	External AVl entry lookup function
	*
***********/
AVL_ENTRY_STRUCT			*LookupAvl( void *key, AVL_ROOT_STRUCT *root )
{
	int			rc;

	avl_root	= root;
	sm_ta		= root->root;				/* Set start of search */
	rc			= FindAvlEntry( key );

	return (rc == AVL_ENTRY_FOUND) ? sm_pa : NULL;
}


/* End of File */
