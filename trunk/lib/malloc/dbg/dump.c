/*
 * $QNXtpLicenseC:
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





/*
 * (c) Copyright 1990, 1991 Conor P. Cahill (uunet!virtech!cpcahil).  
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include "malloc-lib.h"
#include "mallocint.h"
#include "tostring.h"


#define MA_PADDING (__ROUND(sizeof(Dhead), _MALLOC_ALIGN)-sizeof(Dhead))

typedef
struct chain_totals {
    int c_entries;
    int c_size;
} chain_total_t;

int __malloc_detail;

int writeFully(int fd, const char *buf, size_t n);
void _malloc_event(const char *event, const char *funcname, const char *file, 
              int line, const void *link, int type);

/*
 * various macro definitions used within this module.
 */

#define WRITEOUT(fd,str,len)	if( writeFully(fd,str,(unsigned)(len)) != (len) ) \
		{ \
		    (void) writeFully(2,ERRSTR,\
			     (unsigned)strlen(ERRSTR));\
		    exit(120); \
		}

#define DETAIL_NONE 		0
#define DETAIL_NOT_SET_YET	-1
#define DETAIL_ST_COL		(sizeof(DETAIL_HDR_3)-1)
#define ERRSTR	"I/O Error on malloc dump file descriptor\n"
#define FILE_LEN		20
#define LIST_ALL		1
#define LIST_SOME		2
#define LIST_UNREFERENCED	3
#define NUM_BYTES		7
#define TITLE1			" Dump of Malloc Chain "
#define TITLE2			" Dump of Leaked Heap Memory "

#define DETAIL_HDR_1 \
     "                                         ACTUAL SIZE     "
#define DETAIL_HDR_2 \
     "  PTR      NEXT     PREV     FLAGS      INT       HEX    "
#define DETAIL_HDR_3 \
     "-------- -------- -------- ---------- -------- --------- "

#define NORMAL_HDR_1 \
 "POINTER     FILE  WHERE         LINE       ALLOC        DATA     HEX DUMP   \n"
#define NORMAL_HDR_2 \
 "TO DATA      ALLOCATED         NUMBER      FUNCT       LENGTH  OF BYTES 1-7 \n"
#define NORMAL_HDR_3 \
 "-------- -------------------- -------- -------------- ------- --------------\n"

#define THISBUFSIZE (sizeof(NORMAL_HDR_3)+sizeof(DETAIL_HDR_3))


/*
 * Function:	malloc_dump()
 *
 * Purpose:	to dump a printed copy of the malloc chain and
 *		associated data elements
 *
 * Arguments:	fd	- file descriptor to write data to
 *
 * Returns:	nothing of any use
 *
 * Narrative:	Just print out all the data
 *
 */
void
malloc_dump(int fd)
{
    malloc_list_items(fd,LIST_ALL,0L,0L);
}

/*
 * Function:	malloc_list()
 *
 * Purpose:	to dump a printed copy of the malloc chain and
 *		associated data elements
 *
 * Arguments:	fd	- file descriptor to write data to
 *		histid1 - id of the first record to display
 *		histid2 - id one above the last record to display
 *
 * Returns:	nothing of any use
 *
 * Narrative:	Just call malloc_list_items to display the data
 *
 */
void
malloc_list(int fd, ulong_t histid1, ulong_t histid2)
{
    malloc_list_items(fd, LIST_SOME, histid1, histid2);
}

chain_total_t malloc_list_chain_items(int, int, ulong_t, ulong_t, 
             arena_range_t *, chain_t *, int, char *);
static void write_column_headers(int fd, int detail);

