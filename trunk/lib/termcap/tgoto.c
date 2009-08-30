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
 * tgoto - given the cursor motion string cm, make up the string
 * for the cursor to go to (destcol, destline), and return the string.
 * Returns "OOPS" if something's gone wrong, or the string otherwise.
 */
char *tgoto(char *cm, int destcol, int destline) {
    int argno = 0, numval;
    int *dp = &destline;
    static char ret[24];
    int incr = 0;		/* set by %i flag */
    char *rp;

    for (rp = ret; *cm; cm++) {
	switch (*cm) {
	case '%':
	    switch (*++cm) {
	    case '+':
		if (dp == 0)
		    return "OOPS";
		*rp++ = *dp + *++cm;
		dp = (dp == &destline) ? &destcol : 0;
		break;

	    case '%':
		*rp++ = '%';
		break;

	    case 'i':
		incr = 1;
		break;

	    case 'd':
		numval = (argno++ == 0 ? destline : destcol) + incr;
		if (numval >= 100)
		    *rp++ = '0' + (numval / 100), numval %= 100;
		if (numval >= 10)
		    *rp++ = '0' + (numval / 10), numval %= 10;
		*rp++ = '0' + numval;
		break;
	    }

	    break;
	default:
	    *rp++ = *cm;
	}
    }
    *rp = 0;
    return ret;
}
