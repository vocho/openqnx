/* glibstub.c */

/* This file is derived from the header files for glib.  Those header files
 * bear the following copyright notice.
 */

/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* <glib/glibconfig.h> or <glib/glib.h> lower bound of "short" (system dependent) */
#define G_MINSHORT

/* <glib/glibconfig.h> or <glib/glib.h> upper bound of "short" (system dependent) */
#define G_MAXSHORT

/* <glib/glibconfig.h> or <glib/glib.h> lower bound of "int" (system dependent) */
#define G_MININT

/* <glib/glibconfig.h> or <glib/glib.h> upper bound of "int" (system dependent) */
#define G_MAXINT

/* <glib/glibconfig.h> or <glib/glib.h> lower bound of "long" (system dependent) */
#define G_MINLONG

/* <glib/glibconfig.h> or <glib/glib.h> upper bound of "long" (system dependent) */
#define G_MAXLONG

/* <glib/glibconfig.h> or <glib/glib.h> lower bound of "float" (system dependent) */
#define G_MINFLOAT

/* <glib/glibconfig.h> or <glib/glib.h> upper bound of "float" (system dependent) */
#define G_MAXFLOAT

/* <glib/glibconfig.h> or <glib/glib.h> lower bound of "double" (system dependent) */
#define G_MINDOUBLE

/* <glib/glibconfig.h> or <glib/glib.h> upper bound of "double" (system dependent) */
#define G_MAXDOUBLE


/* <glib/glibconfig.h> or <glib/glib.h> */
typedef char gint8;		/* 8-bit signed integer (system dependent) */

/* <glib/glibconfig.h> or <glib/glib.h> */
typedef unsigned char guint8;	/* 8-bit unsigned integer (system dependent) */

/* <glib/glibconfig.h> or <glib/glib.h> */
typedef short gint16;		/* 16-bit signed integer (system dependent) */

/* <glib/glibconfig.h> or <glib/glib.h> */
typedef unsigned short guint16;	/* 8-bit unsigned integer (system dependent) */

/* <glib/glibconfig.h> or <glib/glib.h> */
typedef long gint32;		/* 32-bit signed integer (system dependent) */

/* <glib/glibconfig.h> or <glib/glib.h> */
typedef unsigned long guint32;	/* 8-bit unsigned integer (system dependent) */

/* <glib/glibconfig.h> or <glib/glib.h> */
typedef long long gint64;	/* 64-bit signed integer (system dependent) */

/* <glib/glibconfig.h> or <glib/glib.h> */
typedef unsigned long long guint64; /* 64-bit unsigned integer (system dep.) */

/* <glib/glibconfig.h> or <glib/glib.h> this system's byte order */
#define G_BYTE_ORDER G_LITTLE_ENDIAN /* system dependent */


/* <glib/glibconfig.h> or <glib/glib.h> convert gint to pointer */
#define GINT_TO_POINTER(i)	((gpointer)(p))

/* <glib/glibconfig.h> or <glib/glib.h> convert guint to pointer */
#define GUINT_TO_POINTER(i)	((gpointer)(p))

/* <glib/glibconfig.h> or <glib/glib.h> convert pointer to gint */
#define GPOINTER_TO_INT(p)	((gint)(p))

/* <glib/glibconfig.h> or <glib/glib.h> convert pointer to guint */
#define GPOINTER_TO_UINT(p)	((guint)(p))

/* <glib/glibconfig.h> or <glib/glib.h> wrapper around the atexit() function */
#define g_ATEXIT atexit

/* <glib/glibconfig.h> or <glib/glib.h> wrapper around the memmove() function */
#define g_memmove memmove

/* <glib/glib.h> directory separator (system dependent) */
#define G_DIR_SEPARATOR '/'

/* <glib/glib.h> directory separator, as a string (system dependent) */
#define G_DIR_SEPARATOR_S "/"

/* <glib/glib.h> directory separator (system dependent) */
#define G_SEARCHPATH_SEPARATOR ':'

/* <glib/glib.h> search path separator, as a string (system dependent) */
#define G_SEARCHPATH_SEPARATOR_S ":"

/* <glib/glib.h> */
#define	FALSE	(0)

/* <glib/glib.h> */
#define	TRUE	(!FALSE)

/* <glib/glib.h> */
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))

/* <glib/glib.h> */
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

/* <glib/glib.h> */
#define ABS(a)	  (((a) < 0) ? -(a) : (a))

/* <glib/glib.h> */
#define CLAMP(x,low,high)  (((x)>(high)) ? (high) : (((x)<(low)) ? (low) : (x)))


/* <glib/glib.h> or <glibconfig.h>  Copy va_list variables (system dependent) */
#define G_VA_COPY(ap1, ap2)	  ((ap1) = (ap2))


/* <glib/glib.h>  offset of a field within a struct */
#define G_STRUCT_OFFSET(struct_type, member) ...

/* <glib/glib.h>  pointer to field, given a base pointer and field offset */ 
#define G_STRUCT_MEMBER_P(struct_p, struct_offset)   \
    ((gpointer) ((gchar*) (struct_p) + (gulong) (struct_offset)))

/* <glib/glib.h>  value of a field, given its type, base pointer, and field offset */
#define G_STRUCT_MEMBER(member_type, struct_p, struct_offset)   \
    (*(member_type*) G_STRUCT_MEMBER_P ((struct_p), (struct_offset)))

/* <glib/glib.h> inline keyword, if the compiler supports it.  (system dependent) */
#define G_INLINE_FUNC inline

/* <glib/glib.h> Provide simple macro statement wrappers (adapted from Perl):
 *  G_STMT_START { statements; } G_STMT_END;
 *  can be used as a single statement, as in
 *  if (x) G_STMT_START { ... } G_STMT_END; else ...
 */
#define G_STMT_START	do
#define G_STMT_END	while (0)


/* <glib/glib.h> allocate an array of "count" items of type "type" */
#define g_new(type, count) ...

/* <glib/glib.h> allocate an array of "count" items of type "type", and zero it */
#define g_new0(type, count) ...

/* <glib/glib.h> reallocate an array of "count" items of type "type" */
#define g_renew(type, mem, count) ...

/* <glib/glib.h> */
#define g_mem_chunk_create(type, pre_alloc, alloc_type) ...

/* <glib/glib.h> */
#define g_chunk_new(type, chunk) ...

/* <glib/glib.h> */
#define g_chunk_new0(type, chunk) ...

/* <glib/glib.h> */
#define g_chunk_free(mem, mem_chunk) ...

/* <glib/glib.h> convert a symbol to a string */
#define g_string(x) #x


#ifdef G_DISABLE_ASSERT

/* <glib/glib.h> assertions, have no effect if G_DISABLE_ASSERT is defined */
#define g_assert(expr)
#define g_assert_not_reached()

#ifdef G_DISABLE_CHECKS

/* <glib/glib.h> Return if an expression is FALSE (with or without a return value */
#define g_return_if_fail(expr)		if (expr); else return
#define g_return_val_if_fail(expr,val)	if (expr); else return (val)

/* <glib/glib.h> */
typedef char   gchar;

/* <glib/glib.h> */
typedef short  gshort;

/* <glib/glib.h> */
typedef long   glong;

/* <glib/glib.h> */
typedef int    gint;

/* <glib/glib.h> */
typedef gint   gboolean;

/* <glib/glib.h> */
typedef unsigned char	guchar;

/* <glib/glib.h> */
typedef unsigned short	gushort;

/* <glib/glib.h> */
typedef unsigned long	gulong;

/* <glib/glib.h> */
typedef unsigned int	guint;

/* <glib/glib.h> */
typedef float	gfloat;

/* <glib/glib.h> */
typedef double	gdouble;

/* <glib/glib.h> */
typedef void* gpointer;

/* <glib/glib.h> */
typedef const void *gconstpointer;

/* <glib/glib.h> */
typedef gint32	gssize;

/* <glib/glib.h> */
typedef guint32 gsize;

/* <glib/glib.h> */
typedef guint32 GQuark;

/* <glib/glib.h> */
typedef gint32	GTime;


/* <glib/glib.h> compare to G_BYTE_ORDER */
#define G_LITTLE_ENDIAN 1234

/* <glib/glib.h> compare to G_BYTE_ORDER */
#define G_BIG_ENDIAN    4321

/* <glib/glib.h> compare to G_BYTE_ORDER */
#define G_PDP_ENDIAN    3412		/* unused, need specific PDP check */	


/* <glib/glib.h> convert big-endian to little-endian or vice-versa */
#define GUINT16_SWAP_LE_BE_CONSTANT(val) ...

/* <glib/glib.h> convert big-endian to little-endian or vice-versa */
#define GUINT32_SWAP_LE_BE_CONSTANT(val) ...

/* <glib/glib.h> convert big-endian to little-endian or vice-versa */
#define GUINT64_SWAP_LE_BE_CONSTANT(val) ...

/* <glib/glib.h> like *_CONSTANT macros, but may be replaced with faster code for
 * variable values on some architectures.
 */
#define GUINT16_SWAP_LE_BE(val) (GUINT16_SWAP_LE_BE_CONSTANT (val))
#define GUINT32_SWAP_LE_BE(val) (GUINT32_SWAP_LE_BE_CONSTANT (val))
#define GUINT64_SWAP_LE_BE(val) (GUINT64_SWAP_LE_BE_CONSTANT (val))

/* <glib/glib.h> */
#define GUINT16_SWAP_LE_PDP(val) ...

/* <glib/glib.h> */
#define GUINT16_SWAP_BE_PDP(val) ...

/* <glib/glib.h> */
#define GUINT32_SWAP_LE_PDP(val) ...

/* <glib/glib.h> */
#define GUINT32_SWAP_BE_PDP(val) ...

/* <glib/glib.h> */
#define GINT16_FROM_LE(val) ...

/* <glib/glib.h> */
#define GUINT16_FROM_LE(val) ...

/* <glib/glib.h> */
#define GINT16_FROM_BE(val) ...

/* <glib/glib.h> */
#define GUINT16_FROM_BE(val) ...

/* <glib/glib.h> */
#define GINT32_FROM_LE(val) ...

/* <glib/glib.h> */
#define GUINT32_FROM_LE(val) ...

/* <glib/glib.h> */
#define GINT32_FROM_BE(val) ...

