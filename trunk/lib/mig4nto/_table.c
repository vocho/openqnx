/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

/*
 * _table.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <mig4nto_table.h>

#define TEST_MAIN   0  /* Set this to non-zero to test the API */
#define DEBUG_OUT   0  /* Set this to non-zero for Debugging output */

void *Lfind (const void *key, const void *base, unsigned num, unsigned width,
			int (*compare)(const void *element1, const void *element2) );
void *Bfind (const void *key, const void *base, unsigned num, unsigned width, 
			 long *slot, int (*compare)(const void *element1, 
			 const void *element2) );

/* 
 * Lfind
 *
 *  The Lfind() function performs a linear search for the value key
 *  in the array of num elements pointed to by base.  Each element of the
 *  array is width bytes in size.   The argument compare is a pointer to
 *  a user-supplied routine that will be called by lfind() to determine 
 *  the relationship of an array element with the key.  One of the arguments
 *  to the compare function will be an array element, and the other will be
 *  key.
 *
 *  The compare function should return 0 if element1 is identical to
 *  element2, and non-zero if the elements are not identical.
 *	
 *	Inputs:
 *		key     Key to search for.
 *		base    Start of the structure to search in.
 *		num     Number of elements.
 *		width   Size of one element.
 *		compare Comparison function.
 *
 *	Outputs:
 *		None 
 *
 *	Returns:
 *		If key is found, Lfind() returns a pointer to the array element in
 *		base that matches it.  If key isn't found, the function returns Null.                          
 */
void *
Lfind(const void *key, const void *base, unsigned num, unsigned width,
	 	int (*compare)(const void *element1, const void *element2))
{
	unsigned    i;
	void        *next_element;
	void        *ret_code = NULL;

	next_element = (void *) base;

	for (i = 0; i < num; i++) {
		if (!compare(key, next_element)) {
			ret_code = next_element;
			break;
		}    
		next_element =  (void*) (((unsigned) next_element) + width);
	}
	return ret_code;
}  

/* 
 *  Bfind
 *
 *  The Bfind() function performs a linear search for the value key
 *  in the array of num elements pointed to by base.  Each element of the
 *  array is width bytes in size.   The argument compare is a pointer to
 *  a user-supplied routine that will be called by Bfind() to determine 
 *  the relationship of an array element with the key.  One of the arguments
 *  to the compare function will be an array element, and the other will be
 *  key.
 *
 *  The compare function should return 0 if element1 is identical to
 *  element2, and non-zero if the elements are not identical.
 *
 *  NOTE: In the future we could change this to use a modified bsearch to
 *        avoid a linear search
 *
 *	Inputs:
 *		key     Key to search for.
 *		base    Start of the structure to search in.
 *		num     Number of elements.
 *		width   Size of one element.
 *		compare Comparison function.
 *
 *	Outputs:
 *		slot: The last location of where the key was less than 
 *			  the locations record (according to the compare 
 *			  function)
 *
 *	Returns:
 *		If key is found, Bfind() returns a pointer to the array element in
 *		base that matches it.  If key isn't found, the function returns Null.                          
 */
void *
Bfind(const void *key, const void *base, unsigned num, 
	 	 unsigned width, long *slot,
	 	 int (*compare)(const void *element1, const void *element2))
{
	unsigned    i;
	void        *next_element;
	void        *ret_code = NULL;
	int         compare_result;
	
	*slot = -1;
	next_element = (void *) base;

	for (i = 0; i < num; i++) {
		compare_result = compare(key, next_element);
		if (!compare_result) {
			ret_code = next_element;
			break;
		} else {
			if (compare_result > 0)
				*slot = i;    
		}
		next_element = (void*) (((unsigned) next_element) + width);
	}
	return ret_code;
}  

/* 
 *  init_table
 *
 *	This function will initialize the record table.
 *	
 *	Inputs:
 *		table           Pointer to the table structure to be initialzed 
 *		record_size     The size in bytes of each record
 *		compare         A pointer to the compare function to be used in 
 *						the lsearch and bsearch functions.
 *		flags           flags on how to process the table
 *
 *	Outputs:
 *		table           The initialized table structure             
 *
 *	Returns:
 *		Returns a one upon success, otherwise a zero is returned
 */
