/*
  Copyright (c) 1990-2005 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2004-May-22 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
#define fhow "r","mbc=60"
#define fbad NULL
typedef void *ftype;
#define zopen(n,p)   (vms_native?vms_open(n)    :(ftype)fopen((n), p))
#define zread(f,b,n) (vms_native?vms_read(f,b,n):fread((b),1,(n),(FILE*)(f)))
#define zclose(f)    (vms_native?vms_close(f)   :fclose((FILE*)(f)))
#define zerr(f)      (vms_native?vms_error(f)   :ferror((FILE*)(f)))
#define zstdin stdin

ftype vms_open OF((char *));
size_t vms_read OF((ftype, char *, size_t));
int vms_close OF((ftype));
int vms_error OF((ftype));
#ifdef VMS_PK_EXTRA
int vms_get_attributes OF((ftype, struct zlist far *, iztimes *));
#endif
