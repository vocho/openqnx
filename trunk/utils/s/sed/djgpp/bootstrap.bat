@echo off
if exist config.h goto Compile
echo Creating basic config.h...
echo /* A bootstrap version of config.h for DJGPP DOS systems */ > config.h
echo #define PACKAGE "sed" >> config.h
echo #define VERSION "``bootstrap''" >> config.h
echo #define BOOTSTRAP 1 >> config.h

:Compile
Rem tell the user what we're doing from here on...
@echo on
if exist lib\*.o del lib\*.o
if exist sed\*.o del sed\*.o
if exist sed\sed del sed\sed
cd lib
gcc -DHAVE_CONFIG_H -I.. -I. -c getopt.c getopt1.c memcmp.c memmove.c regex.c strerror.c
cd ..\sed
gcc -DHAVE_CONFIG_H -I.. -I. -I../lib -c sed.c compile.c execute.c utils.c
gcc -o sed sed.o compile.o execute.o utils.o ../lib/*.o
