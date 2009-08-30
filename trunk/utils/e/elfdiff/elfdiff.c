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




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x)    x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <elfdiff.h>
#include <libelf.h>
#include <libelf_int.h>
#include <lib/compat.h>

#ifndef MAKE_LIB

extern int optind, optopt;
extern char* optarg;

#define SECTION_DEFAULTS	"QNX_usage;QNX_PhAB"
#define USAGE "elfdiff - test that ELF binaries are functionally identical (QNX)\n\n\
elfdiff [-vq] [-S<section>] <file 1> <file 2>\n\n\
Options:\n\
 -v             Be Extra Verbose\n\
 -q             Be Quiet\n\
 -S<section>    Compare <section> as well (multiple -S options may be used)\n\
                default: -SQNX_usage -SQNX_PhAB\n\
 -n             Ignore default sections\n\
 -h		Handle unsorted .rel.dyn sections\n"

int handle_rel_dyn=0;
 
int main(int argc, char *argv[]) {
  int opt=0, i=0;
  int verbose = 1, no_default_sections = 0, first = 1;
  char *section_list=NULL;
  int fd[2];

  while ((opt = getopt (argc, argv, "hvqS:n")) != -1) {
    switch (opt) {
      case 'h':
	handle_rel_dyn=1;
	break;
      case 'v':
        verbose++;
        break;
      case 'q':
        verbose--;
        break;
      case 'n':
	no_default_sections = 1;
        break;
      case 'S':
	if (section_list) section_list = (char*)realloc(section_list, strlen(optarg)+strlen(section_list));
	else section_list = (char*)realloc(section_list, strlen(optarg));
	if (!first) strcat(section_list, ";");
	strcat(section_list, optarg);
        first = 0;
        break;
      default:
        free(section_list);
        exit(1);
        break;
    }
  }  

  if ((argc-optind) != 2) {
    if (verbose) fprintf(stderr, "%s", USAGE);
    free(section_list);
    exit(1);  
  }

  if (verbose > 2) verbose = 2;
  else if (verbose < 0) verbose = 0;

  if (!no_default_sections) {
    if (section_list!=NULL) {
      section_list = (char*)realloc(section_list, strlen(section_list));
      strcat(section_list, ";");
    }
    if (section_list) section_list = (char*)realloc(section_list, strlen(SECTION_DEFAULTS)+strlen(section_list));
    else section_list = (char*)realloc(section_list, strlen(SECTION_DEFAULTS));
    strcat( section_list, SECTION_DEFAULTS );
  }

  for (i=0;i<2;i++) {
    fd[i]=open(argv[argc-(argc-optind)+i], O_RDONLY | O_BINARY);
    if (fd[i]==-1) {
      if (verbose) fprintf(stderr, "elfdiff: Error opening file '%s'\n", argv[argc-(argc-optind)+i]);
      free(section_list);
      exit(1);
    }
  }

  return binary_diff(fd[0],fd[1], section_list, verbose);
}

#endif //MAKE_LIB

int binary_diff(int fd1, int fd2, char *sections, int verbose) {
  Elf *elf1, *elf2;
  Elf_Kind kind1, kind2;

  if (elf_version(EV_CURRENT) == EV_NONE) {
    if (verbose) fprintf(stderr, "elfdiff: ELF Version mismatch\n");
    return ED_ELFFAIL;
  }

  if ((elf1 = elf_begin(fd1, ELF_C_READ, NULL)) != NULL) {
    kind1 = elf_kind(elf1);
    elf_end(elf1);
  } else {
    if (verbose) fprintf(stderr, "elfdiff: Error reading ELF information from first input file.\n");  
    return ED_NOELF1;
  }

  if ((elf2 = elf_begin(fd2, ELF_C_READ, NULL)) != NULL) {
    kind2 = elf_kind(elf2);
    elf_end(elf2);
  } else {
    if (verbose) fprintf(stderr, "elfdiff: Error reading ELF information from second input file.\n");  
    return ED_NOELF2;
  }

  if ((kind1 == ELF_K_ELF) && (kind2 == ELF_K_ELF)){ 
    return elf_diff (fd1, fd2, sections, verbose);
  } else if ((kind1 == ELF_K_AR) && (kind2 == ELF_K_AR)) {
    return ar_diff (fd1, fd2, sections, verbose);
  } else {
    if (verbose) fprintf(stderr, "elfdiff: The specified files are not of matching/supported types.\n");
    return ED_ELFFAIL;
  }
}