chain_total_t malloc_list_chain_items_bands(int fd, int list_type,
	       ulong_t histid1, ulong_t histid2, arena_range_t *range, 
         chain_t *chain, int detail, char *buffer)
{
	/* Look through the bands */
  int nb;
  Band *band;
  chain_total_t	  total;

  total.c_entries = 0;
  total.c_size = 0;

  for (nb = 0; nb < *__pnband; nb++) 
  {
    arena_range_t new_range;
	  Block *bp = NULL;
    chain_total_t ctotal;
  	chain_total_t	  round_total;
    int free_count = 0, free_size = 0;
    int esize;
    char		  buffer1[THISBUFSIZE];

  	round_total.c_entries = 0;
  	round_total.c_size = 0;
	  band = __pBands[nb];
	  esize = band->nbpe + SB_OVERHEAD();

    sprintf(buffer, "\n\nBAND %x, small buffer size %u:\n\n", 
           (uint_t)band, band->nbpe);
		WRITEOUT(fd, buffer, strlen(buffer));

	  /*
	  * For each Block on the allocated list
	  */
    for (bp = band->alist; bp; bp = bp->next)
    {
			ctotal.c_entries = 0;
	    new_range.r_start = (char *)(bp + 1);
	    new_range.r_end = new_range.r_start + esize*band->nalloc; 
      new_range.r_type = RANGE_BLOCK;
      new_range.un.r_block = bp;
	    ctotal = malloc_list_chain_items(fd, list_type, histid1, 
      histid2, &new_range, &bp->malloc_chain, 
      detail, buffer);
	    round_total.c_entries += ctotal.c_entries;
	    round_total.c_size += ctotal.c_size;
	    free_count += bp->navail;
	    free_size += bp->navail * band->nbpe;
			if (ctotal.c_entries > 0)
				buffer[0] = '\0'; // inhibit header
		}

	  /*
	  * For each Block on the depleted list
	  */
    for (bp = band->dlist; bp; bp = bp->next)
    {
			ctotal.c_entries = 0;
	    new_range.r_start = (char *)(bp + 1);
	    new_range.r_end = new_range.r_start + esize*band->nalloc; 
      new_range.r_type = RANGE_BLOCK;
      new_range.un.r_block = bp;
      ctotal = malloc_list_chain_items(fd, list_type, histid1, 
                            histid2, &new_range, &bp->malloc_chain, detail, 
                            buffer);
	    round_total.c_entries += ctotal.c_entries;
	    round_total.c_size += ctotal.c_size;
			if (ctotal.c_entries > 0)
				buffer[0] = '\0'; // inhibit header
    }
    if( list_type == LIST_SOME || list_type == LIST_ALL )
    {
      sprintf(buffer1, "\n%d Allocated entries %u bytes\n",
              round_total.c_entries, round_total.c_size);
			WRITEOUT(fd,buffer1,strlen(buffer1));
      sprintf(buffer1, "\n%d Free entries %u bytes\n", free_count, free_size);
			WRITEOUT(fd,buffer1,strlen(buffer1));
		}
    else if (list_type == LIST_UNREFERENCED)
    {
      sprintf(buffer1, "\n%d Leaked entries %u bytes\n",
              round_total.c_entries, round_total.c_size);
			WRITEOUT(fd,buffer1,strlen(buffer1));
    }
	  total.c_entries += round_total.c_entries;
	  total.c_size += round_total.c_size;
	}
	return(total);
}

/*
 * Function:	malloc_list_chain_items()
 *
 * Purpose:	to dump a printed copy of a malloc chain and
 *		associated data elements
 *
 * Arguments:	fd	  - file descriptor to write data to
 *		list_type - type of list (all records, or a selected list)
 *		histid1	  - first id to list (if type is some)
 *		histid2   - one above last id to list
 *              chain     - the chain to dump
 *
 * Returns:	nothing of any use
 *
 * Narrative:	Just print out all the data
 *
 * Notes:	This function is implemented using low level calls because
 * 		of the likelyhood that the malloc tree is damaged when it
 *		is called.  (Lots of things in the c library use malloc and
 *		we don't want to get into a catch-22).
 *
 */