short
init_table(TABLE_T *table,  long record_size,
	int (*search)(const void *p1, const void *p2), unsigned char flags)
{
	short   return_code = 1;
		   
	/* Test for valid function parameters */                 
	if (table == NULL || search == NULL || record_size <= 0)
		return_code = 0;
	else {
		memset(table, 0x00, sizeof(TABLE_T));
		table->record_size = record_size;
		table->search = search;
		table->flags = flags;
	}
	return return_code;
}

/* 
 *  add_record
 *
 *	This function will add the given record to the given table.
 *	Note that the record pointer is stored in the table, thus the
 *	user should not free the pointer after the record is added.
 *	
 *	Inputs:
 *		table   Pointer to the table that will have the given record
 *				added to
 *		record  The record to be added to the given table   
 *
 *	Outputs:
 *		table   The modified table with the addition of the given record                  
 *	Returns:
 *		Returns a one upon success, otherwise a zero is returned 
 */
short
add_record(TABLE_T *table, void *record)     
{
	long    new_size;
	long    slot;
	void    *double_key[1], **trecs;      
	  
	if (table == NULL || record == NULL)
		return 0;
    new_size = ((table->num_records+1) * sizeof(void*));
    trecs = (void *) realloc(table->recs, new_size);
    if (!trecs)
        return 0;
    table->recs = trecs;
    table->num_records++;
    if (table->flags & F_SORT) {
        /* Need to keep this table sorted, add an insertion here */

        /* Convert to double pointers to be consistant with the   
           record table */                                         
        double_key[0] = record;                                      
			
        /* Scan thru the table and determine where to insert this 
           record */

        Bfind(double_key, table->recs, table->num_records - 1, 
              sizeof(void*), &slot, table->search);                              
        /* Now move everything above it and insert the record */
        slot++;
        memmove(&table->recs[slot+1], &table->recs[slot], 
                ((table->num_records - 1) - slot) * sizeof(void*));

        table->recs[slot] = record; /* Insert the record */

#if DEBUG_OUT
        printf("add_record (sort) -> Added ptr: %08X  record count: %d slot: %d \n", (int) record, 
            (int) table->num_records, (int) slot);
#endif
    } else {
        /* Not sorted, just add the record to the end */
        table->recs[table->num_records - 1] = record;       
				
#if DEBUG_OUT        
		printf("add_record-> Added ptr: %08X  record count: %d\n", (int) record, (int) table->num_records);
#endif          
	}            
	return 1;
}

/*
 *  delete_record
 *
 *	This function will delete the given record to the given table. 
 *	Note that the key pointer is a pointer to the entire record.
 *	Also the user can delete the record by it's record number 
 *	(or index);
 *	
 *	Inputs:
 *		table   Pointer to the table that will have the given record
 *				delelted from
 *		key     The record to use in searching the table for the record
 *				to be deleted   
 *		rec_num The record number to be removed 
 *	Outputs:
 *		table   The modified table with the removal of the record whose    
 *				key was given               
 *
 *	Returns:
 *		Returns a one upon success, otherwise a zero is returned 
 */
short
delete_record(TABLE_T *table, void *key, long rec_num)        
{
	short   return_code = 0;
	void    *double_key[1];     
	long    index;  
	void    **found = NULL; 
	long    new_size;                   
	long    num_slots_to_move;


	if (key != NULL) {
		/* Convert to double pointers to be consistant with the record table */ 
		double_key[0] = key;                                      
	
		if(table->flags & F_SORT) {
			/* Table is sorted */
			found = bsearch(double_key, table->recs, 
				table->num_records, sizeof(void*), table->search);
		} else {
			/* Search linearly */    
			found = Lfind(double_key, table->recs,        
				table->num_records, sizeof(void*), table->search);  
		}
	} else {
		if ((rec_num >= 0) && (rec_num < table->num_records))
			found = &table->recs[rec_num];
	}
	
	if (found != NULL) {
		index = (found - table->recs);  /* Determine the Index */

		/* reset the next record index that is used in the 
		   get_next_record function */
		if (table->next_record >= index) {
			table->next_record--;
			if (table->next_record < 0)
				table->next_record = 0;
		}
		 
#if DEBUG_OUT
			printf("delete_record-> rec ptr: %08X  index: %d  num records: %d\n",
			(int) table->recs[index],  (int) index, (int) table->num_records);  
#endif
		
		/* Free the pointer at the given slot */
		free(table->recs[index]);
		
		/* Shift everything up a slot */
		num_slots_to_move = (table->num_records - index) - 1;
		if (num_slots_to_move)
			memmove(&table->recs[index], &table->recs[index+1], 
					num_slots_to_move * sizeof(void*));
			 
		/* Re-alloc the table of record pointers to shrink it */
		table->num_records--;               
		if (table->num_records) {
			new_size = (table->num_records * sizeof(void*)); 
			table->recs = (void *) realloc(table->recs, new_size);
		} else {
			free(table->recs);
			table->recs = NULL;
		}
		return_code = 1;
	}                                       
	return return_code;
}