int rel_comp(const void *a, const void *b) {
	if ((*(int*)a) < (*(int *)b)) return -1;
	if ((*(int*)a) == (*(int *)b)) return 0;
	return 1;
}

typedef struct _seg {
  Elf32_Off offset;
  Elf32_Word size;
} seg;

int phdr_diff( Elf *elf[2], int verbose ) {
  int i = 0, j = 0;
  int k[2] = {0,0};
  int hasdyn[2] = {0,0}; 
  Elf32_Dyn *dyn;
  Elf_Scn *scn[2];
  seg dynseg[2] = {{0,0},{0,0}};
  seg symseg[2] = {{0,0},{0,0}};
  seg hashseg[2] = {{0,0},{0,0}};
  seg reldynseg[2] = {{0,0},{0,0}};
  int retval = ED_SUCCESS;

  Elf32_Phdr *phdr[2] = {NULL, NULL};
  Elf32_Off offset[2] = { 0, 0 };
  Elf32_Off size[2] = { 0, 0 };
  Elf32_Off vaddr[2] = { 0, 0 };

  for (i=0;i<2;i++) {
    phdr[i]=elf32_getphdr(elf[i]);
    Elf32_swapPhdr(elf[i]);
    for (j=0;j<elf[i]->e_ehdrp->e_phnum;j++) {
      if (phdr[i][j].p_type==PT_DYNAMIC) {
        hasdyn[i]=1;
        dynseg[i].offset = phdr[i][j].p_offset;
        dynseg[i].size = phdr[i][j].p_filesz;
      } else if (phdr[i][j].p_type==PT_LOAD) {
        if (!vaddr[i] && (phdr[i][j].p_offset==0)) vaddr[i]=phdr[i][j].p_vaddr;
      }
    }
  }

  if ((hasdyn[0]) || (hasdyn[1])) {
    if (hasdyn[0] != hasdyn[1]) {
      if (verbose>1) fprintf(stderr, "elfdiff: One input file is dynamically linked, the other is not.\n");
      return ED_HDRFAIL;
    } else {
      char *buf[2] = {NULL, NULL};

      for (i=0;i<2;i++) {
        buf[i]=(char*)malloc(dynseg[i].size*sizeof(char));
      }

      for (i=0;i<2;i++) {
        if (lseek(elf[i]->e_fd, dynseg[i].offset, SEEK_SET) == -1 ) {
          if (verbose) fprintf(stderr, "elfdiff: Seek error in input file %d.\n", i);
          free(buf[0]); free(buf[1]);
          return ED_IOFAIL;
        }
        if (read(elf[i]->e_fd, buf[i], dynseg[i].size)==-1) {
          if (verbose) fprintf(stderr, "elfdiff: Read error in input file %d.\n", i);
          free(buf[0]); free(buf[1]);
          return ED_IOFAIL;
        }
      }
    
      for (i=0;i<2;i++) {
        dyn=(Elf32_Dyn*)buf[i];
        while (dyn->d_tag!=DT_NULL) {
        Elf32_swapDyn(elf[i], dyn);
          if (dyn->d_tag==DT_SYMTAB) {
            symseg[i].offset=dyn->d_un.d_ptr;
            symseg[i].offset-=vaddr[i];
          } else if (dyn->d_tag==DT_HASH) {
            hashseg[i].offset=dyn->d_un.d_ptr;
            hashseg[i].offset-=vaddr[i];
          }
          dyn++;
        }
      }

      for (i=0;i<2;i++) {
        free(buf[i]);
      }

      for (i=0;i<2;i++) {
        Elf32_Word numchains[2] = {0,0};
        if (lseek(elf[i]->e_fd, hashseg[i].offset, SEEK_SET) == -1) {
          if (verbose) fprintf(stderr, "elfdiff: Seek error in input file %d.\n", i);
          return ED_IOFAIL;
        }
        if (read(elf[i]->e_fd, numchains, 2*sizeof(Elf32_Word))==-1) {
          if (verbose) fprintf(stderr, "elfdiff: Read error in input file %d.\n", i);
          return ED_IOFAIL;
        }
        if (elf[i]->xlat) swap_32(&numchains[1]); 
	symseg[i].size = numchains[1] * sizeof(Elf32_Sym);
      }

    }
  }
  if (handle_rel_dyn) {
	for (i=0;i<2;i++) {
	    scn[i] = elf_getscn(elf[i], 0);
	    elf32_getshdr(scn[i]);
	}

	for (i=0;i<2;i++) {
		for (k[i]=0;k[i]<elf[i]->e_ehdrp->e_shnum;k[i]++) {
	        	if (!strcmp(elf_strptr(elf[i],elf[i]->e_ehdrp->e_shstrndx, elf[i]->e_shdrp[k[i]].sh_name),".rel.dyn")) {
			  reldynseg[i].offset=elf[i]->e_shdrp[k[i]].sh_offset;
			  reldynseg[i].size=elf[i]->e_shdrp[k[i]].sh_size;
		          break;
	        	}
		}
	}
	k[0]=0; k[1]=0;
  } 

  while (!retval) {
    for (i=0;i<2;i++) {
      for (;k[i]<elf[i]->e_ehdrp->e_phnum;k[i]++) {
        if ((phdr[i][k[i]].p_filesz > 0) && (phdr[i][k[i]].p_type != PT_NOTE) && (phdr[i][k[i]].p_type!=PT_PHDR)) {
          offset[i]=phdr[i][k[i]].p_offset;
          size[i]=phdr[i][k[i]].p_filesz;
          if (offset[i] < (sizeof(Elf32_Ehdr)+(elf[i]->e_ehdrp->e_phnum*sizeof(Elf32_Phdr)))) {
            size[i]-=((sizeof(Elf32_Ehdr)+(elf[i]->e_ehdrp->e_phnum*sizeof(Elf32_Phdr)))-offset[i]);
            offset[i]=(sizeof(Elf32_Ehdr)+(elf[i]->e_ehdrp->e_phnum*sizeof(Elf32_Phdr)));
          }
          k[i]++;
          break;
        }
      }
      if (k[i] >= elf[i]->e_ehdrp->e_phnum) {
        return retval;
      }
    }


    if (size[0]) {
      char *buf[2] = { NULL, NULL };

      if (size[0] != size[1]) {
        if (verbose>1) fprintf(stderr, "elfdiff - Size of segment %d - WARNING\n", k[0]-1);
        if (size[0] > size[1]) {
  	      int t;
  	      t=size[1];
  	      size[1]=size[0];
  	      size[0]=t;
        }
      } 

      for (i=0;i<2;i++) {
        buf[i]=(char*)malloc(size[i]*sizeof(char));
      }

      for (i=0;i<2;i++) {
        if (lseek(elf[i]->e_fd, offset[i], SEEK_SET) == -1 ) {
          if (verbose) fprintf(stderr, "elfdiff: Seek error in input file %d.\n", i);
          free(buf[0]); free(buf[1]);
          return ED_IOFAIL;
        }
        if (read(elf[i]->e_fd, buf[i], size[i])==-1) {
          if (verbose) fprintf(stderr, "elfdiff: Read error in input file %d.\n", i);
          free(buf[0]); free(buf[1]);
          return ED_IOFAIL;
        }
        //This makes the assumption that the DT_SYMTAB will not overlap two segments
        if ((symseg[i].offset >= offset[i]) && (symseg[i].offset < (offset[i]+size[i]))) {
          memset(buf[i]+(symseg[i].offset-offset[i]), 0, symseg[i].size);
        }
	if (handle_rel_dyn) {
        	//This makes the assumption that the .rel.dyn section will not overlap two segments
		if ((reldynseg[i].offset >= offset[i]) && (reldynseg[i].offset < (offset[i]+size[i]))) {
	          memset(buf[i]+(reldynseg[i].offset-offset[i]), 0, reldynseg[i].size);
		}
	}
      }

      if (memcmp(buf[0], buf[1], size[0])!=0) {
        if (verbose>1) fprintf(stderr, "elfdiff: Contents of segment %d - FAIL.\n", k[1]-1);
        retval = ED_DATAFAIL;
      }

      for (i=0;i<2;i++) {
        free(buf[i]);
      }

      if (handle_rel_dyn && reldynseg[0].size && reldynseg[1].size) {
	for (i=0;i<2;i++) {
        	buf[i]=(char*)malloc(reldynseg[i].size*sizeof(char));
      	}
	      
	for (i=0;i<2;i++) {
        	if (lseek(elf[i]->e_fd, reldynseg[i].offset, SEEK_SET) == -1 ) {
	          if (verbose) fprintf(stderr, "elfdiff: Seek error in input file %d.\n", i);
	          free(buf[0]); free(buf[1]);
	          return ED_IOFAIL;
	        }
	        if (read(elf[i]->e_fd, buf[i], reldynseg[i].size)==-1) {
	          if (verbose) fprintf(stderr, "elfdiff: Read error in input file %d.\n", i);
	          free(buf[0]); free(buf[1]);
	          return ED_IOFAIL;
	        }
	}
	qsort(buf[0], (reldynseg[0].size / (2*sizeof(int))), 2*sizeof(int), rel_comp);
	qsort(buf[1], (reldynseg[1].size / (2*sizeof(int))), 2*sizeof(int), rel_comp);
	if (memcmp(buf[0], buf[1], reldynseg[1].size)!=0) {
		if (verbose>1) fprintf(stderr, "elfdiff: Contents of .rel.dyn section - FAIL.\n");
		for (i=0;i<2;i++) {
	        	free(buf[i]);
		}
		retval = ED_DATAFAIL;
	}
	for (i=0;i<2;i++) {
	        free(buf[i]);
	}
      }
    } 

    offset[0] = offset[1] = 0;
    size[0] = size[1] = 0;

  }

  ///////////////////////////////////////////
  //FIXME: PT_NOTE and DT_SYMTAB processing here
  ///////////////////////////////////////////

  return retval;
}

