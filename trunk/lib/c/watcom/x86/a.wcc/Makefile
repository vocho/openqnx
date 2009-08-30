EXTRA_OBJS = \
	i8d.o \
	i8m.o \
	i8s.o 
	
include ../../../common.mk

%.o: %.omf
	$(CP_HOST) $^ $@
	omf2elf $@