/*
 *  get_record
 *
 *	This function will search the given table for the record whose
 *	key is in the "key" parameter.  The located record's pointer will
 *	then be returned if found, otherwise a NULL value will be 
 *	returned.  Note that we use double pointers (since the original
 *	table contains a pointer to a pointer) therefore throught the 
 *	functions we must "normalize" the passed key to a double pointer
 *	so that everything will be consistant.
 *		Also, the key value is a pointer to the entire record and
 *	not just the "key" element.  WARNING: do not free or manipulate
 *	the returned pointer.
 *	
 *	Inputs:
 *		table   Pointer to the table that will have the given record
 *				delelted from
 *		key     The record to use in searching the table for the record
 *				to be returned.     
 *
 *	Outputs:
 *		rec_num The record number (slot) of the found record.
 *
 *	Returns:
 *		Returns a pointer to the located record upon success, otherwise a 
 *		NULL value is returned  that indicates the record was not found.
 */
void *
get_record(TABLE_T *table, void *key, long *rec_num)        
{
	void    *return_code = NULL;
	void    *double_key[1]; 
	void    **found = NULL;
	long    index;   

	/* Convert to double pointers to be consistant with the record table */ 
	double_key[0] = key;  
	
	if (table->flags & F_SORT) {
		/* Table is sorted */
		found = bsearch(double_key, table->recs, 
			table->num_records, sizeof(void*), table->search);
	} else {
		/* Search linearly */    
		found = Lfind(double_key, table->recs,        
			table->num_records, sizeof(void*), table->search);  
	}   
		 
	if (found != NULL) { /* Change back from a double to a single pointer */
		return_code = *found;  
		index = (found - table->recs);   /* Determine the Index */        
		
		/* Save this index to be used in the get_next_record() function */  
		table->last_record = index;
		*rec_num = index;

#if DEBUG_OUT
		printf("get_record-> ptr found: %08X  index: %d  num_recs: %d\n",
			(int) found, (int) index, (int) table->num_records);
#endif
	} 
	return return_code;
}

/*
 *  get_next_record
 *
 *	This function will return a pointer to the next record from
 *	the table.  The direction variable indicates which direction
 *	to travel and the from_rec will indicate where to start from.
 *	(This is kind of a record getting fuction with a fseek flavor).
 *	Note: do not free or manipulate the returned pointer.
 *	
 *	Inputs:
 *		table       Pointer to the table.
 *		
 *		direction:  Which direction to travel.  The macros to use are:
 *					T_FORWARD:  travel forward
 *					T_BACKWARD: travel backward
 *			
 *		from_rec    Which record to start from.  The macros to use are:
 *		   S_RESET: start from the very first record.  In
 *					other words, the first record within the
 *					table will be returned, regardless of the
 *					current direction flag.
 *		   S_NEXT:  go to the next record from the previously
 *					returned record and return it's pointer. 
 *					(either go forward one
 *					record, or backward one record depending
 *					one the value of direction flag).
 *		   S_LAST:  go to the last record that was returned
 *					from the get_record function and return
 *					a pointer from it's record.         
 *
 *	Outputs:
 *		rec_num     The record number (slot) of the found record.
 *
 *	Returns:
 *		Returns a pointer to the located record upon success, otherwise a 
 *		NULL value is returned  that indicates that the next record was 
 *		not at a valid record location.                            
 */
void *
get_next_record(TABLE_T *table, short direction, short from_rec, long *rec_num)        
{
	void    *return_code = NULL;
					
	if (table->num_records) {
		switch (from_rec) {
		case S_RESET:
			table->next_record = 0;
			return_code = table->recs[0];
			*rec_num = 0;
			break;
			
		case S_NEXT:
			if (direction == T_FORWARD) {
				if (table->next_record < (table->num_records)) {
					return_code = table->recs[table->next_record];
					*rec_num = table->next_record;
					table->next_record++;
				}
			} else if (direction == T_BACKWARD) {
				if (table->next_record) {
					table->next_record--;
					return_code = table->recs[table->next_record];
					*rec_num = table->next_record;
				}                                                           
			}
			break;
			
		case S_LAST: 
			if (table->last_record < table->num_records) {
				return_code = table->recs[table->last_record];
				*rec_num = table->last_record;
			}
			break;      
		}        
	}

	return return_code;
}