int shdr_diff(Elf *elf[2], char *sections, int verbose) {
  int i=0;
  int k[2] = { 0,0};
  int retval = ED_SUCCESS;
  int nodef[2] = {0, 0};
  Elf32_Off offset[2] = { 0, 0 };
  Elf_Scn *scn[2];
  char *tok;

  if (sections == NULL) {
    return ED_SUCCESS;
  }

  for (i=0;i<2;i++) {
    scn[i] = elf_getscn(elf[i], 0);
    if (scn[i])
      elf32_getshdr(scn[i]);
  }

  if (!scn[0] && !scn[1]) {
    return ED_SUCCESS;          /* no section in both files */
  } else if (( scn[0] && !scn[1]) ||
             (!scn[0] &&  scn[1])) {
    if (verbose)
      fprintf(stderr, "elfdiff: %s input file has no section\n",
              ((scn[0]==0) ? "first" : "second"));
    return ED_HDRFAIL;
  }

  tok = strtok(sections, ";");

  while (!retval && (tok != NULL)) {

    for (i=0;i<2;i++) {
      for (k[i]=0;k[i]<elf[i]->e_ehdrp->e_shnum;k[i]++) {
        if (!strcmp(elf_strptr(elf[i],elf[i]->e_ehdrp->e_shstrndx, elf[i]->e_shdrp[k[i]].sh_name),tok)) {
          offset[i]=elf[i]->e_shdrp[k[i]].sh_offset + elf[i]->e_offset;
          k[i]++;
          break;
        }
      }
      if (k[i] >= elf[i]->e_ehdrp->e_shnum) {
        if ((strcmp (tok, "QNX_usage") != 0) && (strcmp(tok, "QNX_PhAB") != 0)) {
          if (verbose) fprintf(stderr, "elfdiff: Input file does not contain named section '%s'\n", tok);
          return ED_HDRFAIL;
        } else {
          nodef[i] = 1;
	}
      }
    }

    if (nodef[0] != nodef[1]) {
      if (verbose>1) fprintf(stderr, "elfdiff: Default section mismatch.\nelfdiff: One file contains section '%s', one doesn't.\n", tok);
      return ED_HDRFAIL;
    } else if ((nodef[0] == 1) || (nodef[1] == 1)) {
      tok = strtok(NULL, ";");
      break;
    }

    nodef[0] = nodef[1] = 0;

    if (elf[0]->e_shdrp[k[0]-1].sh_size != elf[1]->e_shdrp[k[1]-1].sh_size) {
      if (verbose>1) fprintf(stderr, "elfdiff: Size of section '%s' - FAIL.\n", tok);
      retval = ED_HDRFAIL;
    } else {

      char *buf[2] = { NULL, NULL };
      for (i=0;i<2;i++) {
        buf[i] = (char*)malloc(elf[i]->e_shdrp[k[i]-1].sh_size * sizeof(char));
      }

      for (i=0;i<2;i++) {
        if (lseek(elf[i]->e_fd, offset[i], SEEK_SET) == -1 ) {
          if (verbose) fprintf(stderr, "elfdiff: Seek error in input file %d.\n", i);
          free(buf[0]); free(buf[1]);
          return ED_IOFAIL;
        }
        if (read(elf[i]->e_fd, buf[i], elf[i]->e_shdrp[k[i]-1].sh_size)==-1) {
          if (verbose) fprintf(stderr, "elfdiff: Read error in input file %d.\n", i);
          free(buf[0]); free(buf[1]);
          return ED_IOFAIL;
        }
      }

      if (memcmp(buf[0], buf[1], elf[0]->e_shdrp[k[0]-1].sh_size)!=0) {
        if (verbose>1) fprintf(stderr, "elfdiff: Contents of section '%s' - FAIL.\n", tok);
        retval = ED_DATAFAIL;
      }

      for (i=0;i<2;i++) {
        free(buf[i]);
      }

    }
    offset[0] = offset[1] = 0;
    tok = strtok(NULL, ";");
  }

  return retval;
}

