/*
 * CE_MK_HK(HEADER_BEGIN)
 */



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




#ifndef __QNXNTO__
#include <lib/compat.h>
#endif

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x)	x
#define PLATFORM(x)		<_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define SYSPAGE_TARGET_ALL
#define volatile	/* since we're working with a static copy */
#include _NTO_HDR_(sys/syspage.h)
#undef volatile

#include _NTO_HDR_(sys/trace.h)
/* 
 This library module provides this header, so we don't need to
 do a header install or the Neutrino header specification since 
 it will always be grabbed from the local structure first.
*/
#include <sys/traceparser.h>

#if defined(__NT__) || defined(__MINGW32__)
#if defined(__NT__)
	typedef _Uint64t		uint64_t;
#endif
	#define TF_OPEN_BITS	(O_RDONLY|O_BINARY)
	#include <io.h>
#else
	#define TF_OPEN_BITS	O_RDONLY
#endif

#include _NTO_HDR_(confname.h)

#ifndef EOK
#define EOK 0
#endif


/*
 *  traceparser - Parse the binary event file, providing callbacks to
 *  calling routines. This is a library of functions to be used by the
 *  caller.
 *
 *  Usage notes:
 *
 *   - Initialize traceparser state:     traceparser_init(...)
 *   - Setup debug mode:                 traceparser_debug(...)
 *   - Setup callbacks [in range]:       traceparser_cs[_range](...)
 *   - Call traceparser:                 traceparser(...)
 *   - Destroy (clr.) traceparser state: traceparser_destroy(...)
 *
 *   - Get header/state info-fields:     traceparser_get_info(...)
 *
 */

/* version */
#define _TRACEPARSER_VER_MAJOR  1
#define _TRACEPARSER_VER_MINOR  02

/* tracelogger compatibility version */
#define _TRACELOGGER_COMPAT_VER_MAJOR 1
#define _TRACELOGGER_COMPAT_VER_MINOR 01

/* tracefile header */
#if defined(_TRACE_MK_HK)
#undef _TRACE_MK_HK
#endif
#define _TRACE_MK_HK(k)  _TRACE_HEADER_PREFIX #k _TRACE_HEADER_POSTFIX

/* Link structure for assembling combined events */
typedef struct link_event {
	traceevent_t       event;
	struct link_event* cont;
	struct link_event* next;
} link_event_t;

/* global keyword definition */
static const char* const g_head_keywords[] = {
	_TRACE_HEADER_KEYWORDS()
};

/* constants */
#define _TP_H_ARR_SIZE   (sizeof(g_head_keywords)/sizeof(char*))
#define _TP_ARG_THR      (256U)
#define _TP_EMIT         (1U)
#define _TP_BLOCK        (0U)

/* structure mapping attributes */
typedef struct traceparser_attribute {
	struct traceparser_attribute *next;
	char 			 *key;
	char			 *value;
	int				 valuelen;
} traceparser_attribute_t;

/* state of the traceparser module */
typedef struct traceparser_state {
	link_event_t*       queue;
	traceparser_attribute_t	*attributes;
	tracep_callb_func_t callbacks[_TRACE_TOT_CLASS_NUM][_TRACE_MAX_EVENT_NUM];
	void*               user_data[_TRACE_TOT_CLASS_NUM][_TRACE_MAX_EVENT_NUM];
	void*               single_user_data;
	void*               syspage;
	FILE*               debug_stream;
	unsigned            debug_flags;
	unsigned            endian_conv;
	unsigned            now_callback_class;
	unsigned            now_callback_event;
	unsigned            last_callback_class;
	unsigned            last_callback_event;
	int                 last_callback_return;
	int                 file_des;
	traceparser_error_t error;
} traceparser_state_t;

/* prn error  used only inside local scope functions */
#define _TP_ERROR(s) \
if(tps_p->debug_flags&_TRACEPARSER_DEBUG_ERRORS&&tps_p->debug_stream) { \
	(void) fprintf(tps_p->debug_stream, "TRACEPARSER: %s\n", s); \
}
#define _TP_ALLERROR(s) \
if ((tps_p->debug_flags&_TRACEPARSER_DEBUG_ALL)==_TRACEPARSER_DEBUG_ALL) { \
	_TP_ERROR(s); \
}

/* endian swap */
#define _TRACE_SWAP_32BITS(l) \
       (((uint32_t)(l)&0x000000ff)<<24| \
        ((uint32_t)(l)&0x0000ff00)<<8 | \
        ((uint32_t)(l)&0x00ff0000)>>8 | \
        ((uint32_t)(l)&0xff000000)>>24)
#define _TRACE_SWAP_16BITS(l) \
        ((((uint16_t)(l)&0x00ff)<<8)| \
        (uint16_t)(((uint16_t)(l)&0xff00)>>8))
#define SWAP_64BITS(l)   swap_64bits(l)

/* cond swap */
#define CS16(c,l)         ((c)?_TRACE_SWAP_16BITS(l):(l))
#define CS32(c,l)         ((c)?_TRACE_SWAP_32BITS(l):(l))
#define CS64(c,l)         ((c)?       SWAP_64BITS(l):(l))

