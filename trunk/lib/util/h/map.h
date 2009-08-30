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



#ifndef map_h
#define map_h

typedef struct map_entry {
	char	*key;
	void	*data;
	struct  map_entry *next;
} map_t;

typedef struct {
	int	nentries;
	map_t	*maplist;
} maptab;


extern maptab *map_new(int nentry);
extern void *map_lookup(maptab *mtab, char *name);
extern void *map_install(maptab *mtab, char *name, void *ptr);
extern int  map_apply(maptab *mtab, int (*f)(), void *p);
#endif