int ohdr_diff(Elf *elf[2], int verbose) {
  int i=0;
  int k[2] = { 0,0};
  int retval = ED_SUCCESS;
  Elf32_Off offset[2] = { 0, 0 };
  Elf_Scn *scn[2];

  for (i=0;i<2;i++) {
    scn[i] = elf_getscn(elf[i], 0);
    elf32_getshdr(scn[i]);
  }

  while (!retval) {

    for (i=0;i<2;i++) {
      for (;k[i]<elf[i]->e_ehdrp->e_shnum;k[i]++) {
        if ((elf[i]->e_shdrp[k[i]].sh_flags && SHF_ALLOC) && (elf[i]->e_shdrp[k[i]].sh_type != SHT_NOBITS)) {
          offset[i]=elf[i]->e_shdrp[k[i]].sh_offset + elf[i]->e_offset;
          k[i]++;
          break;
        }
      }
      if (k[i] >= elf[i]->e_ehdrp->e_shnum) {
        return retval;
      }
    }
    if (elf[0]->e_shdrp[k[0]-1].sh_size != elf[1]->e_shdrp[k[1]-1].sh_size) {
      if (verbose>1) fprintf(stderr, "elfdiff: Sizes of section %d - FAIL.\n", k[0]-1);
      retval = ED_HDRFAIL;
    } else {

      char *buf[2] = { NULL, NULL };
      for (i=0;i<2;i++) {
        buf[i] = (char*)malloc(elf[i]->e_shdrp[k[i]-1].sh_size * sizeof(char));
      }

      for (i=0;i<2;i++) {
        if (lseek(elf[i]->e_fd, offset[i], SEEK_SET) == -1 ) {
          if (verbose) fprintf(stderr, "elfdiff: Seek error in input file %d.\n", i);
          free(buf[0]); free(buf[1]);
          return ED_IOFAIL;
        }
        if (read(elf[i]->e_fd, buf[i], elf[i]->e_shdrp[k[i]-1].sh_size)==-1) {
          if (verbose) fprintf(stderr, "elfdiff: Read error in input file %d.\n", i);
          free(buf[0]); free(buf[1]);
          return ED_IOFAIL;
        }
      }

      if (memcmp(buf[0], buf[1], elf[0]->e_shdrp[k[1]-1].sh_size)!=0) {
        if (verbose>1) fprintf(stderr, "elfdiff: Contents of section %d - FAIL.\n", k[1]-1);
        retval = ED_DATAFAIL;
      }

      for (i=0;i<2;i++) {
        free(buf[i]);
      }

    }
    offset[0] = offset[1] = 0;

  }

  return retval;
}