/* syspage support */
#define _TP_SYSPAGE_ENTRY(c,b,f) \
	((struct f##_entry*)((char*)(b)+CS16(c,(b)->f.entry_off)))
#define _TP_SYSPAGE_CPU_ENTRY(a,b,c,f) \
	((struct c##_##f##_entry*)((char*)(b)+CS16(a,(b)->un.c.f.entry_off)))
#define _TP_CK_ENTRY_OFF(c,p,e) \
	(CS16((c),(p)->e.entry_off)+CS16((c),p->e.entry_size)<=CS16((c),p->total_size))
#define _TP_PRINT_SYS_INFO(s,c,e) \
if (s) { \
	(void) fprintf(s," "#e":off/size=%d,%d",CS16(c,p->e.entry_off),CS16(c,p->e.entry_size)); \
}
#define _TP_PRINT_HW_FL(s,a,p,c) \
if (s) { \
	(void) fprintf(s," hwflags=%#08lx\n",(long)CS32(a,_TP_SYSPAGE_CPU_ENTRY(a,p,c,boxinfo)->hw_flags)); \
}

#define _TP_CLOSE_FILE(t) \
if ((t)&&(t)->file_des!=(-1)) { \
	(void) close((t)->file_des); \
	(t)->file_des=(-1); \
}

/*
 *  "No memory" handler
 */
static int nomem(traceparser_state_t* tps_p)
{
	if (tps_p) {
		_TP_CLOSE_FILE(tps_p);
		tps_p->error    = _TRACEPARSER_NO_MORE_MEMORY;
	}
	_TP_ERROR("no more memory");

	return (-1);
}

/*
 * Initializes traceparser state
 */
traceparser_state_t* traceparser_init(traceparser_state_t* tps_p)
{
	
	if (tps_p) {
		(void) memset((void*) tps_p, 0, sizeof(traceparser_state_t));
	} else if ((tps_p=(traceparser_state_t*) malloc(sizeof(traceparser_state_t)))==NULL) {
		(void) nomem(NULL);

		return (NULL);
	}
	(void) memset((void*) tps_p, 0, sizeof(traceparser_state_t));
	tps_p->file_des = (-1);

	return (tps_p);
}

/*
 * Destroys traceparser state structure
 */
void traceparser_destroy(traceparser_state_t** tps_pp)
{
	if(tps_pp&&*tps_pp)
	{
		_TP_CLOSE_FILE(*tps_pp);
		free((void*) *tps_pp);
		*tps_pp = NULL;
	}
}

static char *get_attribute_value(traceparser_state_t* tps_p, const char *key, int raw, unsigned *l_p) {
	traceparser_attribute_t *attr;
	char *rawkey;

	if(raw) {
		rawkey = (char *)key;
	} else {
		rawkey = alloca(strlen(key) + 1);
		strcpy(rawkey, key + strlen(_TRACE_HEADER_PREFIX));
		rawkey[strlen(rawkey) - strlen(_TRACE_HEADER_POSTFIX)] = '\0';
	}

	for(attr = tps_p->attributes; attr != NULL; attr = attr->next) {
		if(strcmp(attr->key, rawkey) == 0) {
			if(l_p) {
				*l_p = attr->valuelen;
			}
			return attr->value;
		}
	}
	
	return NULL;
}

static char *get_header_value(traceparser_state_t* tps_p, int id, unsigned *l_p) {
	return get_attribute_value(tps_p, g_head_keywords[id], 0, l_p);
}

/*
 * Gets header/keyword/endian info
 */
void* traceparser_get_info(traceparser_state_t* tps_p, info_modes_t i_m, unsigned* l_p)
{
	/* check if state structure is ok */
	if(tps_p==NULL) {
		errno = (EINVAL);

		return (NULL);
	}

	switch(i_m) {
		case _TRACEPARSER_INFO_SYSPAGE:
		{
			get_header_value(tps_p, _TRACEPARSER_INFO_SYSPAGE_LEN, l_p);

			return (tps_p->syspage);
		}
		case _TRACEPARSER_INFO_ENDIAN_CONV:
		{
			if (l_p) *l_p = sizeof(tps_p->endian_conv);

			return ((void*) &tps_p->endian_conv);
		}
		case _TRACEPARSER_INFO_PREV_CALLBACK_CLASS:
		{
			if (l_p) *l_p = sizeof(tps_p->last_callback_class);

			return ((void*) &tps_p->last_callback_class);
		}
		case _TRACEPARSER_INFO_PREV_CALLBACK_EVENT:
		{
			if (l_p) *l_p = sizeof(tps_p->last_callback_event);

			return ((void*) &tps_p->last_callback_event);
		}
		case _TRACEPARSER_INFO_PREV_CALLBACK_RETURN:
		{
			if (l_p) *l_p = sizeof(tps_p->last_callback_return);

			return ((void*) &tps_p->last_callback_return);
		}
		case _TRACEPARSER_INFO_NOW_CALLBACK_CLASS:
		{
			if (l_p) *l_p = sizeof(tps_p->now_callback_class);

			return ((void*) &tps_p->now_callback_class);
		}
		case _TRACEPARSER_INFO_NOW_CALLBACK_EVENT:
		{
			if (l_p) *l_p = sizeof(tps_p->now_callback_event);

			return ((void*) &tps_p->now_callback_event);
		}
		case _TRACEPARSER_INFO_DEBUG:
		{
			if (l_p) *l_p = sizeof(tps_p->debug_flags);

			return ((void*) &tps_p->debug_flags);
		}
		case _TRACEPARSER_INFO_ERROR:
		{
			if (l_p) *l_p = sizeof(tps_p->error);

			return ((void*) &tps_p->error);
		}
		default:
		{
			if (i_m>=_TRACEPARSER_INFO_HEADER_BEGIN&&i_m<=_TRACEPARSER_INFO_HEADER_END) {
				void* b_p;

				b_p = get_header_value(tps_p, i_m, l_p);
				if(b_p == NULL) {
					errno        = (EINVAL);
					tps_p->error = _TRACEPARSER_MISSING_FIELD;
				}

				return b_p;

			} else {
				errno        = EINVAL;
				tps_p->error = _TRACEPARSER_WRONG_H_KEYWORD;

				return (NULL);
			}
		}
	}
}

/*
 *  Locally adds one callback function to an internal class
 */
static int local_tracep_cs
(
 traceparser_state_t* tps_p,
 void*                u_d,
 tracep_callb_func_t  f,
 unsigned             c,
 unsigned             e
)
{
	if ((c>>10)<_TRACE_TOT_CLASS_NUM && e<_TRACE_MAX_EVENT_NUM) {
		tps_p->callbacks[c>>10][e] = f;
		tps_p->user_data[c>>10][e] = u_d;

		return (EOK);
	} else {
		errno        = EINVAL;
		tps_p->error = _TRACEPARSER_UNKNOWN_ERROR;

		return (-1);
	}
}

/*
 *  Locally setups a range of callback functions
 */
static int local_tracep_cs_r
(
 traceparser_state_t* tps_p,
 void*                u_d,
 tracep_callb_func_t  f,
 unsigned             c,
 unsigned             e_r1,
 unsigned             e_r2
)
{
	if ((c>>10) >= _TRACE_TOT_CLASS_NUM) {
		errno = EINVAL;
		tps_p->error = _TRACEPARSER_UNKNOWN_ERROR;

		return (-1);
	}

	if (e_r1<=e_r2 && e_r2<_TRACE_MAX_EVENT_NUM) {
		unsigned e;

		for (e=e_r1; e<=e_r2; ++e) {
			tps_p->callbacks[c>>10][e] = f;
			tps_p->user_data[c>>10][e] = u_d;
		}

		return (EOK);
	} else {
		errno        = EINVAL;
		tps_p->error = _TRACEPARSER_UNKNOWN_ERROR;

		return (-1);
	}
}

/*
 *  Finds rightmost bit location
 */
static unsigned ck_bit(uint32_t k)
{
	if(k) {
		unsigned s=0U;

		while (!(k&0x1)) {
			++s;
			k >>= 1;
		}

		return (s);
	} else {
		return (unsigned)(-1);
	}
}

/*
 *  Used by the caller to setup user data and one callback function
 */
int traceparser_cs(traceparser_state_t* tps_p, void* u_d, tracep_callb_func_t f_p, unsigned c, unsigned e)
{
	/* check if state structure is ok */
	if(tps_p==NULL) {
		errno = (EINVAL);

		return (-1);
	}

	switch(c)
	{
		case _NTO_TRACE_EMPTY:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_EMPTY_C, e));
		case _NTO_TRACE_CONTROL:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_CONTROL_C, e));
		case _NTO_TRACE_KERCALL:
			if (local_tracep_cs(tps_p, u_d, f_p, _TRACE_KER_CALL_C, e) ||
			    local_tracep_cs(tps_p, u_d, f_p, _TRACE_KER_CALL_C, (e+_TRACE_MAX_KER_CALL_NUM))) {
				errno = EINVAL;

				return (-1);
			} else {
				return (EOK);
			}
		case _NTO_TRACE_KERCALLENTER:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_KER_CALL_C, e));
		case _NTO_TRACE_KERCALLEXIT:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_KER_CALL_C, (e+_TRACE_MAX_KER_CALL_NUM)));
		case _NTO_TRACE_KERCALLINT:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_KER_CALL_C, (e+(2*_TRACE_MAX_KER_CALL_NUM))));
		case _NTO_TRACE_INT:
			errno = EINVAL;

			return (-1);
		case _NTO_TRACE_INTENTER:
			errno = EINVAL;

			return (-1);
		case _NTO_TRACE_INTEXIT:
			errno = EINVAL;

			return (-1);
		case _NTO_TRACE_PROCESS:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_PR_TH_C, (ck_bit(e)+1)<<6));
		case _NTO_TRACE_THREAD:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_PR_TH_C, ck_bit(e)));
		case _NTO_TRACE_VTHREAD:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_PR_TH_C, (ck_bit(e)+_TRACE_MAX_TH_STATE_NUM)));
		case _NTO_TRACE_SYSTEM:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_SYSTEM_C, e));
		case _NTO_TRACE_USER:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_USER_C, e));
		case _NTO_TRACE_COMM:
			return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_COMM_C, e));
		default:
			errno = EINVAL;

			return (-1);
	}
}

