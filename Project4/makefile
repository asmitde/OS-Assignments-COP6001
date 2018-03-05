# makefile
#
# Running the 'make' command in the current directory will compile all the .c
# source files and generate the output binaries for each source file.
#
# Author: Asmit De | U72377278
# Date: 01/21/2016

sources := $(wildcard *.c)
programs := $(patsubst %.c,%,$(sources))

CFLAGS += -Wall

.PHONY: all clean

all: $(programs)

%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	@- $(RM) $(programs)

