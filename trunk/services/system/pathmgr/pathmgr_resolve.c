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

#include "externs.h"
#include "pathmgr_node.h"
#include "pathmgr_object.h"
#include "pathmgr_proto.h"
#include <sys/pathmgr.h>

struct node_entry *pathmgr_resolve_path(struct node_entry *node, struct _io_connect *connect, char *path, struct node_entry *root) {
	register char						*p1, *p2;
	register struct node_entry			*locked_nop;
	register unsigned					eflag;

	eflag = 0;
	p2 = 0;
	p1 = path;
	locked_nop = 0;

	for(;;) {
		// remove extra slashes
		if(p1[0] == '/') {
			eflag |= _IO_CONNECT_EFLAG_DIR;
			eflag &= ~(_IO_CONNECT_EFLAG_DOT | _IO_CONNECT_EFLAG_DOTDOT);
			while(p1[0] == '/') {
				p1++;
			}
		}

		// check for dot and dotdot
		if(p1[0] == '.') {
			if(p1[1] == '.') {
				if(p1[2] == '/' || p1[2] == '\0') {
					// dotdot so go back a directory
					if(p2) {
						// go back through path string
						p2 -= 2;
						while(p2[-1] != '/') {
							if(p2-- == path) {
								p2 = 0;
								break;
							}
						}
					} else {
						// if the process root is not the node root, don't allow escaping
						if(node != root) {
							// go back through nodes
							node = node->parent;
						}
					}
					p1 += 2;
					eflag |= _IO_CONNECT_EFLAG_DIR | _IO_CONNECT_EFLAG_DOT | _IO_CONNECT_EFLAG_DOTDOT;
					continue;
				}
			} else if(p1[1] == '/' || p1[1] == '\0') {
				// found dot so remove it
				p1++;
				eflag |= _IO_CONNECT_EFLAG_DIR | _IO_CONNECT_EFLAG_DOT;
				continue;
			}
		}

		if(*p1) {
			// more to the path, doesn't end in dot or slash
			eflag &= ~(_IO_CONNECT_EFLAG_DIR | _IO_CONNECT_EFLAG_DOT | _IO_CONNECT_EFLAG_DOTDOT);
			if(!p2) {
				register struct node_entry			*n;

				if((n = pathmgr_node_lookup(node, p1, PATHMGR_LOOKUP_PRUNE | PATHMGR_LOOKUP_ATTACH, 0))) {
					if(locked_nop) {
						// release the previous locked nop
						pathmgr_node_detach(locked_nop);
					}
					locked_nop = node = n;
					p1 += node->len;
				} else {
					p2 = path;
				}
			}
		} 

		if(p2) {
			// write path string for component
			while((p2[0] = p1[0])) {
				p2++;
				if(p2[-1] == '/') {
					break;
				}
				p1++;
			}
		}

		if(p1[0] == '\0') {
			break;
		}
	}
	if(p2) {
		// if path ends in a slash, flag it and remove the slash
		if(p2[-1] == '/') {
			eflag |= _IO_CONNECT_EFLAG_DIR;
			*--p2 = '\0';
		}
	} else {
		// everythins was in the nodes
		*path = '\0';
	}
	if(connect) {
		connect->eflag |= eflag;
	}

	if(node != locked_nop) {
		// Cause the node to be locked
		(void)pathmgr_node_lookup(node, "", PATHMGR_LOOKUP_ATTACH, 0);

		if(locked_nop) {
			pathmgr_node_detach(locked_nop);
		}
	}

	// at this point, "node" has the lowest node,
	// and "path" has the remaining path without dots or extra slashes.
	// connect->eflag is set correctly
	return node;
}