/*
 *  Used by the caller to setup a range of user data and callback functions
 */
int traceparser_cs_range
(
 traceparser_state_t* tps_p,
 void*                u_d,
 tracep_callb_func_t  f_p,
 unsigned             c,
 unsigned             e_r1,
 unsigned             e_r2
)
{
	/* check if state structure is ok */
	if(tps_p==NULL) {
		errno = (EINVAL);

		return (-1);
	}

	switch(c)
	{
		case _NTO_TRACE_EMPTY:
			return (local_tracep_cs_r(tps_p, u_d, f_p, _TRACE_EMPTY_C, e_r1, e_r2));
		case _NTO_TRACE_CONTROL:
			return (local_tracep_cs_r(tps_p, u_d, f_p, _TRACE_CONTROL_C, e_r1, e_r2));
		case _NTO_TRACE_KERCALL:
			if (local_tracep_cs_r(tps_p, u_d, f_p, _TRACE_KER_CALL_C, e_r1, e_r2) ||
			    local_tracep_cs_r
			    (
					 tps_p,
					 u_d,
			     f_p,
			     _TRACE_KER_CALL_C,
			     (e_r1+_TRACE_MAX_KER_CALL_NUM),
			     (e_r2+_TRACE_MAX_KER_CALL_NUM)
			    )) {
				errno = EINVAL;

				return (-1);
			} else {
				return (EOK);
			}
		case _NTO_TRACE_KERCALLENTER:
			return (local_tracep_cs_r(tps_p, u_d, f_p, _TRACE_KER_CALL_C, e_r1, e_r2));
		case _NTO_TRACE_KERCALLEXIT:
			return (local_tracep_cs_r
			       (
					 		tps_p,
			        u_d,
			        f_p,
			        _TRACE_KER_CALL_C,
			        (e_r1+_TRACE_MAX_KER_CALL_NUM),
			        (e_r2+_TRACE_MAX_KER_CALL_NUM)
			       ));
		case _NTO_TRACE_KERCALLINT:
			return (local_tracep_cs_r
			       (
					 		tps_p,
			        u_d,
			        f_p,
			        _TRACE_KER_CALL_C,
			        (e_r1+(2*_TRACE_MAX_KER_CALL_NUM)),
			        (e_r2+(2*_TRACE_MAX_KER_CALL_NUM))
			       ));
		case _NTO_TRACE_INT:
			if (e_r1==_NTO_TRACE_INTFIRST&&e_r2==_NTO_TRACE_INTLAST) {
				if (local_tracep_cs(tps_p, u_d, f_p, _TRACE_INT_C, _TRACE_INT_ENTRY) ||
				    local_tracep_cs(tps_p, u_d, f_p, _TRACE_INT_C, _TRACE_INT_EXIT)  ||
					local_tracep_cs(tps_p, u_d, f_p, _TRACE_INT_C, _TRACE_INT_HANDLER_ENTRY) ||
				    local_tracep_cs(tps_p, u_d, f_p, _TRACE_INT_C, _TRACE_INT_HANDLER_EXIT)) {
					errno = EINVAL;

					return (-1);
				} else {
					return (EOK);
				}
			} else {
				errno = EINVAL;

				return (-1);
			}
		case _NTO_TRACE_INTENTER:
			if (e_r1==_NTO_TRACE_INTFIRST&&e_r2==_NTO_TRACE_INTLAST) {
				return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_INT_C, _TRACE_INT_ENTRY));
			} else {
				errno = EINVAL;

				return (-1);
			}
		case _NTO_TRACE_INTEXIT:
			if (e_r1==_NTO_TRACE_INTFIRST&&e_r2==_NTO_TRACE_INTLAST) {
				return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_INT_C, _TRACE_INT_EXIT));
			} else {
				errno = EINVAL;

				return (-1);
			}
		case _NTO_TRACE_INT_HANDLER_ENTER:
			if (e_r1==_NTO_TRACE_INTFIRST&&e_r2==_NTO_TRACE_INTLAST) {
				return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_INT_C, _TRACE_INT_HANDLER_ENTRY));
			} else {
				errno = EINVAL;

				return (-1);
			}
		case _NTO_TRACE_INT_HANDLER_EXIT:
			if (e_r1==_NTO_TRACE_INTFIRST&&e_r2==_NTO_TRACE_INTLAST) {
				return (local_tracep_cs(tps_p, u_d, f_p, _TRACE_INT_C, _TRACE_INT_HANDLER_EXIT));
			} else {
				errno = EINVAL;

				return (-1);
			}
		case _NTO_TRACE_PROCESS:
			return (local_tracep_cs_r(tps_p, u_d, f_p, _TRACE_PR_TH_C, (ck_bit(e_r1)+1)<<6, (ck_bit(e_r2)+1)<<6));
		case _NTO_TRACE_THREAD:
			return (local_tracep_cs_r(tps_p, u_d, f_p, _TRACE_PR_TH_C, ck_bit(e_r1), ck_bit(e_r2)));
		case _NTO_TRACE_VTHREAD:
			return (local_tracep_cs_r
			       (
					 		tps_p,
			        u_d,
			        f_p,
			        _TRACE_PR_TH_C,
			        (ck_bit(e_r1)+_TRACE_MAX_TH_STATE_NUM),
			        (ck_bit(e_r2)+_TRACE_MAX_TH_STATE_NUM)
			       ));
		case _NTO_TRACE_SYSTEM:
			return (local_tracep_cs_r(tps_p, u_d, f_p, _TRACE_SYSTEM_C, e_r1, e_r2));
		case _NTO_TRACE_USER:
			return (local_tracep_cs_r(tps_p, u_d, f_p, _TRACE_USER_C, e_r1, e_r2));
		case _NTO_TRACE_COMM:
			return (local_tracep_cs_r(tps_p, u_d, f_p, _TRACE_COMM_C, e_r1, e_r2));
		default:
		{
			errno = EINVAL;

			return (-1);
		}
	}
}