chain_total_t
malloc_list_chain_items(int fd, int list_type,
	ulong_t histid1, ulong_t histid2,
	arena_range_t *range, chain_t *chain, int detail, char *buffer)
{
    void		  exit();
    char		* func;
    int			  i;
    DebugInfo_t 	* ptr;
    Flink		* flink;
    chain_total_t	  total;
		int printed_header=0;

    total.c_entries = 0;
    total.c_size = 0;

    /*
     * for each element in the trace
     */
    for(flink = chain->head; flink; flink = flink->f_next)
    {
			ptr = &((Dhead *)flink)->d_debug;

			if ((ptr->flag & M_INUSE) != 0
	    && (list_type == LIST_SOME || list_type == LIST_ALL))
			{
	    total.c_entries ++;
	    total.c_size += (ulong_t)_msize((Dhead *)flink+1);
			}
			if (range->r_type == RANGE_ARENA
	    && flink->f_prev != NULL
	    && ((char *)flink->f_prev + DH_LEN(flink->f_prev)) != (char *)flink) {
	  		Flink *fp;
	  		Flink *gap = (Flink *)((char *)flink->f_prev + DH_LEN(flink->f_prev));
	  		int found = 0;
	  		/*
	  		* We have a gap.
	  		* Look in the free lists, then check for a Block
	  		*/
				for (i=0; i < __flist_nbins ; i++) {
					Flink *fp_list;
					Flink *curflistptr;
					curflistptr = __malloc_getflistptr();
					fp_list = &(curflistptr[i]);
    			for (fp = fp_list->f_next;
            	!found && fp != fp_list; fp = fp->f_next)
    			{
    				if (fp == gap) {
        			found = 1;
      			}
					}
    		}
  		}

			/*
	 		* if this item is not in use and we are not in detail mode or
	 		* we are not in list-all mode.
	 		*/
			if(((ptr->flag & M_INUSE) == 0)
     		&& ((detail == DETAIL_NONE) || (list_type != LIST_ALL)) )
			{
				continue;
			}

			/*
	 		* else if we are only listing a range of items, check to see
	 		* if this item is in the correct range.  if not, skip it
	 		*/
			else if(   (list_type == LIST_SOME)
				&& (    (ptr->hist_id < histid1)
	     	|| (ptr->hist_id >= histid2)) )
			{
	  		continue;
			}
			else if (list_type == LIST_UNREFERENCED)
			{
	  		if ((ptr->flag & M_INUSE) == 0 || (ptr->flag & M_REFERENCED) != 0)
	  		{
	      	continue;
	  		}
				//if ((!(ptr->dbg)) || (ptr->dbg->tid < 0))
					//continue;
				if (!(ptr->callerpc_line))
					continue;
	  		total.c_entries++;
	  		total.c_size += (ulong_t)_musize((Dhead *)flink+1);
				_malloc_event("LEAK", "", NULL, (ulong_t)ptr->callerpc_line, flink, M_LEAK);
			}
			if (!printed_header) {
				int len;
				printed_header=1;
				len = strlen(buffer);	
				if (len) {
    			write_column_headers(fd,detail);
				}
			}

			/*
	 		* fill in the string with blanks
	 		*/
			for(i=0; i < (sizeof(DETAIL_HDR_3)+sizeof(NORMAL_HDR_3)); i++)
			{
	    		buffer[i] = ' ';
			}

			/*
	 		* handle detail output
	 		*/
			if( detail != DETAIL_NONE )
			{
	    	(void) tostring(buffer,
				(ulong_t)ptr,8,B_HEX,' ');
	    	(void) tostring(buffer+9,
				(ulong_t)(flink->f_next),8,B_HEX,'0');
	    	(void) tostring(buffer+18,
				(ulong_t)(flink->f_prev),8,B_HEX,'0');
	    	(void) tostring(buffer+27,
				(ulong_t)ptr->flag,10,B_HEX,'0');
	    	(void) tostring(buffer+38,
				(ulong_t)_msize((Dhead *)flink+1),8,B_DEC,' ');
	    	(void) tostring(buffer+47,
				(ulong_t)_msize((Dhead *)flink+1),8,B_HEX,'0');
	    	buffer[46] = '(';
	    	buffer[56] = ')';
			}

			/*
	 		* and now add in the normal stuff
	 		*/
			(void) tostring(buffer+detail,
		    (ulong_t) ((Dhead *)flink+1), 8, B_HEX, ' ');

			/*
	 		* if a file has been specified
	 		*/
			if( (ptr->file != NULL)	 && (ptr->file[0] != '\0') )
			{
	    	int len, j;
	    	if ((len = strlen(ptr->file)) > FILE_LEN) {
					buffer[detail+9] = '.';
					buffer[detail+10] = '.';
					buffer[detail+11] = '.';
	        for(i=len-(FILE_LEN-3), j=0; (j < FILE_LEN) && (ptr->file[i] != '\0'); i++, j++)
	        {
		    		buffer[detail+12+j] = ptr->file[i];
	        }
	    } else {
	      for(i=0; (i < FILE_LEN) && (ptr->file[i] != '\0'); i++)
	      {
		    	buffer[detail+9+i] = ptr->file[i];
	      }
	    }

	    (void) tostring(buffer+detail+30,
				(ulong_t)ptr->callerpc_line,7,B_DEC, ' ');
			}
			else
			{
	    		if ((long)ptr->callerpc_line == -1)
	    		{
					for(i=0; i < (sizeof("unknown")-1); i++)
					{
		    		buffer[detail+9+i] = "unknown"[i];
					}
	    		}
	    		else
	    		{
					(void) tostring(buffer+detail+30,
					(ulong_t)ptr->callerpc_line,8,B_HEX, ' ');
	    		}
			}
	    	
		/*
	 	* determine name of function to use 
	 	*/
		switch( GETTYPE(ptr) )
		{
	    case M_T_MALLOC:
			func = "malloc";
			break;
    
	    case M_T_REALLOC:
			func = "realloc";
			break;
    
	    case M_T_CALLOC:
			func = "calloc";
			break;

	    default:
			func = "unknown";
			break;
		}

		/*
	 	* copy the function name into the string.
	 	*/
		for( i=0; func[i] != '\0'; i++)
		{
	    buffer[detail+39+i] = func[i];
		}

		/*
	 	* add the call number
	 	*/
		buffer[detail+39+ i++] = '(';
		i += tostring(buffer+detail+39+i,(ulong_t)ptr->id,0,B_DEC,' ');
		buffer[detail+39+i] = ')';
    
		/*
	 	* display the length of the segment
	 	*/
		(void) tostring(buffer+detail+54,
		    (ulong_t)DH_ULEN(flink),7,B_DEC,' ');

		/*
	 	* display the first seven bytes of data
	 	*/
		for( i=0; (i < NUM_BYTES) && (i < DH_ULEN(flink)); i++)
		{
	    (void) tostring(buffer+detail + 62 + (i * 2),
			((ulong_t)*((unsigned char *)(((Dhead *)flink)+1)+i)), 2, B_HEX, '0');
		}

		buffer[detail + sizeof(NORMAL_HDR_3)] = '\n';
		WRITEOUT(fd,buffer,detail+sizeof(NORMAL_HDR_3)+1);
		{
			char buf[256];
			int start=0;
			int cnt=0;
			for (i=0; i < 15; i++) {
      	buf[i]=' ';
      	start++;
      }
			buf[start+(cnt++)] = 'C';
			buf[start+(cnt++)] = 'P';
			buf[start+(cnt++)] = 'U';
			buf[start+(cnt++)] = ':';
			tostring(buf+start+cnt,(ptr->dbg->cpu),2,B_DEC,'0');
			cnt+=2;
			for (i=0; i < 2; i++) {
				buf[start+cnt] = ' ';
				cnt++;	
			}
			buf[start+(cnt++)] = 'P';
			buf[start+(cnt++)] = 'I';
			buf[start+(cnt++)] = 'D';
			buf[start+(cnt++)] = ':';
			tostring(buf+start+cnt,(getpid()),10,B_DEC,'0');
			cnt+=10;
			for (i=0; i < 2; i++) {
				buf[start+cnt] = ' ';
				cnt++;	
			}
			buf[start+(cnt++)] = 'T';
			buf[start+(cnt++)] = 'I';
			buf[start+(cnt++)] = 'D';
			buf[start+(cnt++)] = ':';
			tostring(buf+start+cnt,(ptr->dbg->tid),5,B_DEC,'0');
			cnt+=5;
			for (i=0; i < 2; i++) {
				buf[start+cnt] = ' ';
				cnt++;	
			}
			buf[start+(cnt++)] = 'T';
			buf[start+(cnt++)] = 'S';
			buf[start+(cnt++)] = ':';
			tostring64(buf+start+cnt,(ptr->dbg->ts),18,B_HEX,'0');
			cnt+=18;
      buf[start+(cnt++)]='\n';
      WRITEOUT(fd,buf,start+cnt);
		}
        {
          char buf[50];
          int start=0;
					__Dbg_Data *dd;	
					__Dbg_St   *ds;
          for (i=0; i < 21; i++) {
            buf[i]=' ';
            start++;
          }
			dd = ptr->dbg;
			if (dd) {
				ds = dd->bt;
				i=0;
				while (ds) {
         buf[start+0]='B';
         buf[start+1]='T';
         buf[start+2]=' ';
         tostring(buf+start+3,i+1,2,B_DEC,' ');
         buf[start+5]=':';
         buf[start+6]=' ';
         tostring(buf+start+7,(ulong_t)(ds->line),10,B_HEX,'0');
         buf[start+17]='\n';
         WRITEOUT(fd,buf,start+18);
				 ds = ds->next;
				 if (ds && (ds->line == dd->bt->line))
					 break;
				 i++;
				}
			}
        }
  }

  return total;
} /* malloc_list_chain_items(... */