/* <glib/glib.h> */
#define GUINT32_FROM_BE(val) ...

/* <glib/glib.h> */
#define GINT64_FROM_LE(val) ...

/* <glib/glib.h> */
#define GUINT64_FROM_LE(val) ...

/* <glib/glib.h> */
#define GINT64_FROM_BE(val) ...

/* <glib/glib.h> */
#define GUINT64_FROM_BE(val) ...

/* <glib/glib.h> */
#define GLONG_FROM_LE(val) ...

/* <glib/glib.h> */
#define GULONG_FROM_LE(val) ...

/* <glib/glib.h> */
#define GLONG_FROM_BE(val) ...

/* <glib/glib.h> */
#define GULONG_FROM_BE(val) ...

/* <glib/glib.h> */
#define GUINT_FROM_LE(val) ...

/* <glib/glib.h> */
#define GINT_FROM_BE(val) ...

/* <glib/glib.h> */
#define GUINT_FROM_BE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GINT16_TO_LE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GUINT16_TO_LE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GINT16_TO_BE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GUINT16_TO_BE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GINT32_TO_LE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GUINT32_TO_LE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GINT32_TO_BE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GUINT32_TO_BE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GINT64_TO_LE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GUINT64_TO_LE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GINT64_TO_BE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GUINT64_TO_BE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GLONG_TO_LE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GULONG_TO_LE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GLONG_TO_BE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GULONG_TO_BE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GUINT_TO_LE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GINT_TO_BE(val) ...

/* <glib/glib.h> or <glibconfig.h> */
#define GUINT_TO_BE(val) ...

/* <glib/glib.h> similar to ntohl() and related functions, but for specific sizes */
#define g_ntohl(val) ...
#define g_ntohs(val) ...
#define g_htonl(val) ...
#define g_htons(val) ...


/* <glib/glib.h>  variable declaration prefix that works in Windows DLLs.  For Unix,
 * just plain "extern" works okay.
 */
#define GUTILS_C_VAR extern

/* <glib/glib.h> */
GUTILS_C_VAR const guint glib_major_version;

/* <glib/glib.h> */
GUTILS_C_VAR const guint glib_minor_version;

/* <glib/glib.h> */
GUTILS_C_VAR const guint glib_micro_version;

/* <glib/glib.h> */
GUTILS_C_VAR const guint glib_interface_age;

/* <glib/glib.h> */
GUTILS_C_VAR const guint glib_binary_age;

/* <glib/glib.h> return TRUE if glib version is >= requested version */
#define GLIB_CHECK_VERSION(major,minor,micro) ...

/* <glib/glib.h> Tree traverse flags */
typedef enum
{
  G_TRAVERSE_LEAFS	= 1 << 0,
  G_TRAVERSE_NON_LEAFS	= 1 << 1,
  G_TRAVERSE_ALL	= G_TRAVERSE_LEAFS | G_TRAVERSE_NON_LEAFS,
  G_TRAVERSE_MASK	= 0x03
} GTraverseFlags;

/* <glib/glib.h> Tree traverse orders */
typedef enum
{
  G_IN_ORDER,
  G_PRE_ORDER,
  G_POST_ORDER,
  G_LEVEL_ORDER
} GTraverseType;

/* <glib/glib.h> Log level shift offset for user defined
 * log levels (0-7 are used by GLib).
 */
#define	G_LOG_LEVEL_USER_SHIFT	(8)

/* <glib/glib.h> Glib log levels and flags.  */
typedef enum
{
  /* log flags */
  G_LOG_FLAG_RECURSION		= 1 << 0,
  G_LOG_FLAG_FATAL		= 1 << 1,
  
  /* GLib log levels */
  G_LOG_LEVEL_ERROR		= 1 << 2,	/* always fatal */
  G_LOG_LEVEL_CRITICAL		= 1 << 3,
  G_LOG_LEVEL_WARNING		= 1 << 4,
  G_LOG_LEVEL_MESSAGE		= 1 << 5,
  G_LOG_LEVEL_INFO		= 1 << 6,
  G_LOG_LEVEL_DEBUG		= 1 << 7,
  
  G_LOG_LEVEL_MASK		= ~(G_LOG_FLAG_RECURSION | G_LOG_FLAG_FATAL)
} GLogLevelFlags;

/* <glib/glib.h> GLib log levels that are considered fatal by default */
#define	G_LOG_FATAL_MASK	(G_LOG_FLAG_RECURSION | G_LOG_LEVEL_ERROR)

/* <glib/glib.h> */
typedef gpointer (*GCacheNewFunc) (gpointer key);

/* <glib/glib.h> */
typedef gpointer (*GCacheDupFunc) (gpointer value);

/* <glib/glib.h> */
typedef void (*GCacheDestroyFunc) (gpointer value);

/* <glib/glib.h> */
typedef gint (*GCompareFunc) (gconstpointer a, gconstpointer b);

/* <glib/glib.h> */
typedef gchar* (*GCompletionFunc) (gpointer);

/* <glib/glib.h> */
typedef void (*GDestroyNotify) (gpointer data);

/* <glib/glib.h> */
typedef void (*GDataForeachFunc) (GQuark key_id, gpointer data, gpointer user_data);

/* <glib/glib.h> */
typedef void (*GFunc) (gpointer data, gpointer user_data);

/* <glib/glib.h> */
typedef guint (*GHashFunc) (gconstpointer key);

/* <glib/glib.h> */
typedef void (*GFreeFunc) (gpointer data);

/* <glib/glib.h> */
typedef void (*GHFunc) (gpointer key, gpointer value, gpointer user_data);

/* <glib/glib.h> */
typedef gboolean (*GHRFunc) (gpointer key, gpointer value, gpointer user_data);

/* <glib/glib.h> */
typedef gint (*GHookCompareFunc) (GHook *new_hook, GHook *sibling);

/* <glib/glib.h> */
typedef gboolean (*GHookFindFunc) (GHook *hook, gpointer data);

/* <glib/glib.h> */
typedef void (*GHookMarshaller) (GHook *hook, gpointer data);

/* <glib/glib.h> */
typedef gboolean (*GHookCheckMarshaller) (GHook *hook, gpointer data);

/* <glib/glib.h> */
typedef void (*GHookFunc) (gpointer data);

/* <glib/glib.h> */
typedef gboolean (*GHookCheckFunc) (gpointer data);

/* <glib/glib.h> */
typedef void (*GHookFreeFunc) (GHookList *hook_list, GHook *hook);

/* <glib/glib.h> */
typedef void (*GLogFunc) (const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);

/* <glib/glib.h> */
typedef gboolean (*GNodeTraverseFunc) (GNode *node, gpointer data);

/* <glib/glib.h> */
typedef void (*GNodeForeachFunc) (GNode *node, gpointer data);

/* <glib/glib.h> */
typedef gint (*GSearchFunc) (gpointer key, gpointer data);

/* <glib/glib.h> */
typedef void (*GScannerMsgFunc) (GScanner *scanner, gchar *message, gint error);

/* <glib/glib.h> */
typedef gint (*GTraverseFunc) (gpointer key, gpointer value, gpointer data);

/* <glib/glib.h> */
typedef void (*GVoidFunc) (void);

/* <glib/glib.h> */
typedef struct _GList
{
  gpointer data;
  GList *next;
  GList *prev;
} GList;

typedef struct _GSList
{
  gpointer data;
  GSList *next;
} GSList;

typedef struct _GString
{
  gchar *str;
  gint len;
} GString;

typedef struct _GArray
{
  gchar *data;
  guint len;
} GArray;

typedef struct _GByteArray
{
  guint8 *data;
  guint	  len;
} GByteArray;

typedef struct _GPtrArray
{
  gpointer *pdata;
  guint	    len;
} GPtrArray;

typedef struct _GTuples
{
  guint len;
} GTuples;

typedef struct _GDebugKey
{
  gchar *key;
  guint	 value;
} GDebugKey;



/* <glib/glib.h> */
void g_list_push_allocator(GAllocator *allocator)
{
}

/* <glib/glib.h> */
void g_list_pop_allocator(void)
{
}

/* <glib/glib.h> */
GList* g_list_alloc(void)
{
}

/* <glib/glib.h> */
void g_list_free(GList *list)
{
}

/* <glib/glib.h> */
void g_list_free_1(GList *list)
{
}

/* <glib/glib.h> */
GList* g_list_append(GList *list, gpointer data)
{
}

/* <glib/glib.h> */
GList* g_list_prepend(GList *list, gpointer data)
{
}

/* <glib/glib.h> */
GList* g_list_insert(GList *list, gpointer data, gint position)
{
}

/* <glib/glib.h> */
GList* g_list_insert_sorted(GList *list, gpointer data, GCompareFunc func)
{
}

/* <glib/glib.h> */
GList* g_list_concat(GList *list1, GList *list2)
{
}

/* <glib/glib.h> */
GList* g_list_remove(GList *list, gpointer data)
{
}

/* <glib/glib.h> */
GList* g_list_remove_link(GList *list, GList *llink)
{
}

/* <glib/glib.h> */
GList* g_list_reverse(GList *list)
{
}

/* <glib/glib.h> */
GList* g_list_copy(GList *list)
{
}

/* <glib/glib.h> */
GList* g_list_nth(GList *list, guint n)
{
}

/* <glib/glib.h> */
GList* g_list_find(GList *list, gpointer data)
{
}

/* <glib/glib.h> */
GList* g_list_find_custom(GList *list, gpointer data, GCompareFunc func)
{
}

/* <glib/glib.h> */
gint g_list_position(GList *list, GList *llink)
{
}

/* <glib/glib.h> */
gint g_list_index(GList *list, gpointer data)
{
}

/* <glib/glib.h> */
GList* g_list_last(GList *list)
{
}

/* <glib/glib.h> */
GList* g_list_first(GList *list)
{
}

/* <glib/glib.h> */
guint g_list_length(GList *list)
{
}

/* <glib/glib.h> */
void g_list_foreach(GList *list, GFunc func, gpointer user_data)
{
}

/* <glib/glib.h> */
GList* g_list_sort(GList *list, GCompareFunc compare_func)
{
}

/* <glib/glib.h> */
gpointer g_list_nth_data(GList *list, guint n)
{
}

/* <glib/glib.h> */
#define g_list_previous(list)	((list) ? (((GList *)(list))->prev) : NULL)

