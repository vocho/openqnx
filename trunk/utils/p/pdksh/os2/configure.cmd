@echo off
if "%1" == "/h" goto help
if "%1" == "/?" goto help
if "%1" == "-h" goto help
set verbose=no
set debug=no
set ushell=ksh
:parse
if "%1" == "/v" goto verbose
if "%1" == "-v" goto verbose
if "%1" == "/d" goto gdebug
if "%1" == "-d" goto gdebug
if "%1" == "sh" goto usersh
if exist os2 goto start
echo You are in the wrong directory.  Change to source directory.
echo Copy this file and run again or type os2\configure.
goto end
:verbose
set verbose=yes
shift
goto parse
:gdebug
set debug=yes
shift
goto parse
:usersh
if %verbose% == yes echo Configuring for Bourne shell.
set ushell=sh
:start
if exist conftest.c erase conftest.c
if exist confdefs.h erase confdefs.h
if %verbose% == yes echo Searching for sed
:::::for %%i in (%path%) do if exist %%i\sed.exe goto s_success
sed --version && goto s_success
echo No sed in search path.  Copying Makefile for gcc. You will need
echo to edit it if you are not using standard defaults.
copy os2\Makefile Makefile
goto copystuff
:help
echo Run os2\configure to set up for os/2 compilation.
echo You must have current context in the source directory.
echo usage: configure [[/v^|-v][/d^|-d]^|/h^|-h^|/?] [sh]
echo        where /v means verbose output
echo              /d means compile with symbols (debug)
goto end
:s_success
if %verbose% == yes echo checking for compiler
for %%i in (%path%) do if exist %%i\gcc.exe goto g_success
gcc --version && goto g_success
rem for the future we'll use sed processing
for %%i in (%path%) do if exist %%i\bcc.exe goto b_success
for %%i in (%path%) do if exist %%i\icc.exe goto i_success
echo Compiler not found. Check your path
goto end
:b_success
echo Borland C compiler found.  Configuration not complete for
echo this compiler.  You may need to edit the Makefile
set CC=bcc
set CPP=cpp
goto createstuff
:i_success
echo IBM C compiler found.  Configuration not complete for
echo this compiler.  You may need to edit the Makefile.
set CC=icc -q -Sm -Gm -Gt -Spl -D__STDC__
set CPP=cpp
goto createstuff
:checkshell
if %ushell% == sh goto fixshell
goto copystuff
:g_success
echo GNU C compiler found.  This is the standard configuration.
copy os2\Makefile Makefile
if %debug% == no goto checkshell
set CC=gcc -g
set CPP=cpp
:createstuff
echo Creating files for you.
echo s/@CC@/%CC%/> os2\make.tmp
echo s/@CPP@/%CPP%/>> os2\make.tmp
echo s/@LDFLAGS@/-O/>> os2\make.tmp
if %ushell% == ksh goto skipsh
:fixshell
echo s!/bin/sh!ksh!>> os2\make.tmp
echo s/@SHELL_PROG@/sh/>> os2\make.tmp
:skipsh
copy os2\make.tmp+os2\make.sed os2\make.tmp
sed -f os2\make.tmp Makefile.in >  Makefile
del os2\make.tmp
:copystuff
if %verbose% == yes echo Copying config.h and config.status files. 
copy os2\config.h config.h
touch config.h
copy os2\config.status config.status
if %verbose% == yes echo Checking for os2.c and th.cmd.
if not exist os2.c copy os2\os2.c os2.c
if not exist th.cmd copy os2\th.cmd th.cmd
if %ushell% == ksh goto end 
if %verbose% == yes echo Fixing config.h for building sh.
echo #ifdef KSH>> config.h
echo #undef KSH>> config.h
echo #undef VI>> config.h
echo #undef EMACS>> config.h
echo #undef EDIT>> config.h
echo #undef COMPLEX_HISTORY>> config.h
echo #undef EASY_HISTORY>> config.h
echo #undef HISTORY>> config.h
echo #undef BRACE_EXPAND>>config.h
echo #endif>> config.h
:end


