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



/*
 *  lmf.h       Load Module Format
 *

 */
#ifndef __LMF_H_INCLUDED

#pragma pack(1)
struct _lmf_header {            /*  This preceeds each record defined below */
    char            rec_type,
                    zero1;
    short unsigned  data_nbytes,
                    spare;
};


struct _lmf_definition {        /*  Must be first record in load file       */
    short unsigned  version_no,
                    cflags,
                    cpu,
                    fpu,
                    code_index,
                    stack_index,
                    heap_index,
                    argv_index,
                    zero1[4];
    long            code_offset,
                    stack_nbytes,
                    heap_nbytes,
                    flat_offset,
                    unmapped_size,
                    zero2;
                    /* Variable length field of n longs starts here */
};


struct _lmf_data {              /*  Code or data record to load into memory */
    short unsigned  segment_index;
    long            offset;
                    /* Variable length field of n bytes starts here */
};


struct _lmf_seg_fixup {         /*  Segment fixup record                        */
    struct fixups {
        short unsigned  fixup_seg_index;
        long            fixup_offset;
} data[1];              /* May be n of these */
};


struct _lmf_linear_fixup {      /*  Segment fixup record                        */
    short unsigned  fixup_seg_index;
    long            fixup_offset[1];
};


struct _lmf_eof {
    char zero[6];
};


struct _lmf_resource {
    short unsigned resource_type;   /* 0 - usage messages */
    short unsigned zero[3];
};
#pragma pack()

/*
 *  Record types
 */

#define _LMF_DEFINITION_REC     0
#define _LMF_COMMENT_REC        1
#define _LMF_DATA_REC           2
#define _LMF_FIXUP_SEG_REC      3
#define _LMF_FIXUP_80X87_REC    4
#define _LMF_EOF_REC            5
#define _LMF_RESOURCE_REC       6
#define _LMF_ENDDATA_REC        7
#define _LMF_FIXUP_LINEAR_REC   8
#define _LMF_PHRESOURCE         9       /* A widget resource for photon apps */


/*
 *  Bit defitions for lh_code_flags
 */

#define _PCF_LONG_LIVED     0x0001
#define _PCF_32BIT          0x0002
#define _PCF_PRIVMASK       0x000c   /* Two bits */
#define _PCF_FLAT           0x0010
#define _PCF_NOSHARE        0x0020

/*
 *  The top 4 bits of the segment sizes
 */

#define _LMF_CODE           0x2

#define __LMF_H_INCLUDED
#endif
