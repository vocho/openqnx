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





#include "freeze.h"

#include "lz.h"

/*
 * freeze stdin to stdout
 */

void freeze ()
{
	register u_short i, len, r, s;
	register short c;
	putchar((int)(magic_header[0]));
#ifdef COMPAT
	putchar((int)(magic_header[1] | 1));
#else
	putchar((int)(magic_header[1]));
#endif
	write_header();
	StartHuff();
	InitTree();
	s = 0;
	r = N - _F;
	for (i = s; i < r; i++)
		text_buf[i] = ' ';
	for (len = 0; len < _F && (c = getchar()) != EOF; len++)
		text_buf[r + len] = c;
	if(!topipe && text_buf[r] == magic_header[0] &&
		(text_buf[r + 1] ^ magic_header[1]) <= 1) {
		if (!quiet)
			fprintf(stderr, " already frozen ");
		exit_stat = 2;
		return;
	}

	in_count = len;
	for (i = 0; i <= _F; i++)
		InsertNode(r + i - _F);
	while (len != 0) {
		Get_Next_Match(r);

		if (match_length > len)
			match_length = len;

		if (match_length <= THRESHOLD) {
			match_length = 1;
			EncodeChar(text_buf[r]);
#ifdef DEBUG
			symbols_out ++;
			if (verbose)
				fprintf(stderr, "'%s'\n",
					pr_char(text_buf[r]));
#endif /* DEBUG */
		} else {
			register u_short orig_length, orig_position, oldchar;

/* This fragment (delayed coding, non-greedy) is due to ideas of
	Jan Mark Wams' <jms@cs.vu.nl> COMIC:
*/
			oldchar = text_buf[r];
			orig_length = match_length;
			orig_position = match_position;

			DeleteNode(s);
			Next_Char();
			Get_Next_Match(r);

			if (match_length > len) match_length = len;

			if (orig_length > match_length) {
				EncodeChar((u_short)
					(256 - THRESHOLD + orig_length));
				EncodePosition((u_short)orig_position);
#ifdef DEBUG
				match_position = orig_position;
#endif  /* DEBUG */
				match_length = orig_length - 1;
			} else {
				EncodeChar(oldchar);
#ifdef DEBUG
				symbols_out ++;
				if (verbose)
					fprintf(stderr, "'%s'\n",
						pr_char(oldchar));
#endif  /* DEBUG */
				EncodeChar(256 - THRESHOLD + match_length);
				EncodePosition(match_position);
			}
#ifdef DEBUG
			refers_out ++;
			if (verbose) {
				register short pos =
					(r - 1 - match_position) & (N - 1),
				len = match_length;
				fputc('"', stderr);
				for(;len;len--, pos++)
					fprintf(stderr, "%s",
						pr_char(text_buf[pos]));
				fprintf(stderr, "\"\n");
			}
#endif /* DEBUG */
		}
		for (i = 0; i < match_length &&
				(c = getchar()) != EOF; i++) {
			DeleteNode(s);
			text_buf[s] = c;
			if (s < _F - 1)
				text_buf[s + N] = c;
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			InsertNode(r);
		}

		in_count += i;
		if ((in_count > indicator_count) && !quiet) {
			fprintf(stderr, "%5ldK\b\b\b\b\b\b", in_count / 1024);
			fflush (stderr);
			indicator_count += indicator_threshold;
			indicator_threshold += 1024;
		}
		while (i++ < match_length) {
			DeleteNode(s);
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);
		}
	}
	EncodeChar((short)ENDOF);
#ifdef DEBUG
	symbols_out ++;
#endif
	EncodeEnd();
    /*
     * Print out stats on stderr
     */
    if(!quiet) {
#ifdef GATHER_STAT
	fprintf(stderr, "Average number of steps: ");
	prratio(stderr, node_steps, node_matches);
	fprintf(stderr, "\n");
#endif
#ifdef DEBUG
	fprintf( stderr,
		"%ld chars in, %ld codes (%ld bytes) out, freezing factor: ",
		in_count, symbols_out + refers_out, bytes_out);
	prratio( stderr, in_count, bytes_out );
	fprintf( stderr, "\n");
	fprintf( stderr, "\tFreezing as in compact: " );
	prratio( stderr, in_count-bytes_out, in_count );
	fprintf( stderr, "\n");
	fprintf( stderr, "\tSymbols: %ld; references: %ld.\n",
		symbols_out, refers_out);
#else /* !DEBUG */
	fprintf( stderr, "Freezing: " );
	prratio( stderr, in_count-bytes_out, in_count );
#endif /* DEBUG */
    }
    if(bytes_out >= in_count)    /* exit(2) if no savings */
	exit_stat = 2;
    return;
}
