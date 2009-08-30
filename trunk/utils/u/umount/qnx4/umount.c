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



 
#ifdef __USAGE
%C - unmount filesystems or partitions (QNX)

%C  filename
Where:
'filename' should be the name of the mounted block special file
    or
'filename' should be the mount point of a block special file
           which is currently accessible
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/fsys.h>
#include <netdb.h>
#include <unix.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include "/src/os/bsd/nfs/rpcv2.h"
#include "/src/os/bsd/nfs/nfsv2.h"

#define HOST_LEN	30

int	tell_remote_mountd( char *host, char *path );

main(int argc, char *argv[]){
char	mount_pt[RPCMNT_PATHLEN + HOST_LEN];	// contains     host:/path
char	*remote_path;				// points to---------^

	if(argc < 2)
		{
		printf("No file specified\n");
		exit(1);
		}

	if(fsys_get_mount_dev( argv[1], mount_pt))
		memset( mount_pt, 0, RPCMNT_PATHLEN + HOST_LEN );

	if(umount(argv[1]) == -1)
		{
		perror("umount failed");
		exit(EXIT_FAILURE);
		}

	remote_path = strchr( mount_pt, ':' );
	if(remote_path){
		*remote_path = '\0';
		exit( tell_remote_mountd( mount_pt, ++remote_path ));
		}	
		
	exit(EXIT_SUCCESS);
	}

int xdr_dir();
int tell_remote_mountd( char *host, char *path ){

register CLIENT *clp;
struct hostent *hp;
static struct sockaddr_in saddr;
struct timeval pertry, try;
enum clnt_stat clnt_stat;
int so = RPC_ANYSOCK;
u_short tport;
int	retrycnt = 10;

//printf("remote filesystem detected: host = [%s]  path = [%s]\n",
//	host, path);

if ((hp = gethostbyname(host)) == NULL) {
	(void) fprintf(stderr, "umount: Can't get net id for host\n");
	return (EXIT_FAILURE);
	}
bcopy((char *)hp->h_addr, (char *)&saddr.sin_addr, hp->h_length);
while (retrycnt > 0) {
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PMAPPORT);
	if ((tport = pmap_getport(&saddr, RPCPROG_NFS, NFS_VER2, IPPROTO_UDP)) == 0) {
		clnt_pcreateerror("NFS Portmap");
		//exit(EXIT_FAILURE);
		}
	else {
		saddr.sin_port = 0;
		pertry.tv_sec = 10;
		pertry.tv_usec = 0;
		if ((clp = clntudp_create(&saddr, RPCPROG_MNT, RPCMNT_VER1, pertry, &so)) == NULL) {
			clnt_pcreateerror("Cannot MNT PRC");
			//exit(EXIT_FAILURE);
			}
		else {
			clp->cl_auth = authunix_create_default();
			try.tv_sec = 10;
			try.tv_usec = 0;
			clnt_stat = clnt_call(clp, RPCMNT_UMOUNT, xdr_dir, path, xdr_void, 0, try);
			if (clnt_stat != RPC_SUCCESS) {
				clnt_perror(clp, "Bad MNT RPC");
				//exit(EXIT_FAILURE);
				}
			else {
				auth_destroy(clp->cl_auth);
				clnt_destroy(clp);
				return (EXIT_SUCCESS);
				}
			}
		}
	retrycnt--;
	sleep(3);
	}

return (EXIT_FAILURE);
}

/*
 * xdr routines for mount rpc's
 */
xdr_dir(xdrsp, dirp)
	XDR *xdrsp;
	char *dirp;
{
	return (xdr_string(xdrsp, &dirp, RPCMNT_PATHLEN));
}

