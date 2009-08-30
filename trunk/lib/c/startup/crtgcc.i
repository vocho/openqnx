/* Specialized bits of code needed to support construction and
   destruction of file-scope objects in C++ code.
   Copyright (C) 1991, 94, 95, 96, 97, 1998 Free Software Foundation, Inc.
   Contributed by Ron Guilmette (rfg@monkeys.com).

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* As a special exception, if you link this library with files
   compiled with GCC to produce an executable, this does not cause
   the resulting executable to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

/* This file is a bit like libgcc1.c/libgcc2.c in that it is compiled
   multiple times and yields multiple .o files.

   This file is useful on target machines where the object file format
   supports multiple "user-defined" sections (e.g. COFF, ELF, ROSE).  On
   such systems, this file allows us to avoid running collect (or any
   other such slow and painful kludge).  Additionally, if the target
   system supports a .init section, this file allows us to support the
   linking of C++ code with a non-C++ main program.

   Note that if INIT_SECTION_ASM_OP is defined in the tm.h file, then
   this file *will* make use of the .init section.  If that symbol is
   not defined however, then the .init section will not be used.

   Currently, only ELF and COFF are supported.  It is likely however that
   ROSE could also be supported, if someone was willing to do the work to
   make whatever (small?) adaptations are needed.  (Some work may be
   needed on the ROSE assembler and linker also.)

   This file must be compiled with gcc.  */

/* It is incorrect to include config.h here, because this file is being
   compiled for the target, and hence definitions concerning only the host
   do not apply.  */

/* This file is a cleaned-up version of the file crtstuff.c from the gcc
   distribution. It is used with gcc to provide for the calling of global
   constructors and destructors with C++ programs. 
   It should be compiled four times, with the following defines:

   -DCRT_BEGIN
   -DCRT_END
   -DCRT_BEGIN -DDWARF2_UNWIND_INFO
   -DCRT_END -DDWARF2_UNWIND_INFO

   The first two passed will yield the crtbegin.o and crtend.o files, 
   while the last two should yield crtbeginG.o and crtendG.o objects, to
   be used for debug. 

   Also modified to keep a static pointer to the constructor list, so 
   that if the .init section is done more than once (by the dl loader),
   the constructors don't get called again.
   S. Marineau, 07/98
*/

/*lint --e(85) __DTOR_LIST__ and __CTOR_END__ implementation */

#ifdef CRT_C_USEAGE_ATTRIBUTE
#error CRT_C_USEAGE_ATTRIBUTE already defined
#else
#if defined(__GNUC__) && (__GNUC__ < 3)
#define CRT_C_USEAGE_ATTRIBUTE __attribute__((unused))
#else
#define CRT_C_USEAGE_ATTRIBUTE __attribute__((used))
#endif
#endif

#define TEXT_SECTION_ASM_OP	".text"
#define INIT_SECTION_ASM_OP	".section\t.init"
#define FINI_SECTION_ASM_OP	".section\t.fini"
#define CTORS_SECTION_ASM_OP	".section\t.ctors,\"aw\""
#define DTORS_SECTION_ASM_OP	".section\t.dtors,\"aw\""

#ifdef DWARF2_UNWIND_INFO
#define EH_FRAME_SECTION_ASM_OP	".section\t.eh_frame,\"aw\""
#endif

/*  Declare a pointer to void function type.  */
typedef void (*func_ptr) (void);
#define STATIC static

#if defined( __SH__ ) || defined( __MIPS__ )
	//Do indirect call through function pointer to avoid section switching
	//problem with assembler
	#define DOCALL(f) { void (* volatile fp)() = f; fp(); }
#else 
	#define DOCALL(f)	f()
#endif

#ifdef CRT_BEGIN

#ifdef DWARF2_UNWIND_INFO
static char __EH_FRAME_BEGIN__[];
#endif

/*lint --e(31) __DTOR_LIST__ and __CTOR_END__ implementation */
static func_ptr __DTOR_LIST__[];

#if __GNUC__ >= 4
void *__dso_handle __attribute__ ((__visibility__ ("hidden"))) = &__dso_handle;
#else
void *__dso_handle = &__dso_handle; 

#ifndef BUILDENV_ion
/* Don't make this hidden; ION ld gets very unhappy */
asm(".hidden __dso_handle");
#endif
#endif

extern void __cxa_finalize (void *) __attribute__((weak));
static void
CRT_C_USEAGE_ATTRIBUTE __do_global_dtors_aux ()
{
  static func_ptr *p = __DTOR_LIST__ + 1;

  if (__cxa_finalize)
    __cxa_finalize (__dso_handle);

  while (*p)
    {
      p++;
      (*(p-1)) ();
    }

#ifdef DWARF2_UNWIND_INFO
  __deregister_frame_info (__EH_FRAME_BEGIN__);
#endif
}

/* Stick a call to __do_global_dtors_aux into the .fini section.  */

static void
CRT_C_USEAGE_ATTRIBUTE fini_dummy ()
{
  asm (FINI_SECTION_ASM_OP);
  DOCALL(__do_global_dtors_aux);
  asm (TEXT_SECTION_ASM_OP);
}