/*
 * Function:	malloc_list_items()
 *
 * Purpose:	to dump a printed copy of the malloc chain and
 *		associated data elements
 *
 * Arguments:	fd	  - file descriptor to write data to
 *		list_type - type of list (all records, or a selected list)
 *		histid1	  - first id to list (if type is some)
 *		histid2   - one above last id to list
 *
 * Returns:	nothing of any use
 *
 * Narrative:	Just print out all the data
 *
 * Notes:	This function is implemented using low level calls because
 * 		of the likelyhood that the malloc tree is damaged when it
 *		is called.  (Lots of things in the c library use malloc and
 *		we don't want to get into a catch-22).
 *
 */

static void
write_column_headers(int fd, int detail)
{
    /*
     * write out the column headers
     */
    if( detail != DETAIL_NONE )
    {
	WRITEOUT(fd,DETAIL_HDR_1,sizeof(DETAIL_HDR_1)-1);
	WRITEOUT(fd,NORMAL_HDR_1,sizeof(NORMAL_HDR_1)-1);
	WRITEOUT(fd,DETAIL_HDR_2,sizeof(DETAIL_HDR_2)-1);
	WRITEOUT(fd,NORMAL_HDR_2,sizeof(NORMAL_HDR_2)-1);
	WRITEOUT(fd,DETAIL_HDR_3,sizeof(DETAIL_HDR_3)-1);
	WRITEOUT(fd,NORMAL_HDR_3,sizeof(NORMAL_HDR_3)-1);
    }
    else
    {
	WRITEOUT(fd,NORMAL_HDR_1,sizeof(NORMAL_HDR_1)-1);
	WRITEOUT(fd,NORMAL_HDR_2,sizeof(NORMAL_HDR_2)-1);
	WRITEOUT(fd,NORMAL_HDR_3,sizeof(NORMAL_HDR_3)-1);
    }
}

