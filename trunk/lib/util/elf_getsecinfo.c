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



#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include _NTO_HDR_(sys/elf.h)

/*
   returns 0 on success, -1 on failure

   Assuming fp is an open file pointer to a valid ELF load file,
   elf_getsecinfo will locate the section whose name matches the
   supplied secname and will fseek to that offset within the file.
   If size is non-null, it will put the section size into the
   memory area it points to. If offset is non-null, it will put
   the seek offset of the section into the memory area it 
   points to.
*/

int elf_getsecinfo(FILE *fp,char *secname,long int *size, long int *offset)
{
  Elf32_Ehdr ehdr;
  Elf32_Shdr *shdr=NULL;
  char *shstrtab=NULL;

  /* seek to beginning of file */
  if (-1==fseek(fp,0L, SEEK_SET)) return -1;

  /* get ELF header */
  if (1==fread(&ehdr,sizeof(ehdr),1,fp)) {
    /* check ELF header fields */
    if (ehdr.e_ident[0]==0x7f &&      
      ehdr.e_ident[1]=='E' &&
      ehdr.e_ident[2]=='L' &&
      ehdr.e_ident[3]=='F') 
    {
      /* it's an ELF file */
      if (ehdr.e_ident[4]!=ELFCLASS32) {
        /* ELF file is not 32-bit */
        return -1;
      }
    }
  }

  /* allocate space for section header table */
  if (NULL!=(shdr=malloc(ehdr.e_shentsize*ehdr.e_shnum))) {
    if (-1!=fseek(fp,ehdr.e_shoff, SEEK_SET)) {
      if (ehdr.e_shnum==fread(shdr,ehdr.e_shentsize,ehdr.e_shnum,fp)) {
        if (NULL!=(shstrtab=malloc(shdr[ehdr.e_shstrndx].sh_size))) {
          if (-1!=fseek(fp,shdr[ehdr.e_shstrndx].sh_offset, SEEK_SET)) {
            if (1==fread(shstrtab,shdr[ehdr.e_shstrndx].sh_size,1,fp)) {
			  int i;

              for (i=1;i<=ehdr.e_shnum;i++) {
                if (!strcmp(&shstrtab[shdr[i].sh_name],secname)) 
                  break;
              }
              if (i<=ehdr.e_shnum) {
				if (size) *size=shdr[i].sh_size;
                if (offset) *offset=shdr[i].sh_offset;
                free(shdr);
                free(shstrtab);
                return ((-1==fseek(fp,shdr[i].sh_offset, SEEK_SET))?-1:0);
              }
              errno=ENOENT;
            }
          }
        }
      }
    }
  }

  /* failure */
  if (shdr) free(shdr);
  if (shstrtab) free(shstrtab);
  return -1;
}

#ifdef TESTPROG

main(int argc, char **argv) 
{
	int rc;
	long size=0,offset=0;

	rc=elf_getsecinfo(stdin,argv[1],&size,&offset);

	if (rc==-1) fprintf(stderr,"error! errno=%d (%s)\n",errno,strerror(errno));
	
	fprintf(stdout,"elf_getsecinfo(stdin,'%s',...,...) returned %d): size->%d, offset->%d\n",
			argv[1],rc, size, offset);
}

#endif