/*
 *  close_table
 *
 *	This function will close the table by freeing up all of the 
 *	records.
 *	
 *	Inputs:
 *		table   Pointer to the table that will have all of its records
 *				removed.
 *
 *	Outputs:
 *		table   The "cleaned" table.
 *
 *	Returns:
 *		None
 */
void
close_table(TABLE_T *table)
{
	long    i;

	if (table) {
		for (i = 0; i < table->num_records; i++) {
			if (table->recs[i]) {
#if DEBUG_OUT
				printf("close_table-> ptr removed: %08X  index: %d \n",
				(int) table->recs[i], (int) i);
#endif

				free(table->recs[i]);
			}    
		}
		memset(table, 0x00, sizeof(TABLE_T));
	}
}

/*
 *  table_stats
 *
 *	This function will printout the table configuration.
 *	
 *	Inputs:
 *		table   Pointer to the table that will have it's config
 *				data printed.
 *	Outputs:
 *		none
 *
 *	Returns:
 *		None
 */
void
table_stats(TABLE_T *table)
{
	if (table) {
		printf("Table Configuration Data:\n");
		printf("Number of Records: %d\n", (int) table->num_records);
		printf("Next Record: %d\n", (int) table->next_record);
		printf("Last Record: %d\n", (int) table->last_record);    
		printf("Record Size in Bytes: %d\n", (int) table->record_size);    
		printf("Record Flags: %02X\n", table->flags);    
		printf("Search Function ptr: %p\n", table->search);
		printf("Records ptr: %p\n", table->recs);
	}
}


#if TEST_MAIN

#define DATA_SIZE  128  
#define NUM_TEST_RECS   (sizeof(test_data) / sizeof(TEST_DATA_TYPE))     

typedef struct 
{                 
	long    stuff_before_key;
	short   key;
	char    data[DATA_SIZE];
}TEST_REC_TYPE;

typedef struct 
{       
	short   key;
	char    *data;
}TEST_DATA_TYPE;

/* 
	int compare(const void *p1, const void *p2)

	This function is used for searching the table of test records.
	It works exactly like the function passed to bsearch().

	Inputs:
		p1,p2: pointers to the two records to be compared

	Outputs:
		None 

	Returns:
		Returns -1 if p1 is less than p2, 1 if p1 is greater than
		p2, else a zero is returned to indicate p1 equals p2.
*/

int compare(const void *p1, const void *p2)
{                       
	int ret_code;       
	TEST_REC_TYPE  **ptr2 = (TEST_REC_TYPE  **) p2;  
	TEST_REC_TYPE   *pp2;      
	
	TEST_REC_TYPE  **ptr1 = (TEST_REC_TYPE  **) p1; 
	TEST_REC_TYPE   *pp1;        
	
	pp2 = *ptr2;
	pp1 = *ptr1;
	
	if( pp1->key < pp2->key )
		ret_code = -1;
	else if ( pp1->key == pp2->key )    
		ret_code = 0;                                         
	else
		ret_code = 1;         
		
	return(ret_code);       
}

/* 
	int main(void)
*/

