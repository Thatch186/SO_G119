#variables
SHELL=bash
CC = gcc #Compiler
CFLAGS = -g -O2 -Wall #Compilation Flags
CFLAGS += -Wno-unused-command-line-argument

#Directories
SRCDIR := src
INCDIR := includes
BLDDIR := build
PROGRAM := executable
WARNINGFILE := warnings.txt
LIXO := $(BLDDIR) program $(WARNINGFILE)
INCLUDES := -I $(INCDIR)

#
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(BLDDIR)/%.o,$(SOURCES))

vpath %.c $(SRCDIR)

.DEFAULT_GOAL = program

$(BLDDIR)/%.o: %.c
	@$(CC) -c $(INCLUDES) $(CFLAGS) $< -o $@ $(LFLAGS) 
	
$(PROGRAM): $(OBJECTS)
	@$(CC) $(INCLUDES) $(CFLAGS) -o program -g $(OBJECTS) $(LFLAGS)

program: setup $(PROGRAM)

setup:
	@mkdir -p $(BLDDIR)

.PHONY: clean
clean:
	@rm -rf $(LIXO)