/*
 * Used by the caller to setup debug mode
 */
int traceparser_debug(struct traceparser_state* tps_p, FILE* s, unsigned f)
{
	/* check if state structure and FILE are ok */
	if((tps_p==NULL) || (s==NULL)) {
		errno = (EINVAL);

		return (-1);
	}

	tps_p->debug_stream = s;
	tps_p->debug_flags  = f;

	return (0);
}

/*
 *  Decodes the input event[stream]
 */
static int decode(traceparser_state_t* tps_p, unsigned h, unsigned t, unsigned* b_p, unsigned b_s)
{
	unsigned i=_NTO_TRACE_GETEVENT_C(h)>>10;
	unsigned j=_NTO_TRACE_GETEVENT(h);

	if(i < _TRACE_TOT_CLASS_NUM && j < _TRACE_MAX_EVENT_NUM) {

		if(tps_p->callbacks[i][j]) {
			tps_p->now_callback_class = i;
			tps_p->now_callback_event = j;
			if(tps_p->user_data[i][j]) {
				tps_p->last_callback_return = (*tps_p->callbacks[i][j])(tps_p, tps_p->user_data[i][j], h, t, b_p, b_s); 
			} else {
				tps_p->last_callback_return = (*tps_p->callbacks[i][j])(tps_p, tps_p->single_user_data, h, t, b_p, b_s); 
			}
			tps_p->last_callback_class = i;
			tps_p->last_callback_event = j;
		}

		return (0);
	} else {
		return (-1);
	}
}

/*
 *  Finishes processing pending "trace events"
 */
static void finish(traceparser_state_t* tps_p)
{
	(void) fflush(tps_p->debug_stream);

	return;
}

/*
 *  Processes one "simple trace event"
 */
static int simple(traceparser_state_t* tps_p, traceevent_t* t_e_p)
{
	/*                   FIX_ME!!
	 *   #if 0 ... #endif
	 *   One unballanced combine event will block the queue.
	 *   A mechanism to automatically empty events is needed.
	 */
#if 0
	if (tps_p->queue) {
		link_event_t* l_e_p;

		for (l_e_p=tps_p->queue; l_e_p->next; l_e_p=l_e_p->next);
		if ((l_e_p->next=(link_event_t*) malloc(sizeof(link_event_t)))==NULL) nomem();
		l_e_p        = l_e_p->next;
		l_e_p->event = *t_e_p;
		l_e_p->cont  = NULL;
		l_e_p->next  = NULL;
	} else
#endif
	{
		(void) decode(tps_p, t_e_p->header, t_e_p->data[0], (unsigned*) &t_e_p->data[1], 2U);
	}

	return (0);
}

/*
 *  Processes one "combine trace event"
 */