void
malloc_list_items(int fd, int list_type, ulong_t histid1, ulong_t histid2)
{
    char		  buffer[THISBUFSIZE];
    int		  detail;
    void		  exit();
    int			  i;
    int			  loc;
    Arena		* ap;
    Band		* band;
    int			  nb;
    int			  arena_count = 0, arena_size = 0;
    chain_total_t	  totals;
    chain_total_t	  btotals;
    chain_total_t	  running_arena_totals;
    chain_total_t	  all_totals;
    char		  buffer1[THISBUFSIZE];
    arena_range_t range;

		running_arena_totals.c_entries=0;
		running_arena_totals.c_size=0;
		all_totals.c_entries=0;
		all_totals.c_size=0;

    ENTER();
    MALLOC_INIT();

    /*
     * See if they want full detail (includes internal pointer info)
     */
	detail = ( __malloc_detail != 0 ) ? DETAIL_ST_COL : DETAIL_NONE;

    /*
     * fill the title line with asterisks
     */
    for(i=0; i < (detail + sizeof(NORMAL_HDR_3)-1); i++) {
		buffer[i] = '*';
    }
    buffer[i] = '\n';
    buffer[i+1] = '\0';

    /*
     * add in the title  (centered, of course)
     */
    if (list_type == LIST_UNREFERENCED) {
        loc = (i - sizeof(TITLE2)) / 2;
        for(i=0; i < (sizeof(TITLE2)-1); i++) {
		    buffer[loc+i] = TITLE2[i];
        }
    } else {
        loc = (i - sizeof(TITLE1)) / 2;
        for(i=0; i < (sizeof(TITLE1)-1); i++) {
		    buffer[loc+i] = TITLE1[i];
        }
    }

    /*
     * and write it out
     */
    WRITEOUT(fd,buffer,strlen(buffer));

    /*
     * Go through malloc chains for each Arena
     */
		for (ap = __arenas.a_next; ap != &__arenas; ap = ap->a_next) {
        Dhead *dp = (Dhead *)(ap+1);
        Dhead *dpend;
        int    free_size = 0, free_count = 0;

	if (ap->a_size < sizeof(*ap)) panic("a_size");

        arena_count++;
        arena_size += ap->a_size;

        dpend = (Dhead *)((char *)ap + ap->a_size - sizeof(Dtail));

        range.r_start = (char *)dp;
        range.r_end = (char *)dpend;
        range.r_type = RANGE_ARENA;
        range.un.r_arena = ap;

        sprintf(buffer, "\n\nARENA %x, size %u:\n\n", (uint_t)ap, ap->a_size);
				WRITEOUT(fd, buffer, strlen(buffer));

	totals =
	malloc_list_chain_items(fd, list_type, histid1, histid2,
				&range, &ap->a_malloc_chain, detail, buffer);
	running_arena_totals.c_entries += totals.c_entries;
	running_arena_totals.c_size += totals.c_size;

        if( list_type == LIST_SOME || list_type == LIST_ALL )
        {
            Flink *fp;

            sprintf(buffer1, "\n%d Allocated entries %u bytes\n",
                   totals.c_entries, totals.c_size);
						WRITEOUT(fd,buffer1,strlen(buffer1));

            sprintf(buffer1, "\n\nFree list:\n\n");
						WRITEOUT(fd,buffer1,strlen(buffer1));
						for (i=0; i < __flist_nbins ; i++) {
							Flink *fp_list;
							Flink *curflistptr;
							curflistptr = __malloc_getflistptr();
    					fp_list = &(curflistptr[i]);	
            	for (fp = fp_list->f_next; fp != fp_list; fp = fp->f_next)
            	{
                	if ((char *)fp >= range.r_start && (char *)fp <= range.r_end) {
                    	free_count++; free_size += fp->f_size;
                    	printf(" %p..%p\n", fp, (char *)fp + fp->f_size);
                	}
            	}
						}
            sprintf(buffer1, "\n%d Free entries %u bytes\n", free_count, free_size);
				WRITEOUT(fd,buffer1,strlen(buffer1));
        }
        else if (list_type == LIST_UNREFERENCED)
        {
            sprintf(buffer1, "\n%d Leaked entries %u bytes\n",
                   totals.c_entries, totals.c_size);
				WRITEOUT(fd,buffer1,strlen(buffer1));
        }
    }
    if( list_type == LIST_SOME || list_type == LIST_ALL )
    {
        sprintf(buffer1, "\n%d Arenas %u bytes\n", arena_count, arena_size);
				WRITEOUT(fd,buffer1,strlen(buffer1));
    }
		btotals =
		malloc_list_chain_items_bands(fd, list_type, histid1, histid2,
					&range, &ap->a_malloc_chain, detail, buffer);
    if( list_type == LIST_SOME || list_type == LIST_ALL )
    {
      sprintf(buffer1, "\n%d Allocated Band entries %u bytes\n",
              btotals.c_entries, btotals.c_size);
			WRITEOUT(fd,buffer1,strlen(buffer1));

    }
    else if (list_type == LIST_UNREFERENCED)
    {
      sprintf(buffer1, "\n%d Leaked Band entries %u bytes\n",
                   btotals.c_entries, btotals.c_size);
			WRITEOUT(fd,buffer1,strlen(buffer1));
    }

    /*
     * Go through malloc chains for every Block in every Band
     */
    for (nb = 0; nb < *__pnband; nb++) 
    {
	Block *bp;
        int free_count = 0, free_size = 0;
        int esize;

	band = __pBands[nb];
	esize = band->nbpe + SB_OVERHEAD();

        if (band->alist != NULL || band->dlist != NULL) {
            sprintf(buffer1, "\n\nBAND %x, small buffer size %u:\n\n", (uint_t)band, band->nbpe);
				WRITEOUT(fd,buffer1,strlen(buffer1));
        }

        totals.c_entries = 0;
        totals.c_size = 0;
	/*
	 * For each Block on the allocated list
	 */
        for (bp = band->alist; bp; bp = bp->next)
        {
	    int entries = band->nalloc - bp->navail;
	    totals.c_entries += entries;
	    totals.c_size += (entries * band->nbpe);
	    free_count += bp->navail;
	    free_size += bp->navail * band->nbpe;
        }

	/*
	 * For each Block on the depleted list
	 */
        for (bp = band->dlist; bp; bp = bp->next)
        {
	    totals.c_entries += band->nalloc;
	    totals.c_size += (band->nalloc * band->nbpe);
        }

        if ( (band->alist != NULL || band->dlist != NULL) ) 
        {
            if (list_type == LIST_SOME || list_type == LIST_ALL)
            {
                sprintf(buffer1, "\n%d Allocated entries %u bytes\n",
                       totals.c_entries, totals.c_size);
				WRITEOUT(fd,buffer1,strlen(buffer1));
        sprintf(buffer1, "\n%d Free entries %u bytes\n", free_count, free_size);
				WRITEOUT(fd,buffer1,strlen(buffer1));
            }
        }
    }

		all_totals.c_entries += running_arena_totals.c_entries + btotals.c_entries;
		all_totals.c_size += running_arena_totals.c_size + btotals.c_size;

    if( list_type == LIST_SOME || list_type == LIST_ALL )
    {
      sprintf(buffer1, "\n%d Total Allocated entries %u bytes\n",
              all_totals.c_entries, all_totals.c_size);
			WRITEOUT(fd,buffer1,strlen(buffer1));

    }
    else if (list_type == LIST_UNREFERENCED)
    {
      sprintf(buffer1, "\n%d Total Leaked entries %u bytes\n",
                   all_totals.c_entries, all_totals.c_size);
			WRITEOUT(fd,buffer1,strlen(buffer1));
    }

    if( detail != DETAIL_NONE )
    {
	WRITEOUT(fd,"Malloc start:      ",19);
	(void) tostring(buffer, (ulong_t) _malloc_start, 8, B_HEX, '0');
	buffer[8] = '\n';
	WRITEOUT(fd,buffer,9);

	WRITEOUT(fd,"Malloc end:        ", 19);
	(void) tostring(buffer, (ulong_t) _malloc_end, 8, B_HEX, '0');
	buffer[8] = '\n';
	WRITEOUT(fd,buffer,9);

#if 0
	WRITEOUT(fd,"Malloc data start: ", 19);
	(void) tostring(buffer,(ulong_t) malloc_data_start,8,B_HEX,'0');
	buffer[8] = '\n';
	WRITEOUT(fd,buffer,9);

	WRITEOUT(fd,"Malloc data end:   ", 19);
	(void) tostring(buffer,(ulong_t) malloc_data_end, 8, B_HEX, '0');
	buffer[8] = '\n';
	WRITEOUT(fd,buffer,9);

	WRITEOUT(fd,"Malloc free list:  ", 19);
	(void) tostring(buffer, (ulong_t) malloc_freelist, 8,B_HEX,'0');
	buffer[8] = '\n';
	WRITEOUT(fd,buffer,9);

	for( ptr=malloc_freelist->freenext; ptr != NULL; ptr = ptr->freenext)
	{
	    WRITEOUT(fd,"                -> ", 19);
	    (void) tostring(buffer, (ulong_t) ptr, 8, B_HEX, '0');
	    buffer[8] = '\n';
	    WRITEOUT(fd,buffer,9);
	}

#endif
    }

    WRITEOUT(fd,"\n",1);
    LEAVE();
    
} /* malloc_dump(... */