/* <glib/glib.h> */
#define g_list_next(list)	((list) ? (((GList *)(list))->next) : NULL)

/* <glib/glib.h> */
void g_slist_push_allocator(GAllocator *allocator)
{
}

/* <glib/glib.h> */
void g_slist_pop_allocator(void)
{
}

/* <glib/glib.h> */
GSList* g_slist_alloc(void)
{
}

/* <glib/glib.h> */
void g_slist_free(GSList *list)
{
}

/* <glib/glib.h> */
void g_slist_free_1(GSList *list)
{
}

/* <glib/glib.h> */
GSList* g_slist_append(GSList *list, gpointer data)
{
}

/* <glib/glib.h> */
GSList* g_slist_prepend(GSList *list, gpointer data)
{
}

/* <glib/glib.h> */
GSList* g_slist_insert(GSList *list, gpointer data, gint position)
{
}

/* <glib/glib.h> */
GSList* g_slist_insert_sorted(GSList *list, gpointer data, GCompareFunc func)
{
}

/* <glib/glib.h> */
GSList* g_slist_concat(GSList *list1, GSList *list2)
{
}

/* <glib/glib.h> */
GSList* g_slist_remove(GSList *list, gpointer data)
{
}

/* <glib/glib.h> */
GSList* g_slist_remove_link(GSList *list, GSList *llink)
{
}

/* <glib/glib.h> */
GSList* g_slist_reverse(GSList *list)
{
}

/* <glib/glib.h> */
GSList* g_slist_copy(GSList *list)
{
}

/* <glib/glib.h> */
GSList* g_slist_nth(GSList *list, guint n)
{
}

/* <glib/glib.h> */
GSList* g_slist_find(GSList *list, gpointer data)
{
}

/* <glib/glib.h> */
GSList* g_slist_find_custom(GSList *list, gpointer data, GCompareFunc func)
{
}

/* <glib/glib.h> */
gint g_slist_position(GSList *list, GSList *llink)
{
}

/* <glib/glib.h> */
gint g_slist_index(GSList *list, gpointer data)
{
}

/* <glib/glib.h> */
GSList* g_slist_last(GSList *list)
{
}

/* <glib/glib.h> */
guint g_slist_length(GSList *list)
{
}

/* <glib/glib.h> */
void g_slist_foreach(GSList *list, GFunc func, gpointer user_data)
{
}

/* <glib/glib.h> */
GSList* g_slist_sort(GSList *list, GCompareFunc compare_func)
{
}

/* <glib/glib.h> */
gpointer g_slist_nth_data(GSList *list, guint n)
{
}

/* <glib/glib.h> */
#define g_slist_next(slist)	((slist) ? (((GSList *)(slist))->next) : NULL)

/* <glib/glib.h> */
GHashTable* g_hash_table_new(GHashFunc hash_func, GCompareFunc key_compare_func)
{
}

/* <glib/glib.h> */
void g_hash_table_destroy(GHashTable *hash_table)
{
}

/* <glib/glib.h> */
void g_hash_table_insert(GHashTable *hash_table, gpointer key, gpointer value)
{
}

/* <glib/glib.h> */
void g_hash_table_remove(GHashTable *hash_table, gconstpointer key)
{
}

/* <glib/glib.h> */
gpointer g_hash_table_lookup(GHashTable *hash_table, gconstpointer key)
{
}

/* <glib/glib.h> */
gboolean g_hash_table_lookup_extended(GHashTable *hash_table, gconstpointer lookup_key, gpointer *orig_key, gpointer *value)
{
}

/* <glib/glib.h> */
void g_hash_table_freeze(GHashTable *hash_table)
{
}

/* <glib/glib.h> */
void g_hash_table_thaw(GHashTable *hash_table)
{
}

/* <glib/glib.h> */
void g_hash_table_foreach(GHashTable *hash_table, GHFunc func, gpointer user_data)
{
}

/* <glib/glib.h> */
guint g_hash_table_foreach_remove(GHashTable *hash_table, GHRFunc func, gpointer user_data)
{
}

/* <glib/glib.h> */
guint g_hash_table_size(GHashTable *hash_table)
{
}

/* <glib/glib.h> */
GCache* g_cache_new(GCacheNewFunc value_new_func, GCacheDestroyFunc value_destroy_func, GCacheDupFunc key_dup_func, GCacheDestroyFunc key_destroy_func, GHashFunc hash_key_func, GHashFunc hash_value_func, GCompareFunc key_compare_func)
{
}

/* <glib/glib.h> */
void g_cache_destroy(GCache *cache)
{
}

/* <glib/glib.h> */
gpointer g_cache_insert(GCache *cache, gpointer key)
{
}

/* <glib/glib.h> */
void g_cache_remove(GCache *cache, gpointer value)
{
}

/* <glib/glib.h> */
void g_cache_key_foreach(GCache *cache, GHFunc func, gpointer user_data)
{
}

/* <glib/glib.h> */
void g_cache_value_foreach(GCache *cache, GHFunc func, gpointer user_data)
{
}

/* <glib/glib.h> */
GTree* g_tree_new(GCompareFunc key_compare_func)
{
}

/* <glib/glib.h> */
void g_tree_destroy(GTree *tree)
{
}

/* <glib/glib.h> */
void g_tree_insert(GTree *tree, gpointer key, gpointer value)
{
}

/* <glib/glib.h> */
void g_tree_remove(GTree *tree, gpointer key)
{
}

/* <glib/glib.h> */
gpointer g_tree_lookup(GTree *tree, gpointer key)
{
}

/* <glib/glib.h> */
void g_tree_traverse(GTree *tree, GTraverseFunc traverse_func, GTraverseType traverse_type, gpointer data)
{
}

/* <glib/glib.h> */
gpointer g_tree_search(GTree *tree, GSearchFunc search_func, gpointer data)
{
}

/* <glib/glib.h> */
gint g_tree_height(GTree *tree)
{
}

/* <glib/glib.h> */
gint g_tree_nnodes(GTree *tree)
{
}

/* <glib/glib.h> */
typedef struct _GNode
{
  gpointer data;
  GNode	  *next;
  GNode	  *prev;
  GNode	  *parent;
  GNode	  *children;
} GNode;

/* <glib/glib.h> */
#define	 G_NODE_IS_ROOT(node) ...

/* <glib/glib.h> */
#define	 G_NODE_IS_LEAF(node) ...


/* <glib/glib.h> */
void g_node_push_allocator(GAllocator *allocator)
{
}

/* <glib/glib.h> */
void g_node_pop_allocator(void)
{
}

/* <glib/glib.h> */
GNode* g_node_new(gpointer data)
{
}

/* <glib/glib.h> */
void g_node_destroy(GNode *root)
{
}

/* <glib/glib.h> */
void g_node_unlink(GNode *node)
{
}

/* <glib/glib.h> */
GNode* g_node_insert(GNode *parent, gint position, GNode *node)
{
}

/* <glib/glib.h> */
GNode* g_node_insert_before(GNode *parent, GNode *sibling, GNode *node)
{
}

/* <glib/glib.h> */
GNode* g_node_prepend(GNode *parent, GNode *node)
{
}

/* <glib/glib.h> */
guint g_node_n_nodes(GNode *root, GTraverseFlags flags)
{
}

/* <glib/glib.h> */
GNode* g_node_get_root(GNode *node)
{
}

/* <glib/glib.h> */
gboolean g_node_is_ancestor(GNode *node, GNode *descendant)
{
}

/* <glib/glib.h> */
guint g_node_depth(GNode *node)
{
}

/* <glib/glib.h> */
GNode* g_node_find(GNode *root, GTraverseType order, GTraverseFlags flags, gpointer data)
{
}

/* <glib/glib.h> */
#define g_node_append(parent, node) ...

/* <glib/glib.h> */
#define	g_node_insert_data(parent, position, data) ...

/* <glib/glib.h> */
#define	g_node_insert_data_before(parent, sibling, data) ...

/* <glib/glib.h> */
#define	g_node_prepend_data(parent, data) ...

/* <glib/glib.h> */
#define	g_node_append_data(parent, data) ...

/* <glib/glib.h> */
void g_node_traverse(GNode *root, GTraverseType order, GTraverseFlags flags, gint max_depth, GNodeTraverseFunc func, gpointer data)
{
}

/* <glib/glib.h> */
guint g_node_max_height(GNode *root)
{
}

/* <glib/glib.h> */
void g_node_children_foreach(GNode *node, GTraverseFlags flags, GNodeForeachFunc func, gpointer data)
{
}

/* <glib/glib.h> */
void g_node_reverse_children(GNode *node)
{
}

/* <glib/glib.h> */
guint g_node_n_children(GNode *node)
{
}

/* <glib/glib.h> */
GNode* g_node_nth_child(GNode *node, guint n)
{
}

/* <glib/glib.h> */
GNode* g_node_last_child(GNode *node)
{
}

/* <glib/glib.h> */
GNode* g_node_find_child(GNode *node, GTraverseFlags flags, gpointer data)
{
}

/* <glib/glib.h> */
gint g_node_child_position(GNode *node, GNode *child)
{
}

/* <glib/glib.h> */
gint g_node_child_index(GNode *node, gpointer data)
{
}

/* <glib/glib.h> */
GNode* g_node_first_sibling(GNode *node)
{
}

/* <glib/glib.h> */
GNode* g_node_last_sibling(GNode *node)
{
}

/* <glib/glib.h> */
#define	 g_node_prev_sibling(node) ...

/* <glib/glib.h> */
#define	 g_node_next_sibling(node) ...

/* <glib/glib.h> */
#define	 g_node_first_child(node) ...

/* <glib/glib.h> */
#define G_HOOK_FLAG_USER_SHIFT	(4)

/* <glib/glib.h> */
typedef enum
{
  G_HOOK_FLAG_ACTIVE	= 1 << 0,
  G_HOOK_FLAG_IN_CALL	= 1 << 1,
  G_HOOK_FLAG_MASK	= 0x0f
} GHookFlagMask;

/* <glib/glib.h> */
#define	G_HOOK_DEFERRED_DESTROY	((GHookFreeFunc) 0x01)