#define _PMFLAG_SEEN_PATHMGR 0x0001
#define _PMFLAG_SEEN_ROOT	 0x0002
#define _PMFLAG_SEEN_OPAQUE	 0x0004
#define _PMFLAG_IS_LINK		 0x0008
int pathmgr_resolve_servers(resmgr_context_t *ctp, struct node_entry *node, struct _io_connect *connect, const char *path, struct node_entry *root) {
	struct node_entry				*n;
	char							*reply;
	struct _io_connect_entry		*server, *s, *snode_lookup;
	unsigned						len;
	unsigned						pos;
	unsigned						eflag;
	unsigned						local_flags;
	struct _io_connect_link_reply	*linkp;
	int								file_type;
	int								server_max_space;
	PROCESS							*prp;
	unsigned						ret;

	eflag = connect->eflag;
	file_type = connect->file_type;
	local_flags = 0;

	if(connect->type == _IO_CONNECT && 
	   connect->subtype == _IO_CONNECT_LINK && connect->extra_type == _IO_CONNECT_EXTRA_RESMGR_LINK) {
	   local_flags |= _PMFLAG_IS_LINK;
	}
	
	for(len = 1, n = node; n != n->parent; n = n->parent) {
		len += n->len + 1;
	}
	reply = (char *)connect + ((offsetof(struct _io_connect, path) + connect->path_len + connect->extra_len + len + _MALLOC_ALIGN - 1) & ~(_MALLOC_ALIGN - 1));

	server_max_space = min((char *)reply + connect->reply_max, (char *)connect + ctp->msg_max_size) - (char *)reply;

	if(server_max_space <= 0) {					//Early check to see if we don't have enough space
		return ENAMETOOLONG;
	}

	s = server = (struct _io_connect_entry *)reply;
	linkp = (struct _io_connect_link_reply *)(void *)connect;

	if(path[0]) {
		*--reply = '/';
	} else {
		len--;
	}
	pos = len;
	linkp->path_len = strlen(path) + len + 1;

	linkp->eflag = eflag;
	linkp->nentries = 0;
	linkp->chroot_len = 0;

	if(proc_thread_pool_reserve() != 0) {
		return EAGAIN;
	}

	for(n = node;; n = n->parent) {
		union object						*o;

		pathmgr_node_access(n);

		snode_lookup = NULL;
		for(o = n->object; o; o = o->hdr.next) {		
#ifndef NDEBUG
			if(n->object == NULL) {
				crash();
			}
#endif
			/* Only the pathmanager resolves after an OPAQUE entry, and only 
			   one pathmanager entry should be reported else we get duplicates */
			if (local_flags & _PMFLAG_SEEN_OPAQUE) { 
				if (o->hdr.type != OBJECT_SERVER || (local_flags & _PMFLAG_SEEN_PATHMGR) || 
				    o->server.pid != PATHMGR_PID || o->server.handle != root_id) {
					continue;
				}
			}

			if(o->hdr.type == OBJECT_SERVER) {
				if(		(!(local_flags & _PMFLAG_IS_LINK) || o->server.pid == PATHMGR_PID) &&
						(path[0] == '\0' || (o->hdr.flags & PATHMGR_FLAG_DIR)) &&
						(o->server.pid != ctp->info.pid ||
							ND_NODE_CMP(o->server.nd, ctp->info.nd) ||
							(o->hdr.flags & PATHMGR_FLAG_SELF)) ) {
					if((eflag & _IO_CONNECT_EFLAG_DIR) && !(o->hdr.flags & PATHMGR_FLAG_DIR)) {
						pathmgr_node_complete(n);
						proc_thread_pool_reserve_done();
						return ENOTDIR;
					}

					//Check to see if we have the right file_type info
					if (o->hdr.flags & PATHMGR_FLAG_FTYPEONLY && 
					    o->server.file_type != file_type) {
							continue;
					}
					
					if(server_max_space < sizeof(*s)) {
						pathmgr_node_complete(n);
						proc_thread_pool_reserve_done();
						return ENAMETOOLONG;
					}
					server_max_space -= sizeof(*s);
					
					if(o->hdr.flags & PATHMGR_FLAG_OPAQUE) {
						local_flags |= _PMFLAG_SEEN_OPAQUE;
					}
					linkp->nentries++;

					// this will get resolved below, outside the locked region
					if (ND_NODE_CMP(ctp->info.nd, ND_LOCAL_NODE) == 0) {
						s->nd = o->server.nd;
					} else if(ND_NODE_CMP(ctp->info.nd, o->server.nd) == 0) {
						s->nd = ND_LOCAL_NODE;
					} else {
						s->nd = o->server.nd;
						if (snode_lookup == NULL) {
							snode_lookup = s;
						}
					}
					s->pid = o->server.pid;
					s->chid = o->server.chid;
					s->handle = o->server.handle;
					s->file_type = o->server.file_type;
					s->prefix_len = pos;
					s->key = 0;
					s++;
				}
			} 

			//Always replace the last symlink as we climb up the tree
			else {
				if(server_max_space < sizeof(*s)) {
					pathmgr_node_complete(n);
					proc_thread_pool_reserve_done();
					return ENAMETOOLONG;
				}
				server_max_space -= sizeof(*s);

				/* 
				 Proc symlinks are like any other OPAQUE entry, except for the
				 fact that they re-direct to the link_root_id handle of the
				 pathmanager. OPAQUE entries stop all entries underneath
				 them (except the pathmanager) from being reported.
				*/
				local_flags |= _PMFLAG_SEEN_OPAQUE;
				linkp->nentries++;

				/*
				seen_pathmgr = (o->hdr.type == OBJECT_PROC_SYMLINK) ? seen_pathmgr : 1;
				*/
				if(o->hdr.type != OBJECT_PROC_SYMLINK) {
					local_flags |= _PMFLAG_SEEN_PATHMGR;
				}

				s->nd = ctp->info.srcnd; // netmgr_remote_nd(ctp->info.nd, ND_LOCAL_NODE);
				s->pid = PATHMGR_PID;
				s->chid = PATHMGR_CHID;
				s->file_type = file_type;
				s->handle = (o->hdr.type == OBJECT_PROC_SYMLINK) ? link_root_id : root_id;
				s->prefix_len = 1;
				s->key = 0;
				s++;
			}
		}

		pathmgr_node_complete(n);

		//Perform nd lookups outside of node locks, but avoid translating the
		//proc symlink entries with some weak heuristics.
		if(snode_lookup) {
			do {
				if (ND_NODE_CMP(ctp->info.srcnd, snode_lookup->nd) == 0) {
					snode_lookup++;
					continue;
				}
				snode_lookup->nd = netmgr_remote_nd(ctp->info.nd, snode_lookup->nd);
				snode_lookup++;
			} while(snode_lookup != s);
		}

		if(n == root) {
			local_flags |= _PMFLAG_SEEN_ROOT;
		}

		if(n->len) {
			reply -= n->len;
			memcpy(reply, n->name, n->len);
			*--reply = '/';
			pos -= n->len;
			if(n != node || path[0]) {
				pos--;
			}
			if(local_flags & _PMFLAG_SEEN_ROOT) {
				linkp->chroot_len += n->len + 1;
			}
		}

		if(n == n->parent) {
			break;
		} 

	}

	proc_thread_pool_reserve_done();

	ret = _IO_CONNECT_RET_LINK | _IO_CONNECT_RET_CHROOT;
	if((prp = proc_lock_pid(ctp->info.pid))) {
		linkp->umask = prp->umask;
		ret |= _IO_CONNECT_RET_UMASK;
		if(prp->session && prp->session->leader == prp->pid && prp->session->fd == -1) {
			ret |= _IO_CONNECT_RET_NOCTTY;
		}
		proc_unlock(prp);
	}
	_IO_SET_CONNECT_RET(ctp, ret);

	SETIOV(ctp->iov + 0, linkp, sizeof *linkp);
	SETIOV(ctp->iov + 1, server, sizeof *server * linkp->nentries);
	SETIOV(ctp->iov + 2, reply, len);
	SETIOV(ctp->iov + 3, path, linkp->path_len - len);
	return _RESMGR_NPARTS(4);
}	


__SRCVERSION("pathmgr_resolve.c $Rev: 153052 $");
