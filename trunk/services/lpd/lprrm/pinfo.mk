# Copyright 2003, QNX Software Systems Ltd. All Rights Reserved.

# This source code may contain confidential information of QNX Software
# Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
# modification, disclosure, distribution or transfer of this software,
# or any software which includes or is based upon any of this code, is
# prohibited unless expressly authorized by QSSL by written agreement. For
# more information (including whether this source code file has been
# published) please email licensing@qnx.com.


#define PINFO
#PINFO DESCRIPTION=BSD line printer spooler job removal program
#endef

EXTRA_OBJS=startdaemon.o common.o printcap.o rmjob.o

FILE_INFO = 0 0 4775 