/*
 * $Log$
 * Revision 1.17  2007/04/24 15:44:27  shiv
 * PR:29730
 * CI:cburgess
 * CI:xtang
 *
 * this is part of work order WO790334 for IGT. Included enhancements
 * for configurability of the memory allocator.  Includes matching changes for
 * lib/c/alloc and lib/malloc as usual. This is to bring head in line
 * with the work committed to the branch.
 *
 * Revision 1.16  2006/09/28 19:05:57  alain
 * PR:41782
 * CI: shiv@qnx.com
 * CI: cburgess@qnx.com
 *
 * Commiting the work done for IGT-CE on the head.
 *
 * Revision 1.15.2.1  2006/03/02 21:32:02  alain
 *
 *
 * We should not call getenv() anywhere this can lead to potential deadlocks
 * the strategy here is to do all the initialization upfront in the init code.
 * Also some minor formatting fixes.
 *
 * Revision 1.15  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.14  2005/02/25 03:03:37  shiv
 * More fixes for the debug malloc, for the tools to work
 * better.
 * Modified Files:
 * 	malloc-lib.h mallocint.h dbg/calloc.c dbg/dump.c dbg/m_init.c
 * 	dbg/malloc_debug.c dbg/malloc_g.c dbg/mtrace.c dbg/realloc.c
 * 	public/malloc_g/malloc.h
 *
 * Revision 1.13  2005/02/11 19:00:28  shiv
 * Some more malloc_g changes.
 * Modified Files:
 * 	mallocint.h dbg/dump.c dbg/m_init.c dbg/m_perror.c
 *  	dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 *
 * Revision 1.12  2005/01/19 16:53:00  shiv
 * Slight mods to allow easier access to the free list
 * to allow user land to gather information about allocator.
 * Matching with libc checkin.
 * Modified Files:
 * 	dbg/dump.c dbg/malloc_chk.c dbg/malloc_g.c
 *
 * Revision 1.11  2005/01/16 20:38:45  shiv
 * Latest DBG malloc code. Lots of cleanup/optimistions
 * Modified Files:
 * 	common.mk mallocint.h common/tostring.c dbg/analyze.c
 * 	dbg/calloc.c dbg/dump.c dbg/free.c dbg/m_init.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h public/malloc_g/malloc.h
 * 	std/calloc.c std/free.c std/m_init.c std/malloc_wrapper.c
 * 	std/mtrace.c std/realloc.c
 * Added Files:
 * 	dbg/dl_alloc.c dbg/malloc_control.c dbg/malloc_debug.c
 * 	dbg/new.cc public/malloc_g/malloc-control.h
 * 	public/malloc_g/malloc-debug.h
 *
 * Revision 1.10  2004/02/12 15:43:16  shiv
 * Updated copyright/licenses
 * Modified Files:
 * 	common.mk debug.h malloc-lib.h mallocint.h tostring.h
 * 	common/tostring.c dbg/analyze.c dbg/calloc.c dbg/context.h
 * 	dbg/crc.c dbg/dump.c dbg/free.c dbg/m_init.c dbg/m_perror.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc/malloc.h public/malloc_g/malloc-lib.h
 * 	public/malloc_g/malloc.h public/malloc_g/prototypes.h
 * 	std/calloc.c std/context.h std/free.c std/m_init.c
 * 	std/malloc_wrapper.c std/mtrace.c std/realloc.c test/memtest.c
 * 	test/mtrace.c
 *
 * Revision 1.9  2003/10/27 14:27:38  shiv
 * Fixed an error in the output of the initial bytes.
 *
 * Revision 1.8  2003/09/25 23:08:36  shiv
 * One missed output print statement.
 * Modified Files:
 * 	dbg/dump.c
 *
 * Revision 1.7  2003/09/25 22:52:22  shiv
 * Made output consistent for all types of leaks detected.
 * Modified Files:
 * 	dbg/dump.c
 *
 * Revision 1.6  2003/09/25 19:06:49  shiv
 * Fixed several things in the malloc code including the
 * leak detection for both small and large blocks. re-arranged
 * a lot code, and removed pieces that were not necessary after the
 * re-org. Modified the way in which the elf sections are read to
 * determine where heap references could possibly be stored.
 * set the optimisation for the debug variant at -O0, just so
 * so that debugging the lib itself is a little easier.
 * Modified Files:
 * 	common.mk mallocint.h dbg/dump.c dbg/malloc_chk.c
 * 	dbg/malloc_chn.c dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 * 	dbg/process.c dbg/string.c
 *
 * Revision 1.5  2003/09/22 16:59:55  shiv
 * Some fixes for the way the leak detection is done and reported.
 * Modified Files:
 * 	dbg/dump.c dbg/malloc_chk.c dbg/mtrace.c
 *
 * Revision 1.4  2003/07/30 13:10:28  sebastien
 * Start fixing the memory leak detection (PR12920). This is
 * again a partial fix, which properly matches the structures and alignement
 * from the mainline library.
 *
 * One issue that remains is the scanning of "gaps" which are composed of
 * multiple blocks. The gap scanning algorithm has to be updated to allow that.
 *
 * Revision 1.3  2003/06/05 15:21:14  shiv
 * Some cleanup to match changes to the malloc code in
 * libc.
 *
 * Revision 1.2  2000/02/15 22:01:07  furr
 * Changed to a more reasonable heap marking algorithm for memory leak
 * detection.
 *   - won't blow the stack
 * Fixed up file and line printing in all dumps.
 *
 *  Modified Files:
 *  	mallocint.h dbg/dump.c dbg/malloc_g.c dbg/malloc_gc.c
 *  	dbg/process.c dbg/tostring.c test/memtest.C
 *
 * Revision 1.1  2000/01/31 19:03:30  bstecher
 * Create libmalloc.so and libmalloc_g.so libraries for everything. See
 * Steve Furr for details.
 *
 * Revision 1.1  2000/01/28 22:32:44  furr
 * libmalloc_g allows consistency checks and bounds checking of heap
 * blocks allocated using malloc.
 * Initial revision
 *
 *  Added Files:
 *  	Makefile analyze.c calloc_g.c crc.c dump.c free.c m_init.c
 *  	m_perror.c malloc-config.c malloc_chk.c malloc_chn.c
 *  	malloc_g.c malloc_gc.c mallopt.c memory.c process.c realloc.c
 *  	string.c tostring.c inc/debug.h inc/mallocint.h inc/tostring.h
 *  	inc/malloc_g/malloc inc/malloc_g/malloc.h
 *  	inc/malloc_g/prototypes.h test/memtest.C test/memtest.c
 *  	x86/Makefile x86/so/Makefile
 *
 * Revision 1.2  1996/08/18 21:00:20  furr
 * ÿ¡ÿ¡ÿ¡ÿ¡print the caller return address on errors
 *
 * Revision 1.1  1996/07/24 18:22:02  furr
 * Initial revision
 *
 * Revision 1.9  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.8  1991/12/04  09:23:36  cpcahil
 * several performance enhancements including addition of free list
 *
 * Revision 1.7  91/11/25  14:41:52  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.6  91/11/24  00:49:25  cpcahil
 * first cut at patch 4
 * 
 * Revision 1.5  90/08/29  21:22:37  cpcahil
 * miscellaneous lint fixes
 * 
 * Revision 1.4  90/05/11  00:13:08  cpcahil
 * added copyright statment
 * 
 * Revision 1.3  90/02/24  21:50:07  cpcahil
 * lots of lint fixes
 * 
 * Revision 1.2  90/02/24  17:27:48  cpcahil
 * changed $header to $Id to remove full path from rcs id string
 * 
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 * 
 */
