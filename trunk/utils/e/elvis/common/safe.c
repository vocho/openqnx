/* safe.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_safe[] = "$Id: safe.c,v 2.17 2003/10/17 17:41:23 steve Exp $";
#endif

#ifndef DEBUG_ALLOC
void *safealloc(qty, size)
	int	qty;	/* number of items to allocate */
	size_t	size;	/* size of each item */
{
	void	*newp;

	newp = (void *)calloc((size_t)qty, size);
	if (!newp)
	{
		msg(MSG_FATAL, "no memory");
	}
	return newp;
}

void safefree(ptr)
	void	*ptr;	/* pointer to item(s) to be freed */
{
	free(ptr);
}

char *safedup(str)
	char	*str;	/* nul-terminated string to be duplicated */
{
	char *newp = (char *)safealloc((int)strlen(str) + 1, sizeof(char));
	strcpy(newp, str);
	return newp;
}
#else

#define MAGIC1 0x10d934a2
#define MAGIC2 0x42df3219
typedef struct sainfo_s
{
	struct sainfo_s *next;	/* another allocated memory chunk */
	char		*file;	/* source file where allocated */
	int		line;	/* source line where allocated */
	int		size;	/* number of longs allocated */
	ELVBOOL		kept;	/* if ElvTrue, don't complain if never freed */
	long		magic[2];/* magic number plus application info */
} sainfo_t;

sainfo_t *allocated;
	
/* inspect the list of allocated memory */
void safeinspect()
{
	sainfo_t	*scan;

	/* inspect list of allocated blocks for overflow/underflow */
	for (scan = allocated; scan; scan = scan->next)
	{
		if (scan->magic[0] != MAGIC1)
		{
			fprintf(stderr, "underflow in memory at 0x%lx, allocated from %s:%d, magic[0]=0x%lx\n",
				(long)scan, scan->file, scan->line, scan->magic[0]);
			abort();
		}
		else if (scan->magic[scan->size + 1] != MAGIC2)
		{
			fprintf(stderr, "overflow in memory at 0x%lx, allocated from %s:%d, magic[%d]=0x%lx\n",
				(long)scan, scan->file, scan->line, scan->size+1, scan->magic[scan->size + 1]);
			abort();
		}
	}
}


/* allocate memory, and remember where it was allocated */
void *_safealloc(file, line, kept, qty, size)
	char	*file;	/* name of source file where this func was called */
	int	line;	/* line of source file where this func was called */
	ELVBOOL	kept;	/* if ElvTrue, don't complain if never allocated */
	int	qty;	/* number of items to allocate */
	size_t	size;	/* size of each item */
{
	int	nlongs;
	sainfo_t *newp, *scan, *lag;

	/* inspect previous allocations */
	safeinspect();

	/* round user request up to whole number of longs */
	nlongs = (qty * size + sizeof(long) - 1) / sizeof(long);
	assert(nlongs * sizeof(long) >= qty * size);

	/* allocate storage space */
	newp = (sainfo_t *)calloc(1, sizeof(sainfo_t) + nlongs * sizeof(long));
	if (!newp)
	{
		msg(MSG_FATAL, "no memory");
	}

	/* save info about allocated memory */
	newp->file = file;
	newp->line = line;
	newp->size = nlongs;
	newp->kept = kept;
	newp->magic[0] = MAGIC1;
	newp->magic[1 + newp->size] = MAGIC2;
	for (scan = allocated, lag = NULL;
	     scan && (strcmp(file, scan->file) < 0 || (!strcmp(file, scan->file) && line < scan->line));
	     lag = scan, scan = scan->next)
	{
	}
	newp->next = scan;
	if (lag)
		lag->next = newp;
	else
		allocated = newp;

	/* count allocations from that spot */
	for (nlongs = 0, scan = newp;
	     scan && !strcmp(file, scan->file) && line == scan->line;
	     nlongs++, scan = scan->next)
	{
	}
	if (nlongs > 100 && (!kept || strcmp(file, "options.c")))
	{
		fprintf(stderr, "%d allocations from %s(%d)\n", nlongs, file, line);
	}

	/* return the application portion of allocated memory */
	return (void *)&newp->magic[1];
}


/* free allocated memory */
void _safefree(file, line, mem)
	char	*file;	/* name of source file where this func was called */
	int	line;	/* line of source file where this func was called */
	void	*mem;	/* item(s) to be freed */
{
	sainfo_t *scan, *lag;

	/* inspect previous allocations */
	safeinspect();

	/* locate the memory in the allocated list */
	for (lag = NULL, scan = allocated;
	     scan && (void *)&scan->magic[1] != mem;
	     lag = scan, scan = scan->next)
	{
	}

	/* if not in allocation list, fail */
	if (!scan)
	{
		fprintf(stderr, "attempt to free unallocated memory from %s:%ld\n",
			file, (long)line);
		return;
	}

	/* delete the memory from the allocated list */
	if (lag)
	{
		lag->next = scan->next;
	}
	else
	{
		allocated = scan->next;
	}

	/* free the memory */
	free(scan);
}

/* allocate a duplicate of a string, using _safealloc() */
char *_safedup(file, line, kept, str)
	char	*file;	/* name of source file where this func was called */
	int	line;	/* line of source file where this func was called */
	ELVBOOL	kept;	/* if ElvTrue, don't complain if never freed */
	char	*str;	/* nul-terminated string to duplicate */
{
	char	*newp;

	newp = (char *)_safealloc(file, line, kept, (int)(strlen(str) + 1), sizeof(char));
	strcpy(newp, str);
	return newp;
}

/* list any unfreed memory */
void safeterm()
{
	sainfo_t *scan;

	for (scan = allocated; scan; scan = scan->next)
	{
		if (!scan->kept)
		{
			fprintf(stderr, "memory allocated from %s:%ld never freed\n",
				scan->file, (long)scan->line);
		}
	}
}
#endif