int main(void)
{   
	TABLE_T         Table;
	short           result;
	long            rec_num;
	long            i,j;  
	unsigned char   flags;
	TEST_REC_TYPE   *test_rec = NULL; 
	TEST_REC_TYPE   *found_rec;
	TEST_DATA_TYPE  test_data[] = 
	{ 
		{   4,  "RECORD NUMBER FOUR\n" }, 
		{   8,  "RECORD NUMBER EIGHT\n" },            
		{   5,  "RECORD NUMBER FIVE\n" }, 
		{   16, "RECORD NUMBER SIXTEEN\n" },                       
		{   1,  "RECORD NUMBER ONE\n" },                         
		{   20, "RECORD NUMBER TWENTY\n" },                            
	};   
	
	printf("Table Tester\n");
	for(j = 0; j < 2; j++)
	{
		if (j)
		{
			printf("\n Sorted Table\n");
			flags = F_SORT;
		}
		else
		{
			printf("\n Unsorted Table\n");
			flags = 0;             
		}
		/* Initialize the Table */
		result = init_table(&Table, sizeof(TEST_REC_TYPE), compare, flags);         
		if( result == NULL )
			printf("Error Initializing Table\n");
		
		/* Add some records */        
		for(i = 0; i < NUM_TEST_RECS; i++)
		{   
			test_rec = (TEST_REC_TYPE *) calloc(1, sizeof(TEST_REC_TYPE));
			assert( NULL != test_rec);
			test_rec->key = test_data[i].key;
			strcpy(test_rec->data, test_data[i].data);
			if(! add_record(&Table, test_rec) )
				printf("add_record failed\n");
		}   
				
		/* Get some records */
		test_rec = (TEST_REC_TYPE *) calloc(1, sizeof(TEST_REC_TYPE));
		assert( NULL != test_rec);                                    

		for(i = 0; i < NUM_TEST_RECS; i++) 
		{                       
			memset(test_rec, 0x00, sizeof(test_rec));
			test_rec->key = test_data[i].key;  
			found_rec = (TEST_REC_TYPE*) get_record(&Table, test_rec, &rec_num );
			if(found_rec == NULL)
				printf("Error in get_record\n");
			else
			{
				printf("Record Found-> ptr: %p Key: %d  Data: %s",
					found_rec, found_rec->key, found_rec->data);
			}       
		} 

		/* Print out the stats */
		table_stats(&Table);
	
		/* Delete some records */           
		for(i = 0; i < NUM_TEST_RECS; i++) 
		{                       
			memset(test_rec, 0x00, sizeof(test_rec));
			test_rec->key = test_data[i].key;  
			if(!delete_record(&Table, test_rec, rec_num ))
				printf("Error in delete_record\n");
		}      
	
		free(test_rec);
		test_rec = NULL;

		/* Add some records */        
		for(i = 0; i < NUM_TEST_RECS; i++)
		{   
			test_rec = (TEST_REC_TYPE *) calloc(1, sizeof(TEST_REC_TYPE));      
			assert( NULL != test_rec);                                    
			test_rec->key = test_data[i].key;
			strcpy(test_rec->data, test_data[i].data);
			if(! add_record(&Table, test_rec) )
				printf("add_record failed\n");
		}    
	
		/* Test get next record */              
		for(i = 0; i < NUM_TEST_RECS + 1; i++) 
		{   
			if(i)
				found_rec =
			 (TEST_REC_TYPE*) get_next_record(&Table,  T_FORWARD, S_NEXT, &rec_num); 
			else
				found_rec = 
			 (TEST_REC_TYPE*) get_next_record(&Table,  T_FORWARD, S_RESET, &rec_num);              
			if(found_rec == NULL)
				printf("Error in get_next_record\n");
			else
			{  
				printf("get_next_record returned > ptr: %p Key: %d  Data: %s",
					found_rec, found_rec->key, found_rec->data); 
			} 
		}

		/* Test Deleting some records while scanning thru the list */
		printf("Test Deleting and Scanning records at the same time\n");

		printf("Delete Some Records\n");        
		get_next_record(&Table,  T_FORWARD, S_RESET, &rec_num);              
		for(i = 0; i < NUM_TEST_RECS; i++) 
		{
			found_rec = 
				(TEST_REC_TYPE*) get_next_record(&Table,  T_FORWARD, S_NEXT, &rec_num);
			if(i % 2)
			{ 
				printf("deleted record > ptr: %p Key: %d RecNum: %d Data: %s ", 
					found_rec, found_rec->key, rec_num, found_rec->data);    
				delete_record(&Table, NULL, rec_num);
			}
		}
		printf("Which Records are Left\n");        
		get_next_record(&Table,  T_FORWARD, S_RESET, &rec_num);              
		for(i = 0; i < NUM_TEST_RECS; i++) 
		{
			found_rec = 
				(TEST_REC_TYPE*) get_next_record(&Table, T_FORWARD, S_NEXT, &rec_num);
			if(found_rec)
			{ 
				printf("found record > ptr: %p Key: %d  Data: %s", 
					found_rec, found_rec->key, found_rec->data);  
			}
		}

		/* Print out the stats */
		table_stats(&Table);     

		close_table(&Table);
	}
	return(0);
}
#endif 

