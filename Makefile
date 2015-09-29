# DO NOT FORGET to define BITBOX environment variable 

NAME = check3d
GAME_C_FILES = bb3d.c nonsimple.c main.c
GAME_H_FILES = bb3d.h nonsimple.h

# see this file for options
include $(BITBOX)/lib/bitbox.mk

dbg:  clean
	$(MAKE) M=`pwd` ALL_CFLAGS="$(ALL_CFLAGS) -O1 -g -DDEBUG"
