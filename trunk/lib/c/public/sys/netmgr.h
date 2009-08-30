/*
 * $QNXLicenseC: $
*/


#ifndef _NETMGR_H_INCLUDED
#define _NETMGR_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

_C_STD_BEGIN

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

_C_STD_END

#define	ND_LOCAL_NODE		0
#define ND_NODE_BITS		16
#define ND_NODE_MASK		((1<<ND_NODE_BITS)-1)
#define ND_NODE_CMP(a,b)	(((a)&ND_NODE_MASK) - ((b)&ND_NODE_MASK))

#define ND2S_DIR_SHOW		0x0001	/* Returned value will be a usable path (ending in slash) */
#define ND2S_DIR_HIDE		0x0002	/* Never show the directory (default) */
#define ND2S_QOS_SHOW		0x0004	/* Always show QOS string even if it is the default QOS */
#define ND2S_QOS_HIDE		0x0008	/* Never show the QOS */
#define ND2S_NAME_SHOW		0x0010	/* Always show the node name (default) */
#define ND2S_NAME_HIDE		0x0020	/* Never show the node name */
#define ND2S_DOMAIN_SHOW	0x0040	/* Always show the domain even if it is the default domain */
#define ND2S_DOMAIN_HIDE	0x0080	/* Never show the domain */
#define ND2S_LOCAL_STR		0x1000	/* Used to return smaller string for displaying localy */
#define ND2S_SEP_FORCE		0x2000	/* Always return a seperator even if it is the first thing returned */

extern int		netmgr_remote_nd(int __remote_nd, int __local_nd);
extern int 		netmgr_ndtostr(unsigned __flags, int __nd, char *__buf,
								_CSTD size_t __maxbuf);
extern int 		netmgr_strtond(const char *__nodename, char **__endstr);
extern int 		netmgr_path(const char *__netname, const char *__suffix, char *__path,  _CSTD size_t __path_max);

/* process manager private interface */
extern int		netmgr_ctl(int __nd, int __fop);

__END_DECLS

#endif
