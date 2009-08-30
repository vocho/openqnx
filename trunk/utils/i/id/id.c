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
 *
 *-----------------------------------------------------------------------------
 * Filename:    id.c
 *
 * Author:              Jeffrey G. George
 *
 * Change                By                       Date                  Reason for Change
 * 0                    JGG                     01-Feb-90               First Release
 *              EAJ         18-Jul-91       comply with 1003.2 draft 11
 *-----------------------------------------------------------------------------
 * System :     Posix Utility Set
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>

/* Define declarations */
#define TRUE                            01
#define FALSE                           00
#define ID_ERROR                        1


/*
 *- ---------------------------------------------------------------------------
 */

#define TXT(x)  (x)
#define T_BAD_ARGS "id: Invalid combination of options.\n"

#ifdef __USAGE
%C - return user ID (POSIX)

%C      [user]
%C      -G [-n] [user]
%C      -g [-nr] [user]
%C      -u [-nr] [user]
Options:
 -G       Output all different group IDs (effective and real)
		  only, as unsigned integers.
 -g       Output only the effective group ID, as an unsigned integer.
 -n       Output the ids in their character string form instead of
	  unsigned integers.
 -r       Output the real ID instead of the effective ID.
 -u       Output only the effective user ID, as an unsigned integer.
#endif

int isallnumeric(char *s1) 
{
	while ((*s1<='9') && (*s1>='0')) s1++;
	return (*s1==0);
}

int main( int argc, char **argv )
{
	int                     i, option_given, egroup_only, euser_only, all_group;
	int                     real_id, text_id;
	int						ngroups=0;
	gid_t					groups[NGROUPS_MAX+1];
	
	uid_t                   uid, euid;
	gid_t                   gid, egid;
	char                    *userid, *uname, *euname;
	char                    *gname, *egname;
	struct passwd   *user_data;
	struct group    *group_data;
		       
	egroup_only = text_id = real_id = euser_only = all_group = FALSE;
	option_given = FALSE;

	while ((i=getopt(argc,argv,"Ggnru")) != -1) {
		option_given++;
		switch (i) {
			case 'G':   all_group = 1;                      break;
			case 'g':       egroup_only = 1;                break;
			case 'n':       text_id = 1;                    break;
			case 'r':       real_id = 1;                    break;
			case 'u':       euser_only = 1;                 break;
			default:        
				exit(EXIT_FAILURE);
				break;
		}
	}

	/* check for illegal option combos */
	if (((all_group+egroup_only+euser_only)>1) || (all_group && real_id)) {
		fprintf(stderr,TXT(T_BAD_ARGS));
		exit(EXIT_FAILURE);
	}

	if (optind<argc) userid = argv[ optind++ ];
	else userid = NULL;
	if (optind<argc) {
		fprintf(stderr,"id: Only one user may be specified as an operand\n");
		exit(EXIT_FAILURE);
	}

	/*
	 * If we have specified a user name on the command line, lookup all the
	 * necessary info we can on him
	 */

	if (userid!=NULL) {
		if ((user_data = getpwnam(userid)) == NULL) {
		    if(!isallnumeric(userid) || (user_data=getpwuid(atoi(userid)))==NULL) {
				fprintf(stderr,"id: user %s not found.\n",userid);
				exit(EXIT_FAILURE);
		    }
		}

		gid = user_data->pw_gid;
		uid = user_data->pw_uid;
		euid = uid;     /* .2 - if user is named we assume effective=real */
		egid = gid;
		uname = strdup(user_data->pw_name);
		euname = uname;
		group_data = getgrgid(gid);
		if (group_data!=NULL) gname = strdup(group_data->gr_name);
		else gname = "";
		egname = gname;

		ngroups=NGROUPS_MAX;
		(void) getgrouplist(uname, gid, groups, &ngroups);
	} else {
		gid  = getgid();
		uid  = getuid();
		euid = geteuid();
		egid = getegid();

		user_data = getpwuid(uid);
		if (user_data!=NULL) uname = strdup(user_data->pw_name);
		else uname = "";

		group_data = getgrgid(gid);
		if (group_data!=NULL) gname = strdup(group_data->gr_name);
		else gname = "";

		if (euid!=uid){
			user_data = getpwuid(euid);
			if (user_data!=NULL)euname = strdup(user_data->pw_name);
			else euname = "";
		} else euname = uname;

		if (egid!=gid) {
			group_data = getgrgid( egid );
			if (group_data!=NULL) egname = strdup(group_data->gr_name);
			else egname = "";
		} else egname = gname;

		ngroups=getgroups(NGROUPS_MAX,groups);
	}

	/* Now that all the data has been gathered, display it as per
	 * spec and command line options specified */

	/* all these if conditions are exclusive (cmd line parsing won't
       let more than one be true) so I can fall thru to the one exit
       at the end */

	if (option_given == 0) {
		printf("uid=%u",uid);
		if (*uname) printf("(%s)",uname);
		printf(" gid=%u",gid);
		if (*gname) printf("(%s)",gname);
		if (euid != uid) {
			printf(" euid=%u",euid);
			if (*euname) printf("(%s)",euname);
		}
		if (egid != gid) {
			printf(" egid=%u",egid);
			if (*egname) printf("(%s)",egname);
		}

		if (ngroups>1) {
			int first = 1;
			for (i=0;i<ngroups;i++) {
				if (first) { first = 0; printf(" groups="); } else putchar(',');
				printf("%u",groups[i]);
				if ((group_data=getgrgid(groups[i]))) printf("(%s)", group_data->gr_name); 
			}
		}
		printf("\n");
	}

	if (all_group) {
		if (text_id) {
			printf("%s",gname);
			if (egid!=gid) printf(" %s",egname);
		} else {
			printf("%u",gid);
			if (egid!=gid) printf(" %u",egid);
		}

		if (ngroups>1) {
			for (i=1;i<ngroups;i++) {
				if (groups[i]==egid || groups[i]==gid) continue;

				if (text_id) {
					if ((group_data=getgrgid(groups[i]))) printf(" %s", group_data->gr_name); 
					else printf(" %d",groups[i]);
				} else {
					printf(" %d",groups[i]);
				}
			}
		}
		printf("\n");
	}

	if (egroup_only) {
		if (text_id) printf("%s\n",real_id?gname:egname);
		else printf("%u\n",real_id ? gid : egid);
	}       

	if (euser_only) {
		if (text_id) printf("%s\n", real_id ? uname : euname);
		else printf(" %u\n", real_id ? uid : euid);
	}
	return EXIT_SUCCESS;
}