#ifdef EH_FRAME_SECTION_ASM_OP
/* Stick a call to __register_frame_info into the .init section.  For some
   reason calls with no arguments work more reliably in .init, so stick the
   call in another function.  */

static void
frame_dummy ()
{
  static struct object object;
  __register_frame_info (__EH_FRAME_BEGIN__, &object);
}

static void
CRT_C_USEAGE_ATTRIBUTE init_dummy ()
{
  asm (INIT_SECTION_ASM_OP);
  DOCALL(frame_dummy);
  asm (TEXT_SECTION_ASM_OP);
}
#endif /* EH_FRAME_SECTION_ASM_OP */

#if defined (__GNUC__) && __GNUC__ < 3
/* Force cc1 to switch to .data section.  */
static func_ptr __attribute__ ((unused)) force_to_data[0] = { };

/* NOTE:  In order to be able to support SVR4 shared libraries, we arrange
   to have one set of symbols { __CTOR_LIST__, __DTOR_LIST__, __CTOR_END__,
   __DTOR_END__ } per root executable and also one set of these symbols
   per shared library.  So in any given whole process image, we may have
   multiple definitions of each of these symbols.  In order to prevent
   these definitions from conflicting with one another, and in order to
   ensure that the proper lists are used for the initialization/finalization
   of each individual shared library (respectively), we give these symbols
   only internal (i.e. `static') linkage, and we also make it a point to
   refer to only the __CTOR_END__ symbol in crtend.o and the __DTOR_LIST__
   symbol in crtbegin.o, where they are defined.  */

/* The -1 is a flag to __do_global_[cd]tors
   indicating that this table does not start with a count of elements.  */
asm (CTORS_SECTION_ASM_OP);	/* cc1 doesn't know that we are switching! */
STATIC __attribute__ ((unused)) func_ptr __CTOR_LIST__[1] = { (func_ptr) (-1) };
asm (DTORS_SECTION_ASM_OP);	/* cc1 doesn't know that we are switching! */
STATIC __attribute__ ((unused)) func_ptr __DTOR_LIST__[1] = { (func_ptr) (-1) };
#else
STATIC func_ptr __CTOR_LIST__[1]
  __attribute__ ((used, section(".ctors"), aligned(sizeof(func_ptr))))
  = { (func_ptr) (-1) };
STATIC func_ptr __DTOR_LIST__[1]
  __attribute__((used, section(".dtors"), aligned(sizeof(func_ptr))))
  = { (func_ptr) (-1) };
#endif

#ifdef EH_FRAME_SECTION_ASM_OP
/* Stick a label at the beginning of the frame unwind info so we can register
   and deregister it with the exception handling library code.  */

asm (EH_FRAME_SECTION_ASM_OP);
#ifdef INIT_SECTION_ASM_OP
STATIC
#endif
char __EH_FRAME_BEGIN__[] = { };
#endif /* EH_FRAME_SECTION_ASM_OP */

#endif /* defined(CRT_BEGIN) */



#ifdef CRT_END

static func_ptr __CTOR_END__[];
static void 
CRT_C_USEAGE_ATTRIBUTE __do_global_ctors_aux ()
{
    static func_ptr *p = __CTOR_END__ - 1;
    while (*p != (func_ptr) -1){
        (*p) ();
        p--;
    }
}

/* Stick a call to __do_global_ctors_aux into the .init section.  */

static void
CRT_C_USEAGE_ATTRIBUTE init_dummy ()
{
  asm (INIT_SECTION_ASM_OP);
  DOCALL(__do_global_ctors_aux);
  asm (TEXT_SECTION_ASM_OP);
}

#if defined (__GNUC__) && __GNUC__ < 3
/* Force cc1 to switch to .data section.  */
static func_ptr __attribute__ ((unused)) force_to_data[0] = { };

/* Put a word containing zero at the end of each of our two lists of function
   addresses.  Note that the words defined here go into the .ctors and .dtors
   sections of the crtend.o file, and since that file is always linked in
   last, these words naturally end up at the very ends of the two lists
   contained in these two sections.  */

asm (CTORS_SECTION_ASM_OP);	/* cc1 doesn't know that we are switching! */
STATIC func_ptr __attribute__ ((unused)) __CTOR_END__[1] = { (func_ptr) 0 };

asm (DTORS_SECTION_ASM_OP);	/* cc1 doesn't know that we are switching! */
STATIC func_ptr __attribute__ ((unused)) __DTOR_END__[1] = { (func_ptr) 0 };
#else
STATIC func_ptr __CTOR_END__[1]
  __attribute__((used, section(".ctors"), aligned(sizeof(func_ptr))))
  = { (func_ptr) 0 };
STATIC func_ptr __DTOR_END__[1]
  __attribute__((used, section(".dtors"), aligned(sizeof(func_ptr))))
  = { (func_ptr) 0 };
#endif

#ifdef EH_FRAME_SECTION_ASM_OP
/* Terminate the frame unwind info section with a 4byte 0 as a sentinel;
   this would be the 'length' field in a real FDE.  */

typedef unsigned int ui32 __attribute__ ((mode (SI)));
asm (EH_FRAME_SECTION_ASM_OP);
STATIC ui32 __FRAME_END__[] = { 0 };
#endif /* EH_FRAME_SECTION */

#endif /* defined(CRT_END) */