/* <glib/glib.h> */
typedef struct _GHookList
{
  guint		 seq_id;
  guint		 hook_size;
  guint		 is_setup : 1;
  GHook		*hooks;
  GMemChunk	*hook_memchunk;
  GHookFreeFunc	 hook_free; /* virtual function */
  GHookFreeFunc	 hook_destroy; /* virtual function */
} GHookList;

/* <glib/glib.h> */
typedef struct _GHook
{
  gpointer	 data;
  GHook		*next;
  GHook		*prev;
  guint		 ref_count;
  guint		 hook_id;
  guint		 flags;
  gpointer	 func;
  GDestroyNotify destroy;
} GHook;


/* <glib/glib.h> */
#define	G_HOOK_ACTIVE(hook) ...

/* <glib/glib.h> */
#define	G_HOOK_IN_CALL(hook) ...

/* <glib/glib.h> */
#define G_HOOK_IS_VALID(hook) ...

/* <glib/glib.h> */
#define G_HOOK_IS_UNLINKED(hook) ...

/* <glib/glib.h> */
void g_hook_list_init(GHookList *hook_list, guint hook_size)
{
}

/* <glib/glib.h> */
void g_hook_list_clear(GHookList *hook_list)
{
}

/* <glib/glib.h> */
GHook* g_hook_alloc(GHookList *hook_list)
{
}

/* <glib/glib.h> */
void g_hook_free(GHookList *hook_list, GHook *hook)
{
}

/* <glib/glib.h> */
void g_hook_ref(GHookList *hook_list, GHook *hook)
{
}

/* <glib/glib.h> */
void g_hook_unref(GHookList *hook_list, GHook *hook)
{
}

/* <glib/glib.h> */
gboolean g_hook_destroy(GHookList *hook_list, guint hook_id)
{
}

/* <glib/glib.h> */
void g_hook_destroy_link(GHookList *hook_list, GHook *hook)
{
}

/* <glib/glib.h> */
void g_hook_prepend(GHookList *hook_list, GHook *hook)
{
}

/* <glib/glib.h> */
void g_hook_insert_before(GHookList *hook_list, GHook *sibling, GHook *hook)
{
}

/* <glib/glib.h> */
void g_hook_insert_sorted(GHookList *hook_list, GHook *hook, GHookCompareFunc func)
{
}

/* <glib/glib.h> */
GHook* g_hook_get(GHookList *hook_list, guint hook_id)
{
}

/* <glib/glib.h> */
GHook* g_hook_find(GHookList *hook_list, gboolean need_valids, GHookFindFunc func, gpointer data)
{
}

/* <glib/glib.h> */
GHook* g_hook_find_data(GHookList *hook_list, gboolean need_valids, gpointer data)
{
}

/* <glib/glib.h> */
GHook* g_hook_find_func(GHookList *hook_list, gboolean need_valids, gpointer func)
{
}

/* <glib/glib.h> */
GHook* g_hook_find_func_data(GHookList *hook_list, gboolean need_valids, gpointer func, gpointer data)
{
}

/* <glib/glib.h> */
GHook* g_hook_first_valid(GHookList *hook_list, gboolean may_be_in_call)
{
}

/* <glib/glib.h> */
GHook* g_hook_next_valid(GHookList *hook_list, GHook *hook, gboolean may_be_in_call)
{
}

/* <glib/glib.h> */
gint g_hook_compare_ids(GHook *new_hook, GHook *sibling)
{
}

/* <glib/glib.h> */
#define g_hook_append( hook_list, hook ) ...

/* <glib/glib.h> */
void g_hook_list_invoke(GHookList *hook_list, gboolean may_recurse)
{
}

/* <glib/glib.h> */
void g_hook_list_invoke_check(GHookList *hook_list, gboolean may_recurse)
{
}

/* <glib/glib.h> */
void g_hook_list_marshal(GHookList *hook_list, gboolean may_recurse, GHookMarshaller marshaller, gpointer data)
{
}

/* <glib/glib.h> */
void g_hook_list_marshal_check(GHookList *hook_list, gboolean may_recurse, GHookCheckMarshaller marshaller, gpointer data)
{
}

/* <glib/glib.h> */
void g_on_error_query(const gchar *prg_name)
{
}

/* <glib/glib.h> */
void g_on_error_stack_trace(const gchar *prg_name)
{
}

/* <glib/glib.h> */
extern const gchar *g_log_domain_glib;

/* <glib/glib.h> */
guint g_log_set_handler(const gchar *log_domain, GLogLevelFlags log_levels, GLogFunc log_func, gpointer user_data)
{
}

/* <glib/glib.h> */
void g_log_remove_handler(const gchar *log_domain, guint handler_id)
{
}

/* <glib/glib.h> */
void g_log_default_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer unused_data)
{
}

/* <glib/glib.h> */
void g_log(const gchar *log_domain, GLogLevelFlags log_level, const gchar *format, ...)
{
}

/* <glib/glib.h> */
void g_logv(const gchar *log_domain, GLogLevelFlags log_level, const gchar *format, va_list args)
{
}

/* <glib/glib.h> */
GLogLevelFlags g_log_set_fatal_mask(const gchar *log_domain, GLogLevelFlags fatal_mask)
{
}

/* <glib/glib.h> */
GLogLevelFlags g_log_set_always_fatal(GLogLevelFlags fatal_mask)
{
}

/* <glib/glib.h> */
#define	G_LOG_DOMAIN	((gchar*) 0)

/* <glib/glib.h> */
#define	g_error(format...)

/* <glib/glib.h> */
#define	g_message(format...)

/* <glib/glib.h> */
#define	g_critical(format...)

/* <glib/glib.h> */
#define	g_warning(format...)

/* <glib/glib.h> */
typedef void(*GPrintFunc) (const gchar *string)
{
}

/* <glib/glib.h> */
void g_print(const gchar *format, ...)
{
}

/* <glib/glib.h> */
GPrintFunc g_set_print_handler(GPrintFunc func)
{
}

/* <glib/glib.h> */
void g_printerr(const gchar *format, ...)
{
}

/* <glib/glib.h> */
GPrintFunc g_set_printerr_handler(GPrintFunc func)
{
}

/* <glib/glib.h> */
gpointer g_malloc(gulong size)
{
}

/* <glib/glib.h> */
gpointer g_malloc0(gulong size)
{
}

/* <glib/glib.h> */
gpointer g_realloc(gpointer mem, gulong size)
{
}

/* <glib/glib.h> */
void g_free(gpointer mem)
{
}

/* <glib/glib.h> */
void g_mem_profile(void)
{
}

/* <glib/glib.h> */
void g_mem_check(gpointer mem)
{
}

/* <glib/glib.h> */
GAllocator* g_allocator_new(const gchar *name, guint n_preallocs)
{
}

/* <glib/glib.h> */
void g_allocator_free(GAllocator *allocator)
{
}

/* <glib/glib.h> */
#define G_ALLOCATOR_LIST (1)

/* <glib/glib.h> */
#define G_ALLOCATOR_SLIST (2)

/* <glib/glib.h> */
#define G_ALLOCATOR_NODE (3)

/* <glib/glib.h> */
#define G_ALLOC_ONLY 1

/* <glib/glib.h> */
#define G_ALLOC_AND_FREE 2

/* <glib/glib.h> */
GMemChunk* g_mem_chunk_new(gchar *name, gint atom_size, gulong area_size, gint type)
{
}

/* <glib/glib.h> */
void g_mem_chunk_destroy(GMemChunk *mem_chunk)
{
}

/* <glib/glib.h> */
gpointer g_mem_chunk_alloc(GMemChunk *mem_chunk)
{
}

/* <glib/glib.h> */
gpointer g_mem_chunk_alloc0(GMemChunk *mem_chunk)
{
}

/* <glib/glib.h> */
void g_mem_chunk_free(GMemChunk *mem_chunk, gpointer mem)
{
}

/* <glib/glib.h> */
void g_mem_chunk_clean(GMemChunk *mem_chunk)
{
}

/* <glib/glib.h> */
void g_mem_chunk_reset(GMemChunk *mem_chunk)
{
}

/* <glib/glib.h> */
void g_mem_chunk_print(GMemChunk *mem_chunk)
{
}

/* <glib/glib.h> */
void g_mem_chunk_info(void)
{
}

/* <glib/glib.h> */
void g_blow_chunks(void)
{
}

/* <glib/glib.h> */
GTimer* g_timer_new(void)
{
}

/* <glib/glib.h> */
void g_timer_destroy(GTimer *timer)
{
}

/* <glib/glib.h> */
void g_timer_start(GTimer *timer)
{
}

/* <glib/glib.h> */
void g_timer_stop(GTimer *timer)
{
}

/* <glib/glib.h> */
void g_timer_reset(GTimer *timer)
{
}

/* <glib/glib.h> */
gdouble g_timer_elapsed(GTimer *timer, gulong *microseconds)
{
}

/* <glib/glib.h> */
#define G_STR_DELIMITERS "_-|> <."

/* <glib/glib.h> */
gchar* g_strdelimit(gchar *string, const gchar *delimiters, gchar new_delimiter)
{
}

/* <glib/glib.h> */
gdouble g_strtod(const gchar *nptr, gchar **endptr)
{
}

/* <glib/glib.h> */
gchar* g_strerror(gint errnum)
{
}

/* <glib/glib.h> */
gchar* g_strsignal(gint signum)
{
}

/* <glib/glib.h> */
gint g_strcasecmp(const gchar *s1, const gchar *s2)
{
}

/* <glib/glib.h> */
gint g_strncasecmp(const gchar *s1, const gchar *s2, guint n)
{
}

/* <glib/glib.h> */
void g_strdown(gchar *string)
{
}

/* <glib/glib.h> */
void g_strup(gchar *string)
{
}

/* <glib/glib.h> */
void g_strreverse(gchar *string)
{
}

/* <glib/glib.h> */
gchar* g_strchug(gchar *string)
{
}

/* <glib/glib.h> */
gchar* g_strchomp(gchar *string)
{
}

/* <glib/glib.h> */
#define g_strstrip( string ) g_strchomp (g_strchug (string))

/* <glib/glib.h> */
gchar* g_strdup(const gchar *str)
{
}

/* <glib/glib.h> */
gchar* g_strdup_printf (const gchar *format, ...) G_GNUC_PRINTF(1, 2)
{
}

/* <glib/glib.h> */
gchar* g_strdup_vprintf(const gchar *format, va_list args)
{
}

