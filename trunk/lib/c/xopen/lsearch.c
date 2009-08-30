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

//Our header was screwed up for a while
//#define _ST unsigned
#define _ST size_t
//#define _CONST const
#define _CONST 

/*
 Search for key data in a table/array pointed to by base in which 
 there are nelp arguments of width size each.
*/
static void *_lsearch(const void *key, void *base, _ST *nelp, _ST width,
			          int (*compar)(const void *, const void *), unsigned add) {
	size_t index;
	void   *insert;

	//This generates a warning under watcom
	//for(index = *nelp, insert = base; index > 0; index--, ((char *)insert) += width) {
	for(index = *nelp, insert = base; index > 0; index--, insert = ((char *)insert) + width) {
		if(compar(key, insert) == 0) {
			return insert;
		}
	}

	if(add) {
		memcpy(insert, key, width);
		(*nelp)++;
		return insert;
	} else {
		return NULL;
	}
}

void *lsearch(const void *key, _CONST void *base, _ST *nelp, _ST width,
			  int (*compar)(const void *, const void *)) {
	return _lsearch(key, (void *)base, nelp, width, compar, 1);
}

void *lfind(const void *key, const void *base, _ST *nelp, _ST width,
			  int (*compar)(const void *, const void *)) {
	return _lsearch(key, (void *)base, nelp, width, compar, 0);
}

#undef _ST
#undef _CONST

#if defined(TEST)

#include <stdio.h>
#include <string.h>
#include <search.h>

#define TABSIZE 50
#define ELSIZE 120

char line[ELSIZE], tab[TABSIZE][ELSIZE];
char *findline = "A test\n";

int main(int argc, char **argv) {
	size_t nel = 0;
	void	*entry;

	printf("Grabbing input lines: \n");
	while(fgets(line, ELSIZE, stdin) != NULL && nel < TABSIZE) {
		if(lsearch(line, tab, &nel, ELSIZE, 
					(int (*)(const void *, const void *))strcmp) == NULL) {
			printf("Error! \n");
		}
	}

	printf("Searching for line \n");
	if((entry = lfind(findline, tab, &nel, ELSIZE, 
					(int (*)(const void *, const void *))strcmp))) {
		printf("Found the line \n");
	} else {
		printf("Didn't find the line \n");
	}


	return 0;
}
				
#endif

__SRCVERSION("lsearch.c $Rev: 153052 $");