#define AVG_NUM_INTS 20
static int combine(traceparser_state_t* tps_p, const traceevent_t* t_e_p)
{
	link_event_t* l_e_p;

	if (_TRACE_GET_STRUCT(t_e_p->header)==_TRACE_STRUCT_CC) {
		if (tps_p->queue) {
			for (l_e_p=tps_p->queue; l_e_p; l_e_p=l_e_p->next) {
				if (l_e_p->event.data[0]==t_e_p->data[0]) {
					for ( ; l_e_p->cont; l_e_p=l_e_p->cont);
					if ((l_e_p->cont = (link_event_t*) malloc(sizeof(link_event_t)))==NULL) {
						return (nomem(tps_p));
					}

					l_e_p        = l_e_p->cont;
					l_e_p->event = *t_e_p;
					l_e_p->cont  = NULL;
					l_e_p->next  = NULL;
					break;
				}
			}
		} else {
			_TP_ALLERROR("unbalanced continuation of combine event ignored");
		}
	} else if (_TRACE_GET_STRUCT(t_e_p->header)==_TRACE_STRUCT_CB) {
		if (tps_p->queue) {
			for (l_e_p=tps_p->queue; l_e_p->next; l_e_p=l_e_p->next);
			if ((l_e_p->next = (link_event_t*) malloc(sizeof(link_event_t)))==NULL) {
				return (nomem(tps_p));
			}

			l_e_p        = l_e_p->next;
			l_e_p->event = *t_e_p;
			l_e_p->cont  = NULL;
			l_e_p->next  = NULL;
		} else {
			if ((tps_p->queue = (link_event_t*) malloc(sizeof(link_event_t)))==NULL) {
				return (nomem(tps_p));	
			}

			tps_p->queue->event = *t_e_p;
			tps_p->queue->cont  = NULL;
			tps_p->queue->next  = NULL;
		}
	} else if (tps_p->queue) {
		unsigned       head;
		unsigned       time;
		link_event_t*  l_e_p2   = NULL;
		unsigned       state    = _TP_EMIT;

		for (l_e_p=tps_p->queue; l_e_p; l_e_p2=l_e_p,l_e_p=l_e_p->next) {
			if (state==_TP_EMIT&&_TRACE_GET_STRUCT(l_e_p->event.header)==_TRACE_STRUCT_S) {
				tps_p->queue = l_e_p->next;
				l_e_p2  = NULL;
				(void) decode(tps_p, l_e_p->event.header, l_e_p->event.data[0], (unsigned*) &l_e_p->event.data[1], 2U);
				free(l_e_p);
			} else if (_TRACE_GET_STRUCT(l_e_p->event.header)!=_TRACE_STRUCT_S) {
				__traceentry   stackbuff[AVG_NUM_INTS];
				__traceentry   *buff;
				int			   buffcnt;

				buffcnt = AVG_NUM_INTS;
				buff = stackbuff;

				state = _TP_BLOCK;
				if (l_e_p->event.data[0]==t_e_p->data[0]) {
					if (l_e_p2) {
						l_e_p2->next = l_e_p->next;
					} else {
						tps_p->queue = l_e_p->next;
					}
					head  = l_e_p->event.header;
					time  = l_e_p->event.data[0];
					state = 0;
					while (l_e_p) {
						//Need to have room for our two guys outside the loop
						if((state + 1) >= buffcnt - 2) {
							__traceentry *nbuff;
							buffcnt = buffcnt + (buffcnt / 2);
							nbuff = realloc((buff == stackbuff) ? NULL : buff, buffcnt * sizeof(*buff));
							if(nbuff == NULL) {
								break;	//Out of the while loop with what we have
							}	
							if ( buff == stackbuff ) {
								memcpy( nbuff, stackbuff, sizeof(stackbuff));
							}
							buff = nbuff; 
						}
						buff[state++] = l_e_p->event.data[1];
						buff[state++] = l_e_p->event.data[2];
						l_e_p2 = l_e_p;
						l_e_p  = l_e_p->cont;
						free(l_e_p2);
					}
					buff[state++] = t_e_p->data[1];
					buff[state++] = t_e_p->data[2];
					(void) decode(tps_p, head, time, (unsigned*) buff, state);
		
					if(buff != stackbuff) {
						free(buff);
					}

					break;
				}
			}
		}

	} else {
		_TP_ALLERROR("unbalanced end of combine event ignored");
	}

	return (0);
}

#if !defined(__WATCOMC__) || (__WATCOMC__ >= 1100)
/*
 *  64-bit endian swap routine
 */
static uint64_t swap_64bits(uint64_t l)
{
	register union u {
		uint64_t l;
		uint32_t s[2];
	} u;

	u.s[0] = _TRACE_SWAP_32BITS(((union u*)&l)->s[1]);
	u.s[1] = _TRACE_SWAP_32BITS(((union u*)&l)->s[0]);

	return (u.l);
}
#endif

/*
 *  32-bit endian swap routine
 */
static void arr_swap32(long* a, unsigned s)
{
	register long r;

	while(s--) {
		r    = a[s];
		a[s] = _TRACE_SWAP_32BITS(r);
	}

	return;
}

static void print_64(FILE *stream, unsigned s, const char *label, uint64_t *v)
{
	#if defined(__WATCOMC__) && (__WATCOMC__ < 1100)
		/*KLUDGE: Don't have 64 bit integer support*/
		fprintf(stream, "%s0x%4.4x%4.4x", label,
				((uint32_t *)v)[s==0], ((uint32_t *)v)[s!=0]);
	#else
		fprintf(stream, "%s%llu", label, CS64(s,*v));
	#endif
}

/*
 *  Prints to "stream" entries of the syspage
 */