/* <glib/glib.h> */
gchar* g_strndup(const gchar *str, guint n)
{
}

/* <glib/glib.h> */
gchar* g_strnfill(guint length, gchar fill_char)
{
}

/* <glib/glib.h> */
gchar* g_strconcat (const gchar *string1, ...); /* NULL terminated */

/* <glib/glib.h> */
gchar* g_strjoin (const gchar *separator, ...); /* NULL terminated */

/* <glib/glib.h> */
gchar* g_strescape(gchar *string)
{
}

/* <glib/glib.h> */
gpointer g_memdup(gconstpointer mem, guint byte_size)
{
}

/* <glib/glib.h> */
gchar** g_strsplit(const gchar *string, const gchar *delimiter, gint max_tokens)
{
}

/* <glib/glib.h> */
gchar* g_strjoinv(const gchar *separator, gchar **str_array)
{
}

/* <glib/glib.h> */
void g_strfreev(gchar **str_array)
{
}

/* <glib/glib.h> */
guint g_printf_string_upper_bound(const gchar* format, va_list args)
{
}

/* <glib/glib.h> */
gchar* g_get_user_name(void)
{
}

/* <glib/glib.h> */
gchar* g_get_real_name(void)
{
}

/* <glib/glib.h> */
gchar* g_get_home_dir(void)
{
}

/* <glib/glib.h> */
gchar* g_get_tmp_dir(void)
{
}

/* <glib/glib.h> */
gchar* g_get_prgname(void)
{
}

/* <glib/glib.h> */
void g_set_prgname(const gchar *prgname)
{
}

/* <glib/glib.h> */
guint g_parse_debug_string(const gchar *string, GDebugKey *keys, guint nkeys)
{
}

/* <glib/glib.h> */
gint g_snprintf(gchar *string, gulong n, gchar const *format, ...)
{
}

/* <glib/glib.h> */
gint g_vsnprintf(gchar *string, gulong n, gchar const *format, va_list args)
{
}

/* <glib/glib.h> */
gchar* g_basename(const gchar *file_name)
{
}

/* <glib/glib.h> */
gboolean g_path_is_absolute(const gchar *file_name)
{
}

/* <glib/glib.h> */
gchar* g_path_skip_root(gchar *file_name)
{
}

/* <glib/glib.h> */
gchar* g_dirname(const gchar *file_name)
{
}

/* <glib/glib.h> */
gchar* g_get_current_dir(void)
{
}

/* <glib/glib.h> */
gchar* g_getenv(const gchar *variable)
{
}

/* <glib/glib.h> */
void g_atexit(GVoidFunc func)
{
}

/* <glib/glib.h> */
G_INLINE_FUNC gint g_bit_nth_lsf(guint32 mask, gint nth_bit)
{
}

/* <glib/glib.h> */
G_INLINE_FUNC gint g_bit_nth_msf(guint32 mask, gint nth_bit)
{
}

/* <glib/glib.h> */
G_INLINE_FUNC guint g_bit_storage(guint number)
{
}

/* <glib/glib.h> */
GStringChunk* g_string_chunk_new(gint size)
{
}

/* <glib/glib.h> */
void g_string_chunk_free(GStringChunk *chunk)
{
}

/* <glib/glib.h> */
gchar* g_string_chunk_insert(GStringChunk *chunk, const gchar *string)
{
}

/* <glib/glib.h> */
gchar* g_string_chunk_insert_const(GStringChunk *chunk, const gchar *string)
{
}

/* <glib/glib.h> */
GString* g_string_new(const gchar *init)
{
}

/* <glib/glib.h> */
GString* g_string_sized_new(guint dfl_size)
{
}

/* <glib/glib.h> */
void g_string_free(GString *string, gint free_segment)
{
}

/* <glib/glib.h> */
GString* g_string_assign(GString *lval, const gchar *rval)
{
}

/* <glib/glib.h> */
GString* g_string_truncate(GString *string, gint len)
{
}

/* <glib/glib.h> */
GString* g_string_append(GString *string, const gchar *val)
{
}

/* <glib/glib.h> */
GString* g_string_append_c(GString *string, gchar c)
{
}

/* <glib/glib.h> */
GString* g_string_prepend(GString *string, const gchar *val)
{
}

/* <glib/glib.h> */
GString* g_string_prepend_c(GString *string, gchar c)
{
}

/* <glib/glib.h> */
GString* g_string_insert(GString *string, gint pos, const gchar *val)
{
}

/* <glib/glib.h> */
GString* g_string_insert_c(GString *string, gint pos, gchar c)
{
}

/* <glib/glib.h> */
GString* g_string_erase(GString *string, gint pos, gint len)
{
}

/* <glib/glib.h> */
GString* g_string_down(GString *string)
{
}

/* <glib/glib.h> */
GString* g_string_up(GString *string)
{
}

/* <glib/glib.h> */
void g_string_sprintf(GString *string, const gchar *format, ...)
{
}

/* <glib/glib.h> */
void g_string_sprintfa(GString *string, const gchar *format, ...)
{
}

/* <glib/glib.h> */
#define g_array_append_val(a,v) g_array_append_vals (a, &(v), 1)

/* <glib/glib.h> */
#define g_array_prepend_val(a,v) g_array_prepend_vals (a, &(v), 1)

/* <glib/glib.h> */
#define g_array_insert_val(a,i,v) g_array_insert_vals (a, i, &(v), 1)

/* <glib/glib.h> */
#define g_array_index(a,t,i) (((t*) (a)->data) [(i)])

/* <glib/glib.h> */
GArray* g_array_new(gboolean zero_terminated, gboolean clear, guint element_size)
{
}

/* <glib/glib.h> */
void g_array_free(GArray *array, gboolean free_segment)
{
}

/* <glib/glib.h> */
GArray* g_array_append_vals(GArray *array, gconstpointer data, guint len)
{
}

/* <glib/glib.h> */
GArray* g_array_prepend_vals(GArray *array, gconstpointer data, guint len)
{
}

/* <glib/glib.h> */
GArray* g_array_insert_vals(GArray *array, guint index, gconstpointer data, guint len)
{
}

/* <glib/glib.h> */
GArray* g_array_set_size(GArray *array, guint length)
{
}

/* <glib/glib.h> */
GArray* g_array_remove_index(GArray *array, guint index)
{
}

/* <glib/glib.h> */
GArray* g_array_remove_index_fast(GArray *array, guint index)
{
}

/* <glib/glib.h> */
#define g_ptr_array_index(array,index) (array->pdata)[index]

/* <glib/glib.h> */
GPtrArray* g_ptr_array_new(void)
{
}

/* <glib/glib.h> */
void g_ptr_array_free(GPtrArray *array, gboolean free_seg)
{
}

/* <glib/glib.h> */
void g_ptr_array_set_size(GPtrArray *array, gint length)
{
}

/* <glib/glib.h> */
gpointer g_ptr_array_remove_index(GPtrArray *array, guint index)
{
}

/* <glib/glib.h> */
gpointer g_ptr_array_remove_index_fast(GPtrArray *array, guint index)
{
}

/* <glib/glib.h> */
gboolean g_ptr_array_remove(GPtrArray *array, gpointer data)
{
}

/* <glib/glib.h> */
gboolean g_ptr_array_remove_fast(GPtrArray *array, gpointer data)
{
}

/* <glib/glib.h> */
void g_ptr_array_add(GPtrArray *array, gpointer data)
{
}

/* <glib/glib.h> */
GByteArray* g_byte_array_new(void)
{
}

/* <glib/glib.h> */
void g_byte_array_free(GByteArray *array, gboolean free_segment)
{
}

/* <glib/glib.h> */
GByteArray* g_byte_array_append(GByteArray *array, const guint8 *data, guint len)
{
}

/* <glib/glib.h> */
GByteArray* g_byte_array_prepend(GByteArray *array, const guint8 *data, guint len)
{
}

/* <glib/glib.h> */
GByteArray* g_byte_array_set_size(GByteArray *array, guint length)
{
}

/* <glib/glib.h> */
GByteArray* g_byte_array_remove_index(GByteArray *array, guint index)
{
}

/* <glib/glib.h> */
GByteArray* g_byte_array_remove_index_fast(GByteArray *array, guint index)
{
}

/* <glib/glib.h> */
gint g_str_equal(gconstpointer v, gconstpointer v2)
{
}

/* <glib/glib.h> */
guint g_str_hash(gconstpointer v)
{
}

/* <glib/glib.h> */
gint g_int_equal(gconstpointer v, gconstpointer v2)
{
}

/* <glib/glib.h> */
guint g_int_hash(gconstpointer v)
{
}

/* <glib/glib.h> */
guint g_direct_hash(gconstpointer v)
{
}

/* <glib/glib.h> */
gint g_direct_equal(gconstpointer v, gconstpointer v2)
{
}

/* <glib/glib.h> */
GQuark g_quark_try_string(const gchar *string)
{
}

/* <glib/glib.h> */
GQuark g_quark_from_static_string(const gchar *string)
{
}

/* <glib/glib.h> */
GQuark g_quark_from_string(const gchar *string)
{
}

/* <glib/glib.h> */
gchar* g_quark_to_string(GQuark quark)
{
}

/* <glib/glib.h> */
void g_datalist_init(GData **datalist)
{
}

/* <glib/glib.h> */
void g_datalist_clear(GData **datalist)
{
}

/* <glib/glib.h> */
gpointer g_datalist_id_get_data(GData **datalist, GQuark key_id)
{
}

/* <glib/glib.h> */
void g_datalist_id_set_data_full(GData **datalist, GQuark key_id, gpointer data, GDestroyNotify destroy_func)
{
}

/* <glib/glib.h> */
void g_datalist_id_remove_no_notify(GData **datalist, GQuark key_id)
{
}

/* <glib/glib.h> */
void g_datalist_foreach(GData **datalist, GDataForeachFunc func, gpointer user_data)
{
}

/* <glib/glib.h> */
#define g_datalist_id_set_data(dl, q, d) ...

/* <glib/glib.h> */
#define g_datalist_id_remove_data(dl, q) ...

/* <glib/glib.h> */
#define g_datalist_get_data(dl, k) ...

/* <glib/glib.h> */
#define g_datalist_set_data_full(dl, k, d, f) ...

/* <glib/glib.h> */
#define g_datalist_remove_no_notify(dl, k) ...