int elf_diff(int fd1, int fd2, char *sections, int verbose) {
  int i = 0;
  int retval = ED_SUCCESS;
  Elf *elf[2];

  if ((elf[0] = elf_begin(fd1,ELF_C_READ,NULL))==NULL) {
    if (verbose) fprintf(stderr, "elfdiff: Error reading ELF information from first input file.\n");
    return ED_ELFFAIL;
  }

  if ((elf[1] = elf_begin(fd2,ELF_C_READ,NULL))==NULL) {
    if (verbose) fprintf(stderr, "elfdiff: Error reading ELF information from second input file.\n");
    elf_end(elf[0]);
    return ED_ELFFAIL;
  }

  if (elf[0]->e_ehdrp->e_machine != elf[1]->e_ehdrp->e_machine) {
    if (verbose) fprintf(stderr, "elfdiff: Input files appear to be for different architectures.\n");
    return ED_HDRFAIL;
  } else if (elf[0]->e_ehdrp->e_type != elf[1]->e_ehdrp->e_type) {
    if (verbose) fprintf(stderr, "elfdiff: Input files appear to be of different types.\n");
    return ED_HDRFAIL;
  } else {
    if ((elf[0]->e_ehdrp->e_type == ET_REL)){
      if (!(retval = ohdr_diff(elf, verbose))) {
        retval = shdr_diff(elf, sections, verbose);
      }
    } else if ((elf[0]->e_ehdrp->e_type == ET_EXEC) || (elf[0]->e_ehdrp->e_type == ET_DYN)){
      if (!(retval = phdr_diff(elf, verbose))) {
        retval = shdr_diff(elf, sections, verbose);
      }
    } else {
      if(verbose) fprintf(stderr, "elfdiff: Files are of an unsupported type\n");
      for (i=0;i<2;i++) {
        elf_end(elf[i]);
      }
      return ED_HDRFAIL;
    }
  }

  for (i=0;i<2;i++) {
    elf_end(elf[i]);
  }

  if (verbose) {
    if (retval) {
      fprintf(stderr, "elfdiff: Files do not match\n");
    } else { 
      printf("elfdiff: Files match.\n");
    }
  }

  return retval;
}