static void print_syspage(FILE* stream, struct syspage_entry* p, unsigned s)
{
	static const char *cpu[]={
		"X86", "PPC", "MIPS", "unknown(3)", "ARM", "SH"
	};

	if(stream==NULL) {
		return;
	}

	(void) fprintf
	(
	 stream,
	 " size=%d total_size=%d",
		 CS16(s, p->size),
		 CS16(s, p->total_size)
	);
	_TP_PRINT_SYS_INFO(stream, s, system_private);
	(void) fprintf(stream, "\n");
	_TP_PRINT_SYS_INFO(stream, s, asinfo);
	_TP_PRINT_SYS_INFO(stream, s, meminfo);
	_TP_PRINT_SYS_INFO(stream, s, hwinfo);
	(void) fprintf(stream, "\n");
	_TP_PRINT_SYS_INFO(stream, s, cpuinfo);
	_TP_PRINT_SYS_INFO(stream, s, cacheattr);
	_TP_PRINT_SYS_INFO(stream, s, qtime);
	(void) fprintf(stream, "\n");
	_TP_PRINT_SYS_INFO(stream, s, callout);
	_TP_PRINT_SYS_INFO(stream, s, callin);
	_TP_PRINT_SYS_INFO(stream, s, intrinfo);
	(void) fprintf(stream, "\n");
	_TP_PRINT_SYS_INFO(stream, s, typed_strings);
	_TP_PRINT_SYS_INFO(stream, s, strings);
	(void) fprintf(stream, "\n");

	/* printing processor type */
	(void) fprintf(stream, " processor=");
	if(CS16(s, p->type)>=sizeof(cpu)/sizeof(char*)||!cpu[CS16(s, p->type)]) {
		(void) fprintf(stream, "Unknown(%d)", CS16(s, p->type));
	} else {
		(void) fprintf(stream, "%s", cpu[CS16(s, p->type)]);
	}

	/* printing cpuinfo */
	(void) fprintf(stream, " num_cpus=%d\n", CS16(s, p->num_cpu));
	if(CS16(s, p->cpuinfo.entry_off) + CS16(s, p->cpuinfo.entry_size) <=
	   CS16(s, p->total_size)) {
		int   i;
		char* s_p=_TP_CK_ENTRY_OFF(s, p, strings)?_TP_SYSPAGE_ENTRY(s, p, strings)->data:NULL;

		for(i=0;s_p&&i<CS16(s, p->num_cpu);++i) {
			struct cpuinfo_entry* c=_TP_SYSPAGE_ENTRY(s, p, cpuinfo)+i;
		
			(void) fprintf
			(
			 stream,
			 "  cpu %d cpu=%d name=%s speed=%d\n",
			 i+1,
			 CS32(s, c->cpu),
			 s_p?s_p+CS16(s, c->name):"",
			 CS32(s, c->speed)
			);
			(void) fprintf(stream, "  flags=%#08x", (uint32_t) CS32(s, c->flags));
			if(CS32(s, c->flags) & CPU_FLAG_FPU) (void) fprintf(stream, " FPU");
			if(CS32(s, c->flags) & CPU_FLAG_MMU) (void) fprintf(stream, " MMU");
			switch(CS16(s, p->type)) {
			case SYSPAGE_X86:
				if(CS32(s, c->flags) & X86_CPU_CPUID)  (void) fprintf(stream, " CPUID");
				if(CS32(s, c->flags) & X86_CPU_RDTSC)  (void) fprintf(stream, " RDTSC");
				if(CS32(s, c->flags) & X86_CPU_INVLPG) (void) fprintf(stream, " INVLPG");
				if(CS32(s, c->flags) & X86_CPU_WP)     (void) fprintf(stream, " WP");
				if(CS32(s, c->flags) & X86_CPU_BSWAP)  (void) fprintf(stream, " BSWAP");
				if(CS32(s, c->flags) & X86_CPU_MMX)    (void) fprintf(stream, " MMX");
				if(CS32(s, c->flags) & X86_CPU_CMOV)   (void) fprintf(stream, " CMOV");
				if(CS32(s, c->flags) & X86_CPU_PSE)    (void) fprintf(stream, " PSE");
				if(CS32(s, c->flags) & X86_CPU_PGE)    (void) fprintf(stream, " PGE");
				if(CS32(s, c->flags) & X86_CPU_MTRR)   (void) fprintf(stream, " MTRR");
				if(CS32(s, c->flags) & X86_CPU_SEP)    (void) fprintf(stream, " SEP");
				if(CS32(s, c->flags) & X86_CPU_SIMD)   (void) fprintf(stream, " SIMD");
				if(CS32(s, c->flags) & X86_CPU_FXSR)   (void) fprintf(stream, " FXSR");
				break;
			case SYSPAGE_PPC:
				if(CS32(s, c->flags) & PPC_CPU_EAR)    (void) fprintf(stream, " EAR");
				if(CS32(s, c->flags) & PPC_CPU_HW_HT)  (void) fprintf(stream, " HW_HT");
				if(CS32(s, c->flags) & PPC_CPU_HW_POW) (void) fprintf(stream, " HW_POW");
				if(CS32(s, c->flags) & PPC_CPU_FPREGS) (void) fprintf(stream, " FPREGS");
				if(CS32(s, c->flags) & PPC_CPU_SW_HT)  (void) fprintf(stream, " SW_HT");
				if(CS32(s, c->flags) & PPC_CPU_ALTIVEC)(void) fprintf(stream, " ALTIVEC");
				break;
			case SYSPAGE_MIPS:
				break;
			case SYSPAGE_SH:
				break;
			case SYSPAGE_ARM:
				break;
			default:
				break;
			}
			(void) fprintf(stream, "\n");
		}
	}

	/* printing time information */
	if(_TP_CK_ENTRY_OFF(s, p, qtime)) {
		struct qtime_entry* t=_TP_SYSPAGE_ENTRY(s, p, qtime);

		print_64(stream, s, " cyc/sec=", &t->cycles_per_sec);
		print_64(stream, s, " tod_adj=", &t->nsec_tod_adjust);
		print_64(stream, s, " nsec=", &t->nsec);
		(void) fprintf
		(
		 stream,
		 "  inc=%lu\n",
		 CS32(s, t->nsec_inc)
		);
		(void) fprintf
		(
		 stream,
		 " boot=%lu epoch=%lu intr=%ld\n",
		 CS32(s, t->boot_time),
		 CS32(s, t->epoch),
		 CS32(s, t->intr)
		);
		(void) fprintf
		(
		 stream,
		 " rate=%lu scale=%ld load=%lu\n",
		 CS32(s, t->timer_rate),
		 CS32(s, t->timer_scale),
		 CS32(s, t->timer_load)
		);
	}

	if(_TP_CK_ENTRY_OFF(s, p, typed_strings)) {
		struct entry {
			uint32_t type;
			char     info[1];
		}* e_p;
		int pos=0;

		for(e_p=(struct entry *)_TP_SYSPAGE_ENTRY(s, p, typed_strings)->data; e_p->type != _CS_NONE;
				e_p=(struct entry *)((char *)e_p + ((offsetof(struct entry, info) +
						strlen(e_p->info)+1+3) & ~3))) {
			char* type;
			char  buff[20];
			int   len;

			switch(CS32(s, e_p->type) & ~_CS_SET) {
			case _CS_HOSTNAME:     type = "HOSTNAME";     break;
			case _CS_RELEASE:      type = "RELEASE";      break;
			case _CS_VERSION:      type = "VERSION";      break;
			case _CS_MACHINE:      type = "MACHINE";      break;
			case _CS_ARCHITECTURE: type = "ARCHITECTURE"; break;
			case _CS_HW_SERIAL:    type = "HW_SERIAL";    break;
			case _CS_HW_PROVIDER:  type = "HW_PROVIDER";  break;
			case _CS_SRPC_DOMAIN:  type = "SRPC_DOMAIN";  break;
			case _CS_SYSNAME:      type = "SYSNAME";      break;
			case _CS_LIBPATH:      type = "LIBPATH";      break;
			case _CS_DOMAIN:       type = "DOMAIN";       break;
			case _CS_RESOLVE:      type = "RESOLVE";      break;
			case _CS_TIMEZONE:     type = "TIMEZONE";     break;
			default:
				(void) sprintf(type=buff, "type(%d)", (int)CS32(s, e_p->type) & ~_CS_SET);
				break;
			}

			len = strlen(type)+strlen(e_p->info)+4;		/* space = " " */
			if(pos && pos+len>75) (void) fprintf(stream, "\n"); pos = 0;
			if(pos == 0)          (void) fprintf(stream, "  ");
			(void) fprintf(stream, " %s=\"%s\"", type, e_p->info);
			pos += len;
		}
		if(pos) (void) fprintf(stream, "\n");
	}

	return;
}

