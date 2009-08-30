#ifndef __ELFDIFF_H_INCLUDED
#define __ELFDIFF_H_INCLUDED

#include <libelf_int.h>

//Everything is super.
#define ED_SUCCESS	0

//The input files are of the same basic type, but differ in
//simple header values (different data segment sizes, etc).
//This indicates a valid difference.
#define ED_HDRFAIL	1

//The input files are elf, but differ in some fundamental way
//(different architectures, different types, etc).
//This indicates a valid difference.
#define ED_ELFFAIL	2

//The input files are basically the same, but have some differences
//in the code or data segments.
//This indicates a valid difference.
#define ED_DATAFAIL	3

//The first input file doesn't appear to be an ELF file.
//This is NOT a valid difference.
#define ED_NOELF1	4

//The second input file doesn't appear to be an ELF file.
//This is NOT a valid difference.
#define ED_NOELF2	5

//There was a failure seeking or reading the file.
//This is NOT a valid difference.
#define ED_IOFAIL	6

int binary_diff(int, int, char*, int);
int phdr_diff(Elf**, int);
int shdr_diff(Elf**, char *, int);
int ohdr_diff(Elf**, int);
int elf_diff(int, int, char*, int);
int ar_diff(int, int, char*, int);

#endif //__ELFDIFF_H_INCLUDED