/* <glib/glib.h> */
#define g_datalist_set_data(dl, k, d) ...

/* <glib/glib.h> */
#define g_datalist_remove_data(dl, k) ...

/* <glib/glib.h> */
void g_dataset_destroy(gconstpointer dataset_location)
{
}

/* <glib/glib.h> */
gpointer g_dataset_id_get_data(gconstpointer dataset_location, GQuark key_id)
{
}

/* <glib/glib.h> */
void g_dataset_id_set_data_full(gconstpointer dataset_location, GQuark key_id, gpointer data, GDestroyNotify destroy_func)
{
}

/* <glib/glib.h> */
void g_dataset_id_remove_no_notify(gconstpointer dataset_location, GQuark key_id)
{
}

/* <glib/glib.h> */
void g_dataset_foreach(gconstpointer dataset_location, GDataForeachFunc func, gpointer user_data)
{
}

/* <glib/glib.h> */
#define g_dataset_id_set_data(l, k, d) ...

/* <glib/glib.h> */
#define g_dataset_id_remove_data(l, k) ...

/* <glib/glib.h> */
#define g_dataset_get_data(l, k) ...

/* <glib/glib.h> */
#define g_dataset_set_data_full(l, k, d, f) ...

/* <glib/glib.h> */
#define g_dataset_remove_no_notify(l, k) ...

/* <glib/glib.h> */
#define g_dataset_set_data(l, k, d) ...

/* <glib/glib.h> */
#define g_dataset_remove_data(l, k) ...

/* <glib/glib.h> */
#define G_CSET_A_2_Z "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

/* <glib/glib.h> */
#define G_CSET_a_2_z "abcdefghijklmnopqrstuvwxyz"

/* <glib/glib.h> */
#define G_CSET_LATINC /* string of uppercase latin-1 characters */

/* <glib/glib.h> */
#define G_CSET_LATINS /* string of lowercase latin-1 characters */

/* <glib/glib.h> Error types */
typedef enum
{
  G_ERR_UNKNOWN,
  G_ERR_UNEXP_EOF,
  G_ERR_UNEXP_EOF_IN_STRING,
  G_ERR_UNEXP_EOF_IN_COMMENT,
  G_ERR_NON_DIGIT_IN_CONST,
  G_ERR_DIGIT_RADIX,
  G_ERR_FLOAT_RADIX,
  G_ERR_FLOAT_MALFORMED
} GErrorType;

/* <glib/glib.h> Token types */
typedef enum
{
  G_TOKEN_EOF			=   0,
  
  G_TOKEN_LEFT_PAREN		= '(',
  G_TOKEN_RIGHT_PAREN		= ')',
  G_TOKEN_LEFT_CURLY		= '{',
  G_TOKEN_RIGHT_CURLY		= '}',
  G_TOKEN_LEFT_BRACE		= '[',
  G_TOKEN_RIGHT_BRACE		= ']',
  G_TOKEN_EQUAL_SIGN		= '=',
  G_TOKEN_COMMA			= ',',
  
  G_TOKEN_NONE			= 256,
  
  G_TOKEN_ERROR,
  
  G_TOKEN_CHAR,
  G_TOKEN_BINARY,
  G_TOKEN_OCTAL,
  G_TOKEN_INT,
  G_TOKEN_HEX,
  G_TOKEN_FLOAT,
  G_TOKEN_STRING,
  
  G_TOKEN_SYMBOL,
  G_TOKEN_IDENTIFIER,
  G_TOKEN_IDENTIFIER_NULL,
  
  G_TOKEN_COMMENT_SINGLE,
  G_TOKEN_COMMENT_MULTI,
  G_TOKEN_LAST
} GTokenType;

/* <glib/glib.h> */
typedef union	_GTokenValue
{
  gpointer	v_symbol;
  gchar		*v_identifier;
  gulong	v_binary;
  gulong	v_octal;
  gulong	v_int;
  gdouble	v_float;
  gulong	v_hex;
  gchar		*v_string;
  gchar		*v_comment;
  guchar	v_char;
  guint		v_error;
} GTokenValue;

/* <glib/glib.h> */
typedef struct	_GScannerConfig
{
  /* Character sets
   */
  gchar		*cset_skip_characters;		/* default: " \t\n" */
  gchar		*cset_identifier_first;
  gchar		*cset_identifier_nth;
  gchar		*cpair_comment_single;		/* default: "#\n" */
  
  /* Should symbol lookup work case sensitive?
   */
  guint		case_sensitive : 1;
  
  /* Boolean values to be adjusted "on the fly"
   * to configure scanning behaviour.
   */
  guint		skip_comment_multi : 1;		/* C like comment */
  guint		skip_comment_single : 1;	/* single line comment */
  guint		scan_comment_multi : 1;		/* scan multi line comments? */
  guint		scan_identifier : 1;
  guint		scan_identifier_1char : 1;
  guint		scan_identifier_NULL : 1;
  guint		scan_symbols : 1;
  guint		scan_binary : 1;
  guint		scan_octal : 1;
  guint		scan_float : 1;
  guint		scan_hex : 1;			/* `0x0ff0' */
  guint		scan_hex_dollar : 1;		/* `$0ff0' */
  guint		scan_string_sq : 1;		/* string: 'anything' */
  guint		scan_string_dq : 1;		/* string: "\\-escapes!\n" */
  guint		numbers_2_int : 1;		/* bin, octal, hex => int */
  guint		int_2_float : 1;		/* int => G_TOKEN_FLOAT? */
  guint		identifier_2_string : 1;
  guint		char_2_token : 1;		/* return G_TOKEN_CHAR? */
  guint		symbol_2_token : 1;
  guint		scope_0_fallback : 1;		/* try scope 0 on lookups? */
} GScannerConfig;

/* <glib/glib.h> */
typedef struct	_GScanner
{
  /* unused fields */
  gpointer		user_data;
  guint			max_parse_errors;
  
  /* g_scanner_error() increments this field */
  guint			parse_errors;
  
  /* name of input stream, featured by the default message handler */
  const gchar		*input_name;
  
  /* data pointer for derived structures */
  gpointer		derived_data;
  
  /* link into the scanner configuration */
  GScannerConfig	*config;
  
  /* fields filled in after g_scanner_get_next_token() */
  GTokenType		token;
  GTokenValue		value;
  guint			line;
  guint			position;
  
  /* fields filled in after g_scanner_peek_next_token() */
  GTokenType		next_token;
  GTokenValue		next_value;
  guint			next_line;
  guint			next_position;
  
  /* to be considered private */
  GHashTable		*symbol_table;
  gint			input_fd;
  const gchar		*text;
  const gchar		*text_end;
  gchar			*buffer;
  guint			scope_id;
  
  /* handler function for _warn and _error */
  GScannerMsgFunc	msg_handler;
} GScanner;


/* <glib/glib.h> */
GScanner* g_scanner_new(GScannerConfig *config_templ)
{
}

/* <glib/glib.h> */
void g_scanner_destroy(GScanner *scanner)
{
}

/* <glib/glib.h> */
void g_scanner_input_file(GScanner *scanner, gint input_fd)
{
}

/* <glib/glib.h> */
void g_scanner_sync_file_offset(GScanner *scanner)
{
}

/* <glib/glib.h> */
void g_scanner_input_text(GScanner *scanner, const gchar *text, guint text_len)
{
}

/* <glib/glib.h> */
GTokenType g_scanner_get_next_token(GScanner *scanner)
{
}

/* <glib/glib.h> */
GTokenType g_scanner_peek_next_token(GScanner *scanner)
{
}

/* <glib/glib.h> */
GTokenType g_scanner_cur_token(GScanner *scanner)
{
}

/* <glib/glib.h> */
GTokenValue g_scanner_cur_value(GScanner *scanner)
{
}

/* <glib/glib.h> */
guint g_scanner_cur_line(GScanner *scanner)
{
}

/* <glib/glib.h> */
guint g_scanner_cur_position(GScanner *scanner)
{
}

/* <glib/glib.h> */
gboolean g_scanner_eof(GScanner *scanner)
{
}

/* <glib/glib.h> */
guint g_scanner_set_scope(GScanner *scanner, guint scope_id)
{
}

/* <glib/glib.h> */
void g_scanner_scope_add_symbol(GScanner *scanner, guint scope_id, const gchar *symbol, gpointer value)
{
}

/* <glib/glib.h> */
void g_scanner_scope_remove_symbol(GScanner *scanner, guint scope_id, const gchar *symbol)
{
}

/* <glib/glib.h> */
gpointer g_scanner_scope_lookup_symbol(GScanner *scanner, guint scope_id, const gchar *symbol)
{
}

/* <glib/glib.h> */
void g_scanner_scope_foreach_symbol(GScanner *scanner, guint scope_id, GHFunc func, gpointer user_data)
{
}

/* <glib/glib.h> */
gpointer g_scanner_lookup_symbol(GScanner *scanner, const gchar *symbol)
{
}

/* <glib/glib.h> */
void g_scanner_freeze_symbol_table(GScanner *scanner)
{
}

/* <glib/glib.h> */
void g_scanner_thaw_symbol_table(GScanner *scanner)
{
}

/* <glib/glib.h> */
void g_scanner_unexp_token(GScanner *scanner, GTokenType expected_token, const gchar *identifier_spec, const gchar *symbol_spec, const gchar *symbol_name, const gchar *message, gint is_error)
{
}

/* <glib/glib.h> */
void g_scanner_error(GScanner *scanner, const gchar *format, ...)
{
}

/* <glib/glib.h> */
void g_scanner_warn(GScanner *scanner, const gchar *format, ...)
{
}

/* <glib/glib.h> */
gint g_scanner_stat_mode(const gchar *filename)
{
}


/* <glib/glib.h> */
typedef struct _GCompletion
{
  GList* items;
  GCompletionFunc func;
  
  gchar* prefix;
  GList* cache;
} GCompletion;


/* <glib/glib.h> */
GCompletion* g_completion_new(GCompletionFunc func)
{
}

/* <glib/glib.h> */
void g_completion_add_items(GCompletion* cmp, GList* items)
{
}

/* <glib/glib.h> */
void g_completion_remove_items(GCompletion* cmp, GList* items)
{
}

/* <glib/glib.h> */
void g_completion_clear_items(GCompletion* cmp)
{
}