/*
 *  Prints and checks for printable characters
 */
static void putprints(const char* s_p, FILE* out_stream)
{
	register const char* c_p=(s_p-1);

	while(*++c_p) if (isprint(*c_p)) putc(*c_p, out_stream);

	return;
}

/*
 * Find the occurrence of a set of bytes in another set of bytes.
 * This is the equivalent of strstr but without the null terminated
 * string limitation.  It returns a pointer to the match or NULL
 * if no match is found.
 */
static void *memfind(const void *b1, int b1len, const void *b2, int b2len) {
	unsigned offset = 0;
    unsigned char c;
	void *found;
	
	while(offset + b2len < b1len) {
		c = *((unsigned char *)b2);
		found = memchr(((unsigned char *)b1) + offset, c, b1len - offset);
		if(found == NULL) {
			return NULL;
		}
		if(memcmp(found, b2, b2len) == 0) {
			return found;
		}
		offset += ((uintptr_t)found - (uintptr_t)b1) + 1;
	}

	return NULL;
}


/*
 *  Reads header information from the file.  The header is in a format
 *  of:
 *  HEADER_BEGIN<prefix><key><postfix><value><prefix><key><postfix><value>...HEADER_END
 */
static int get_header(traceparser_state_t* tps_p, int fd)
{
	char buffer[100];	
	int  buflen, keylen;
	char *start, *key, *value;
	int  lastkeyoffset, syspageoffset;
	int  done, prefixlen, postfixlen, endlen;

	done = 0;
	lastkeyoffset = 0;
	syspageoffset = 0;
	prefixlen = strlen(_TRACE_HEADER_PREFIX);
	postfixlen = strlen(_TRACE_HEADER_POSTFIX);
	endlen = strlen(_TRACE_MK_HK(HEADER_END));

	//Locate the start of the header by finding the header
	lseek(fd, lastkeyoffset, SEEK_SET);	
	buflen = read(fd, buffer, sizeof(buffer) - 1);
	if(buflen <= 0) {
		return -1;
	}
	buffer[buflen] = '\0';

	start = strstr(buffer, _TRACE_MK_HK(HEADER_BEGIN));
	if(start == NULL) {
		return -1;
	}
	start = strstr(start, _TRACE_HEADER_POSTFIX) + postfixlen;

	//Seed the initial setting for the loop
	start = strstr(start, _TRACE_HEADER_PREFIX);
	if(start == NULL || ((start - buffer) + prefixlen) > buflen) {
		return -1;
	}
	start += prefixlen;
	lastkeyoffset += start - buffer;

	//Now go through and pull out PREFIXkeyPOSTFIX values
	//Going into this loop, start is assumed to point to the start of key
	do {
		do {
			traceparser_attribute_t *newattr;

			key = start;

			value = strstr(key, _TRACE_HEADER_POSTFIX);
			if(value == NULL || ((value - buffer) + postfixlen) > buflen) {
				break;
			}

			*value = '\0';
			keylen = value - key;
			value += postfixlen;

			//Need to use memfind here since the data could have '\0' chars
			start = memfind(value, buflen - (value - buffer), _TRACE_HEADER_PREFIX, prefixlen);
			if(start == NULL || (start - buffer) > buflen) {
				break;
			}

			//Before we clobber the entry, see if this is our magic end key
			if(strncmp(start, _TRACE_MK_HK(HEADER_END), endlen) == 0) {
				syspageoffset = lastkeyoffset + (start - key) + endlen;
				done = 1;
			}
			*start = '\0';

			newattr = calloc(1, sizeof(*newattr));
			newattr->key = strdup(key);
			newattr->valuelen = start - value;
			newattr->value = malloc(newattr->valuelen);
			memcpy(newattr->value, value, newattr->valuelen);

			newattr->next = tps_p->attributes;
			tps_p->attributes = newattr;

			if(tps_p->debug_stream) {
				if (tps_p->debug_flags & _TRACEPARSER_DEBUG_ALL) {
					fprintf(tps_p->debug_stream, "%*s%*s:: ", 22 - keylen, "TRACE_", keylen, key);
					if((tps_p->debug_flags & _TRACEPARSER_DEBUG_ALL) == _TRACEPARSER_DEBUG_ALL) {
						fprintf(tps_p->debug_stream, "(len=%d) -> \"%s\" \n", newattr->valuelen, newattr->value);
					} else {
						putprints(newattr->value, tps_p->debug_stream);
						fputc('\n', tps_p->debug_stream);
					}
				}
			}

			start += prefixlen;
			lastkeyoffset += start - key;

		} while(!done);

		//If we popped out because we ran out of buffer
		//go back to the last key we had found and read from there
		if(!done && start == buffer) {
			//We didn't make any further progress, bail out
			//We don't return "nicely" since this could be an error now.
			if(tps_p->debug_stream) {
				fprintf(tps_p->debug_stream, "Can't find header end, results may be invalid\n");
			}
			return -1;
		} else if(!done) {
			start = buffer;
			lseek(fd, lastkeyoffset, SEEK_SET);
			buflen = read(fd, start, sizeof(buffer) - 1);
			if(buflen <= 0) {
				break;
			}
			buffer[buflen] = '\0';
		}
	} while(!done);

	//The API of this call expects the fd to be left at the end of attributes
	lseek(fd, syspageoffset, SEEK_SET); 

	return (0);
}

/*
 * The main entry point of the library
 */