int ar_diff(int fd1, int fd2, char *sections, int verbose) {
  int i=0, retval = ED_SUCCESS;
  Elf *arf[2];
  Elf *elf[2];
  Elf_Cmd cmd[2];

  if ((arf[0] = elf_begin(fd1,ELF_C_READ,NULL))==NULL) {
    if (verbose) fprintf(stderr, "elfdiff: Error reading AR information from first input file.\n");
    return ED_ELFFAIL;
  }

  if ((arf[1] = elf_begin(fd2,ELF_C_READ,NULL))==NULL) {
    if (verbose) fprintf(stderr, "elfdiff: Error reading AR information from second input file.\n");
    elf_end(elf[0]);
    return ED_ELFFAIL;
  }

  if (arf[0]->e_numsyms != arf[1]->e_numsyms) { 
    retval = ED_HDRFAIL;
  } else {

    ////////////////////////////////////////////////////////
    //FIXME: this section should correlate the items by name
    ////////////////////////////////////////////////////////

    cmd[0] = cmd[1] = ELF_C_READ;

    while (((elf[0] = elf_begin(fd1, cmd[0], arf[0])) != 0) && !retval) {
      if (elf32_getehdr(elf[0]) != 0) {

        while ((elf[1] = elf_begin(fd2, cmd[1], arf[1])) != 0) {
          if (elf32_getehdr(elf[1]) != 0) {

            if (strcmp(elf[0]->e_arhdrp->ar_name, elf[1]->e_arhdrp->ar_name) != 0) {
              if (verbose>1) fprintf(stderr, "elfdiff: Internal object order does not match.  ('%s' != '%s')\n", elf[0]->e_arhdrp->ar_name, elf[1]->e_arhdrp->ar_name);
              retval = ED_HDRFAIL;
            } else {
              retval = ohdr_diff(elf, verbose);
              if (!retval && sections) retval=shdr_diff(elf, sections, verbose);
              if (retval) {
                if (verbose>1) fprintf(stderr, "elfdiff: Internal object '%s' - FAIL\n", elf[0]->e_arhdrp->ar_name);
              } else {
                if (verbose>1) fprintf(stdout, "elfdiff: Internal object '%s'- PASS\n", elf[0]->e_arhdrp->ar_name);
              }
            }

            cmd[1]=elf_next(elf[1]);
            elf_end(elf[1]);
            break;
          }
          cmd[1]=elf_next(elf[1]);
          elf_end(elf[1]);
        }

      }
      cmd[0]=elf_next(elf[0]);
      elf_end(elf[0]);
    }

  }

  for (i=0;i<2;i++) {
    elf_end(arf[i]);
  }
 
  if (verbose) {
    if (retval) {
      fprintf(stderr, "elfdiff: Files do not match\n");
    } else {
      printf("elfdiff: Files match.\n");
    }
  }

  return retval;
}