/* <glib/glib.h> */
GList* g_completion_complete(GCompletion* cmp, gchar* prefix, gchar** new_prefix)
{
}

/* <glib/glib.h> */
void g_completion_free(GCompletion* cmp)
{
}


/* <glib/glib.h> */
typedef guint16 GDateYear;

/* <glib/glib.h> */
typedef guint8  GDateDay;   /* day of the month */

/* <glib/glib.h> */
typedef struct _GDate GDate;

/* <glib/glib.h> enum used to specify order of appearance in parsed date strings */
typedef enum
{
  G_DATE_DAY   = 0,
  G_DATE_MONTH = 1,
  G_DATE_YEAR  = 2
} GDateDMY;

/* <glib/glib.h> actual week and month values */
typedef enum
{
  G_DATE_BAD_WEEKDAY  = 0,
  G_DATE_MONDAY       = 1,
  G_DATE_TUESDAY      = 2,
  G_DATE_WEDNESDAY    = 3,
  G_DATE_THURSDAY     = 4,
  G_DATE_FRIDAY       = 5,
  G_DATE_SATURDAY     = 6,
  G_DATE_SUNDAY       = 7
} GDateWeekday;

/* <glib/glib.h> months */
typedef enum
{
  G_DATE_BAD_MONTH = 0,
  G_DATE_JANUARY   = 1,
  G_DATE_FEBRUARY  = 2,
  G_DATE_MARCH     = 3,
  G_DATE_APRIL     = 4,
  G_DATE_MAY       = 5,
  G_DATE_JUNE      = 6,
  G_DATE_JULY      = 7,
  G_DATE_AUGUST    = 8,
  G_DATE_SEPTEMBER = 9,
  G_DATE_OCTOBER   = 10,
  G_DATE_NOVEMBER  = 11,
  G_DATE_DECEMBER  = 12
} GDateMonth;

/* <glib/glib.h> */
#define G_DATE_BAD_JULIAN 0U

/* <glib/glib.h> */
#define G_DATE_BAD_DAY    0U

/* <glib/glib.h> */
#define G_DATE_BAD_YEAR   0U

/* <glib/glib.h> Note: directly manipulating structs is generally a bad idea, but
 * in this case it's an *incredibly* bad idea, because all or part
 * of this struct can be invalid at any given time. Use the functions,
 * or you will get hosed, I promise.
 */
typedef struct _GDate
{
  guint julian_days : 32; /* julian days representation - we use a
                           *  bitfield hoping that 64 bit platforms
                           *  will pack this whole struct in one big
                           *  int 
                           */

  guint julian : 1;    /* julian is valid */
  guint dmy    : 1;    /* dmy is valid */

  /* DMY representation */
  guint day    : 6;  
  guint month  : 4; 
  guint year   : 16; 
} GDate;


/* <glib/glib.h> */
GDate* g_date_new(void)
{
}

/* <glib/glib.h> */
GDate* g_date_new_dmy(GDateDay day, GDateMonth month, GDateYear year)
{
}

/* <glib/glib.h> */
GDate* g_date_new_julian(guint32 julian_day)
{
}

/* <glib/glib.h> */
void g_date_free(GDate *date)
{
}

/* <glib/glib.h> */
gboolean g_date_valid(GDate *date)
{
}

/* <glib/glib.h> */
gboolean g_date_valid_day(GDateDay day)
{
}

/* <glib/glib.h> */
gboolean g_date_valid_month(GDateMonth month)
{
}

/* <glib/glib.h> */
gboolean g_date_valid_year(GDateYear year)
{
}

/* <glib/glib.h> */
gboolean g_date_valid_weekday(GDateWeekday weekday)
{
}

/* <glib/glib.h> */
gboolean g_date_valid_julian(guint32 julian_date)
{
}

/* <glib/glib.h> */
gboolean g_date_valid_dmy(GDateDay day, GDateMonth month, GDateYear year)
{
}

/* <glib/glib.h> */
GDateWeekday g_date_weekday(GDate *date)
{
}

/* <glib/glib.h> */
GDateMonth g_date_month(GDate *date)
{
}

/* <glib/glib.h> */
GDateYear g_date_year(GDate *date)
{
}

/* <glib/glib.h> */
GDateDay g_date_day(GDate *date)
{
}

/* <glib/glib.h> */
guint32 g_date_julian(GDate *date)
{
}

/* <glib/glib.h> */
guint g_date_day_of_year(GDate *date)
{
}

/* <glib/glib.h> */
guint g_date_monday_week_of_year(GDate *date)
{
}

/* <glib/glib.h> */
guint g_date_sunday_week_of_year(GDate *date)
{
}

