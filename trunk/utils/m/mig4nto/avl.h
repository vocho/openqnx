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





#ifdef __cplusplus
extern "C" {
#endif

/*
**	AVL tree entry
*/
typedef struct avl_entry
{
	struct avl_entry	*llink;				/* -> Left hand sub-tree */
	struct avl_entry	*rlink;				/* -> Right hand sub-tree */
	int					 balance_factor;	/* Balance factor of node */
											/* Height of right sub-tree */
											/* less height of left subtree								*/
	void				*key;				/* Key value */
	void				*data;				/* Application data	*/

} AVL_ENTRY_STRUCT;


/*
**	This is the root of an AVL tree.  A dummy AVL tree entry
**	is hung off this block to serve as the root of the tree
*/
typedef struct
{
	AVL_ENTRY_STRUCT	*root;						/* Root AVL table entry	*/
	int					(*comp)( void *, void * );	/* Comparison function */
													/* for this AVL tree */
	unsigned			 entries;					/* Number of entries */

} AVL_ROOT_STRUCT;


/*
**	Boolean Defines
*/
#ifndef		TRUE
#	define	TRUE	1
#	define	FALSE	0
#endif


typedef int (*AVL_COMP_FN)(void *, void *);


/*
**	Function prototypes
*/
AVL_ROOT_STRUCT		*AllocAvlTree( int (*comp)( void *, void * ) );
void				 ForFullAvlTree( AVL_ROOT_STRUCT *root, void (*f)( AVL_ENTRY_STRUCT *s ) );
void				 FreeAvlTree( AVL_ROOT_STRUCT *s, int FreeData );
int					 InsertAvl( void *key, void *data, AVL_ROOT_STRUCT *root, AVL_ENTRY_STRUCT **node );
AVL_ENTRY_STRUCT	*LookupAvl( void *key, AVL_ROOT_STRUCT *root );

#ifdef __cplusplus
};
#endif

/* End of File */
