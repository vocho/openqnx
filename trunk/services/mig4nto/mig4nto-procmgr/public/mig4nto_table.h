/*
 *  mig4nto_table.h
 *
 *  Header file for the table of records api used in various migration
 *  functions.
 */
#ifndef __MIG4NTO_TABLE_H_INCLUDED
#define __MIG4NTO_TABLE_H_INCLUDED

/* Flags to be used with the flags variable in init_table() */
#define F_SORT  0x01    /* Indicates to keep the table sorted */

/* Flags used in the get_next_record() */

/* These are used with the from_rec variable */
#define S_RESET     1   /* Jump the to the first record                 */
#define S_NEXT      2   /* Get the next record                          */
#define S_LAST      3   /* Get the record from the last get_record()    */

/* These are used with the direction variable */
#define T_FORWARD   1   /* scan forward     */
#define T_BACKWARD  2   /* scan backward    */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {    
	unsigned		num_records;	/* Current Number of Records          */
	long			next_record;	/* Number of the last record returned */
									/* from the get_next_record function  */
	long			last_record;	/* Number of the last record returned */
									/* from the get_record function       */
	long			record_size;	/* The size of each record in bytes   */
	unsigned char	flags;			/* Flags on how to operate the table  */          
	int (*search)(const void *p1, const void *p2);                              
									/* search function for lsearch()      */
	void			**recs;			/* Dynamic Table Of Record Pointers   */
} TABLE_T;            
		   
short init_table(TABLE_T *table, long record_size, 
	int (*compare)(const void *p1, const void *p2), unsigned char flags);               
short add_record(TABLE_T *table, void *record);
short delete_record(TABLE_T *table, void *key, long rec_num);
void *get_record(TABLE_T *table, void *key, long *rec_num);
void *get_next_record(TABLE_T *table, short direction, short from_rec, long *rec_num);
void close_table(TABLE_T *table);
void table_stats(TABLE_T *table);

#ifdef __cplusplus
};
#endif

#endif