/* <glib/glib.h> */
void g_date_clear (GDate *date, 

/* <glib/glib.h> */
void g_date_set_parse(GDate *date, const gchar *str)
{
}

/* <glib/glib.h> */
void g_date_set_time(GDate *date, GTime time)
{
}

/* <glib/glib.h> */
void g_date_set_month(GDate *date, GDateMonth month)
{
}

/* <glib/glib.h> */
void g_date_set_day(GDate *date, GDateDay day)
{
}

/* <glib/glib.h> */
void g_date_set_year(GDate *date, GDateYear year)
{
}

/* <glib/glib.h> */
void g_date_set_dmy(GDate *date, GDateDay day, GDateMonth month, GDateYear y)
{
}

/* <glib/glib.h> */
void g_date_set_julian(GDate *date, guint32 julian_date)
{
}

/* <glib/glib.h> */
gboolean g_date_is_first_of_month(GDate *date)
{
}

/* <glib/glib.h> */
gboolean g_date_is_last_of_month(GDate *date)
{
}

/* <glib/glib.h> */
void g_date_add_days(GDate *date, guint n_days)
{
}

/* <glib/glib.h> */
void g_date_subtract_days(GDate *date, guint n_days)
{
}

/* <glib/glib.h> */
void g_date_add_months(GDate *date, guint n_months)
{
}

/* <glib/glib.h> */
void g_date_subtract_months(GDate *date, guint n_months)
{
}

/* <glib/glib.h> */
void g_date_add_years(GDate *date, guint n_years)
{
}

/* <glib/glib.h> */
void g_date_subtract_years(GDate *date, guint n_years)
{
}

/* <glib/glib.h> */
gboolean g_date_is_leap_year(GDateYear year)
{
}

/* <glib/glib.h> */
guint8 g_date_days_in_month(GDateMonth month, GDateYear year)
{
}

/* <glib/glib.h> */
guint8 g_date_monday_weeks_in_year(GDateYear year)
{
}

/* <glib/glib.h> */
guint8 g_date_sunday_weeks_in_year(GDateYear year)
{
}

/* <glib/glib.h> */
gint g_date_compare(GDate *lhs, GDate *rhs)
{
}

/* <glib/glib.h> */
void g_date_to_struct_tm(GDate *date, struct tm *tm)
{
}

/* <glib/glib.h> */
gsize g_date_strftime(gchar *s, gsize slen, const gchar *format, GDate *date)
{
}

/* <glib/glib.h> */
GRelation* g_relation_new(gint fields)
{
}

/* <glib/glib.h> */
void g_relation_destroy(GRelation *relation)
{
}

/* <glib/glib.h> */
void g_relation_index(GRelation *relation, gint field, GHashFunc hash_func, GCompareFunc key_compare_func)
{
}

/* <glib/glib.h> */
void g_relation_insert(GRelation *relation, ...)
{
}

/* <glib/glib.h> */
gint g_relation_delete(GRelation *relation, gconstpointer key, gint field)
{
}

/* <glib/glib.h> */
GTuples* g_relation_select(GRelation *relation, gconstpointer key, gint field)
{
}

/* <glib/glib.h> */
gint g_relation_count(GRelation *relation, gconstpointer key, gint field)
{
}

/* <glib/glib.h> */
gboolean g_relation_exists(GRelation *relation, ...)
{
}

/* <glib/glib.h> */
void g_relation_print(GRelation *relation)
{
}

/* <glib/glib.h> */
void g_tuples_destroy(GTuples *tuples)
{
}

/* <glib/glib.h> */
gpointer g_tuples_index(GTuples *tuples, gint index, gint field)
{
}

/* <glib/glib.h> */
guint g_spaced_primes_closest(guint num)
{
}


/* <glib/glib.h> */
typedef enum
{
  G_IO_ERROR_NONE,
  G_IO_ERROR_AGAIN,
  G_IO_ERROR_INVAL,
  G_IO_ERROR_UNKNOWN
} GIOError;

/* <glib/glib.h> */
typedef enum
{
  G_SEEK_CUR,
  G_SEEK_SET,
  G_SEEK_END
} GSeekType;

/* <glib/glib.h> */
typedef enum
{
  G_IO_IN	GLIB_SYSDEF_POLLIN,
  G_IO_OUT	GLIB_SYSDEF_POLLOUT,
  G_IO_PRI	GLIB_SYSDEF_POLLPRI,
  G_IO_ERR	GLIB_SYSDEF_POLLERR,
  G_IO_HUP	GLIB_SYSDEF_POLLHUP,
  G_IO_NVAL	GLIB_SYSDEF_POLLNVAL
} GIOCondition;


/* <glib/glib.h> */
typedef struct _GIOChannel
{
  guint channel_flags;
  guint ref_count;
  GIOFuncs *funcs;
} GIOChannel;

typedef gboolean (*GIOFunc) (GIOChannel *source, GIOCondition condition, gpointer data);
typedef struct _GIOFuncs
{
 GIOError (*io_read) (GIOChannel *channel, gchar *buf, guint count, guint *bytes_read);
 GIOError (*io_write) (GIOChannel *channel, gchar *buf, guint count, guint *bytes_written);
 GIOError (*io_seek) (GIOChannel *channel, gint offset, GSeekType type); void (*io_close) (GIOChannel *channel);
 guint (*io_add_watch) (GIOChannel *channel, gint priority, GIOCondition condition, GIOFunc func, gpointer user_data, GDestroyNotify notify);
 void (*io_free) (GIOChannel *channel);
} GIOFuncs;


/* <glib/glib.h> */
void g_io_channel_init(GIOChannel *channel)
{
}

/* <glib/glib.h> */
void g_io_channel_ref(GIOChannel *channel)
{
}

/* <glib/glib.h> */
void g_io_channel_unref(GIOChannel *channel)
{
}

/* <glib/glib.h> */
GIOError g_io_channel_read(GIOChannel *channel, gchar *buf, guint count, guint *bytes_read)
{
}

/* <glib/glib.h> */
GIOError g_io_channel_write(GIOChannel *channel, gchar *buf, guint count, guint *bytes_written)
{
}

/* <glib/glib.h> */
GIOError g_io_channel_seek(GIOChannel *channel, gint offset, GSeekType type)
{
}

/* <glib/glib.h> */
void g_io_channel_close(GIOChannel *channel)
{
}

/* <glib/glib.h> */
guint g_io_add_watch_full(GIOChannel *channel, gint priority, GIOCondition condition, GIOFunc func, gpointer user_data, GDestroyNotify notify)
{
}

/* <glib/glib.h> */
guint g_io_add_watch(GIOChannel *channel, GIOCondition condition, GIOFunc func, gpointer user_data)
{
}


/* <glib/glib.h> */
typedef struct _GTimeVal
{
  glong tv_sec;
  glong tv_usec;
} GTimeVal;

/* <glib/glib.h> */
typedef struct _GSourceFuncs
{
 gboolean (*prepare) (gpointer source_data, GTimeVal *current_time, gint *timeout, gpointer user_data);
 gboolean (*check) (gpointer source_data, GTimeVal *current_time, gpointer user_data);
 gboolean (*dispatch) (gpointer source_data, GTimeVal *dispatch_time, gpointer user_data);
 GDestroyNotify destroy;
} GSourceFuncs;

/* <glib/glib.h> */
#define G_PRIORITY_HIGH            -100

/* <glib/glib.h> */
#define G_PRIORITY_DEFAULT          0

/* <glib/glib.h> */
#define G_PRIORITY_HIGH_IDLE        100

/* <glib/glib.h> */
#define G_PRIORITY_DEFAULT_IDLE     200

/* <glib/glib.h> */
#define G_PRIORITY_LOW	            300


/* <glib/glib.h> */
typedef gboolean(*GSourceFunc) (gpointer data)
{
}

guint g_source_add(gint priority, gboolean can_recurse, GSourceFuncs *funcs, gpointer source_data, gpointer user_data, GDestroyNotify notify)
{
}

/* <glib/glib.h> */
gboolean g_source_remove(guint tag)
{
}

/* <glib/glib.h> */
gboolean g_source_remove_by_user_data(gpointer user_data)
{
}

/* <glib/glib.h> */
gboolean g_source_remove_by_source_data(gpointer source_data)
{
}

/* <glib/glib.h> */
gboolean g_source_remove_by_funcs_user_data(GSourceFuncs *funcs, gpointer user_data)
{
}

/* <glib/glib.h> */
void g_get_current_time(GTimeVal *result)
{
}

/* <glib/glib.h> */
GMainLoop* g_main_new(gboolean is_running)
{
}

/* <glib/glib.h> */
void g_main_run(GMainLoop *loop)
{
}

/* <glib/glib.h> */
void g_main_quit(GMainLoop *loop)
{
}

/* <glib/glib.h> */
void g_main_destroy(GMainLoop *loop)
{
}

/* <glib/glib.h> */
gboolean g_main_is_running(GMainLoop *loop)
{
}

/* <glib/glib.h> */
gboolean g_main_iteration(gboolean may_block)
{
}

/* <glib/glib.h> */
gboolean g_main_pending(void)
{
}

/* <glib/glib.h> */
guint g_timeout_add_full(gint priority, guint interval, GSourceFunc function, gpointer data, GDestroyNotify notify)
{
}

/* <glib/glib.h> */
guint g_timeout_add(guint interval, GSourceFunc function, gpointer data)
{
}

/* <glib/glib.h> */
guint g_idle_add(GSourceFunc function, gpointer data)
{
}

/* <glib/glib.h> */
guint g_idle_add_full(gint priority, GSourceFunc function, gpointer data, GDestroyNotify destroy)
{
}

/* <glib/glib.h> */
gboolean g_idle_remove_by_data(gpointer data)
{
}

/* <glib/glib.h> */
typedef gint(*GPollFunc) (GPollFD *ufds, guint nfsd, gint timeout)
{
}

/* <glib/glib.h> */
typedef struct _GPollFD
{
  gint		fd;
  gushort 	events;
  gushort 	revents;
} GPollFD;


/* <glib/glib.h> */
void g_main_add_poll(GPollFD *fd, gint priority)
{
}

/* <glib/glib.h> */
void g_main_remove_poll(GPollFD *fd)
{
}

/* <glib/glib.h> */
void g_main_set_poll_func(GPollFunc func)
{
}

/* <glib/glib.h> */
GIOChannel* g_io_channel_unix_new(int fd)
{
}

/* <glib/glib.h> */
gint g_io_channel_unix_get_fd(GIOChannel *channel)
{
}


/* <glib/glib.h> */
typedef struct _GThreadFunctions
{
  GMutex*  (*mutex_new)       (void);
  void     (*mutex_lock)      (GMutex *mutex);
  gboolean (*mutex_trylock)   (GMutex *mutex);
  void     (*mutex_unlock)    (GMutex *mutex);
  void     (*mutex_free)      (GMutex *mutex);
  GCond*   (*cond_new)        (void);
  void     (*cond_signal)     (GCond *cond);
  void     (*cond_broadcast)  (GCond *cond);
  void     (*cond_wait)       (GCond *cond, GMutex *mutex);
  gboolean (*cond_timed_wait) (GCond *cond, GMutex *mutex, GTimeVal *end_time);
  void      (*cond_free)      (GCond*cond);
  GPrivate* (*private_new)    (GDestroyNotify destructor);
  gpointer  (*private_get)    (GPrivate *private_key);
  void      (*private_set)    (GPrivate *private_key, gpointer data);
} GThreadFunctions;


/* <glib/glib.h> */
GUTILS_C_VAR GThreadFunctions	g_thread_functions_for_glib_use;

/* <glib/glib.h> */
GUTILS_C_VAR gboolean		g_thread_use_default_impl;

/* <glib/glib.h> */
GUTILS_C_VAR gboolean		g_threads_got_initialized;


/* <glib/glib.h> */
void g_thread_init(GThreadFunctions *vtable)
{
}

/* <glib/glib.h> */
GMutex* g_static_mutex_get_mutex_impl(GMutex **mutex)
{
}

/* <glib/glib.h> */
#define G_THREAD_UF(name, arglist) ...

/* <glib/glib.h> */
#define G_THREAD_CF(name, fail, arg) ...

/* <glib/glib.h> */
#define	g_thread_supported() ...

/* <glib/glib.h> */
#define g_mutex_new() ...

/* <glib/glib.h> */
#define g_mutex_lock(mutex) ...

/* <glib/glib.h> */
#define g_mutex_trylock(mutex) ...

/* <glib/glib.h> */
#define g_mutex_unlock(mutex) ...

/* <glib/glib.h> */
#define g_mutex_free(mutex) ...

/* <glib/glib.h> */
#define g_cond_new() ...

/* <glib/glib.h> */
#define g_cond_signal(cond) ...

/* <glib/glib.h> */
#define g_cond_broadcast(cond) ...

/* <glib/glib.h> */
#define g_cond_wait(cond, mutex) ...

/* <glib/glib.h> */
#define g_cond_free(cond) ...

/* <glib/glib.h> */
#define g_cond_timed_wait(cond, mutex, abs_time) ...

/* <glib/glib.h> */
#define g_private_new(destructor) ...

/* <glib/glib.h> */
#define g_private_get(private_key) ...

/* <glib/glib.h> */
#define g_private_set(private_key, value) ...

/* <glib/glib.h> */
#define g_static_mutex_lock(mutex) ...

/* <glib/glib.h> */
#define g_static_mutex_trylock(mutex) ...

/* <glib/glib.h> */
#define g_static_mutex_unlock(mutex) ...


/* <glib/glib.h> */
typedef struct _GStaticPrivate
{
  guint index;
} GStaticPrivate;

/* <glib/glib.h> */
#define G_STATIC_PRIVATE_INIT { 0 }

/* <glib/glib.h> */
gpointer g_static_private_get(GStaticPrivate *private_key)
{
}

/* <glib/glib.h> */
void g_static_private_set(GStaticPrivate *private_key, gpointer data, GDestroyNotify notify)
{
}

/* <glib/glib.h> */
extern void glib_dummy_decl(void)
{
}

/* <glib/glib.h> */
#define G_LOCK_NAME(name) ...

/* <glib/glib.h> */
#define G_LOCK_DEFINE_STATIC(name) ...

/* <glib/glib.h> */
#define G_LOCK_DEFINE(name) ...

/* <glib/glib.h> */
#define G_LOCK_EXTERN(name) ...

/* <glib/glib.h> */
#define G_LOCK(name) ...

/* <glib/glib.h> */
#define G_UNLOCK(name) ...

/* <glib/glib.h> */
#define G_TRYLOCK(name) ...

/* <glib/gmodule.h> declaration for external symbols */
#define	G_MODULE_IMPORT		extern

/* <glib/gmodule.h> */
typedef enum
{
  G_MODULE_BIND_LAZY	= 1 << 0,
  G_MODULE_BIND_MASK	= 0x01
} GModuleFlags;

/* <glib/gmodule.h> */
typedef const gchar* (*GModuleCheckInit) (GModule *module);

/* <glib/gmodule.h> */
typedef void (*GModuleUnload) (GModule *module);


/* <glib/gmodule.h> */
gboolean g_module_supported(void)
{
}

/* <glib/gmodule.h> */
GModule* g_module_open(const gchar *file_name, GModuleFlags flags)
{
}

/* <glib/gmodule.h> */
gboolean g_module_close(GModule *module)
{
}

/* <glib/gmodule.h> */
void g_module_make_resident(GModule *module)
{
}

/* <glib/gmodule.h> */
gchar* g_module_error(void)
{
}

/* <glib/gmodule.h> */
gboolean g_module_symbol(GModule *module, const gchar *symbol_name, gpointer *symbol)
{
}

/* <glib/gmodule.h> */
gchar* g_module_name(GModule *module)
{
}

/* <glib/gmodule.h> */
gchar* g_module_build_path(const gchar *directory, const gchar *module_name)
{
}