int traceparser(traceparser_state_t* tps_p, void* u_d, const char *tracefile)
{
	union  {long l; char c[sizeof(long)];} u={1};
	traceevent_t t_e;

	/* check if state structure is ok */
	if(tps_p==NULL) {
		errno = (EINVAL);

		return (-1);
	}

	/* set user data */
	tps_p->single_user_data  = u_d;

	/* initial info and openning the file */
	if (tps_p->debug_flags&_TRACEPARSER_DEBUG_HEADER&&tps_p->debug_stream)
	{
		(void) fprintf
		(
		 tps_p->debug_stream,
		 "TRACEPARSER LIBRARY version %d.%02d\n",
		 _TRACEPARSER_VER_MAJOR,
		 _TRACEPARSER_VER_MINOR
		);
		(void) fflush(tps_p->debug_stream);
	}
	
	if (tracefile[0] == '-' && tracefile[1] == '\0') {
		tps_p->file_des = fileno(stdin);
	} else {
		tps_p->file_des=open(tracefile, TF_OPEN_BITS);
	}
	if (tps_p->file_des == -1) {
		if(tps_p->debug_flags&_TRACEPARSER_DEBUG_ERRORS&&tps_p->debug_stream) {
			(void) fprintf
			(
			 tps_p->debug_stream,
			 "TRACEPARSER: couldn't open input trace file %s, errno: %s\n",
			 tracefile,
			 strerror(errno)
			);
		}
		errno        = EINVAL;
		tps_p->error =  _TRACEPARSER_CANNOT_READ_IN_FILE;

		return (-1);
	}

	/* reading header information */
	if(tps_p->debug_flags&_TRACEPARSER_DEBUG_HEADER&&tps_p->debug_stream) {
		(void) fprintf(tps_p->debug_stream, " -- HEADER FILE INFORMATION -- \n");
	}
	if (get_header(tps_p, tps_p->file_des)) return (-1);
	if(*u.c==1 && 
       get_header_value(tps_p, _TRACEPARSER_INFO_BIG_ENDIAN, NULL) ||
	   u.c[sizeof(long)-1]==1 && 
       get_header_value(tps_p, _TRACEPARSER_INFO_LITTLE_ENDIAN, NULL)) {
		tps_p->endian_conv = 1;
	} else if (get_header_value(tps_p, _TRACEPARSER_INFO_MIDDLE_ENDIAN, NULL) ||
	           get_header_value(tps_p, _TRACEPARSER_INFO_LITTLE_ENDIAN, NULL) && 
               get_header_value(tps_p, _TRACEPARSER_INFO_BIG_ENDIAN, NULL) ||
	           !(get_header_value(tps_p, _TRACEPARSER_INFO_LITTLE_ENDIAN, NULL) || 
                 get_header_value(tps_p, _TRACEPARSER_INFO_BIG_ENDIAN, NULL))) {
		_TP_ERROR("Wrong input trace file endian type");
		_TP_CLOSE_FILE(tps_p);
		errno           = EINVAL;
		tps_p->error    = _TRACEPARSER_WRONG_ENDIAN_TYPE;

		return (-1);
	}

	/* check tracelogger version */
	if (atoi(get_header_value(tps_p, _TRACEPARSER_INFO_VER_MAJOR, NULL)) < _TRACELOGGER_COMPAT_VER_MAJOR ||
			(atoi(get_header_value(tps_p, _TRACEPARSER_INFO_VER_MAJOR, NULL)) == _TRACELOGGER_COMPAT_VER_MAJOR &&
	     atoi(get_header_value(tps_p, _TRACEPARSER_INFO_VER_MINOR, NULL)) < _TRACELOGGER_COMPAT_VER_MINOR)) {
		_TP_CLOSE_FILE(tps_p);
		errno           = EINVAL;
		tps_p->error    = _TRACEPARSER_WRONG_FILE_VERSION;

		return (-1);
	}

	/* reading syspage */
	if (get_header_value(tps_p, _TRACEPARSER_INFO_SYSPAGE_LEN, NULL)) {
		size_t l=(size_t)strtoul(get_header_value(tps_p, _TRACEPARSER_INFO_SYSPAGE_LEN, NULL), NULL, 10);

		if ((tps_p->syspage=malloc(l))==NULL) {
			return (nomem(tps_p));
		}
		if (read(tps_p->file_des, tps_p->syspage, l)!=l) {
			_TP_ERROR("couldn't read syspage entry from input trace file");
			_TP_CLOSE_FILE(tps_p);
			errno           = EINVAL;
			tps_p->error    = _TRACEPARSER_CANNOT_READ_IN_FILE;

			return (-1);
		}
		if ((tps_p->debug_flags&_TRACEPARSER_DEBUG_ALL)==_TRACEPARSER_DEBUG_ALL&&tps_p->debug_stream) {
			(void) fprintf(tps_p->debug_stream, " -- SYSPAGE INFORMATION -- \n");
			print_syspage(tps_p->debug_stream, (struct syspage_entry*)tps_p->syspage, tps_p->endian_conv);
		}
	}

	/* executing "NULL" callbac function */
	if (tps_p->callbacks[0][0]) {
		tps_p->now_callback_class = 0;
		tps_p->now_callback_event = 0;
		if(tps_p->user_data[0][0]) {
			tps_p->last_callback_return = (*tps_p->callbacks[0][0])(tps_p, tps_p->user_data[0][0], 0U, 0U, NULL, 0U);
		} else {
			tps_p->last_callback_return = (*tps_p->callbacks[0][0])(tps_p, tps_p->single_user_data, 0U, 0U, NULL, 0U);
		}
		tps_p->last_callback_class = 0;
		tps_p->last_callback_event = 0;
	}

	/* reading kernel time events */
	if (tps_p->debug_flags&_TRACEPARSER_DEBUG_HEADER&&tps_p->debug_stream) {
		(void) fprintf(tps_p->debug_stream, " -- KERNEL EVENTS -- \n");
	}
	while(read(tps_p->file_des, &t_e, sizeof(t_e))==sizeof(t_e)) {
		if (tps_p->endian_conv) arr_swap32((long*)(&t_e), 2);
		if ((tps_p->debug_flags&_TRACEPARSER_DEBUG_ALL)==_TRACEPARSER_DEBUG_ALL&&tps_p->debug_stream) {
			(void) fprintf
			(
			 tps_p->debug_stream,
			 "event => h:0x%8.8lx d0:0x%8.8lx d1:0x%8.8lx d2:0x%8.8lx\n",
			 (unsigned long)t_e.header,
			 (unsigned long)t_e.data[0],
			 (unsigned long)t_e.data[1],
			 (unsigned long)t_e.data[2]
			); 
		}
		if (_TRACE_GET_STRUCT(t_e.header)==_TRACE_STRUCT_S) {
			if (simple(tps_p, &t_e)) return (-1);
		} else {
			if (combine(tps_p, &t_e)) return (-1);
		}
	}
	_TP_CLOSE_FILE(tps_p);
	finish(tps_p);

	return (0);
}

#ifdef __QNXNTO__
__SRCVERSION("traceparser.c $Rev: 166135 $");
#endif
