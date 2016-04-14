#Simple Makefile for Factor of Safety library

#Target entry to build program executable from program files

#Compiler: gcc for C program
CC = gcc

#compiler flags:
# -g     adds debugging information to the executable file
# -Wall  turns on most, but not all compiler warnings
CFLAGS = -g -Wall -fno-common

LDFLAGS = -fPIC -shared  # linking flags

# define any directories containing header files other than /usr/include
#
INCLUDES =

TCLINC=/usr/include/tcl8.6
TCLLIB=/usr/lib/tcl8.6

#define the C header files (this is not used anywhere?)
HDRS = databox.h error.h file.h grid.h load.h readdatabox.h \
	region.h tools_io.h general.h usergrid.h pfload_file.h factorsafety.h fstools.h 

#define the C source files
SRCS = databox.c error.c file.c grid.c load.c readdatabox.c \
       region.c tools_io.c usergrid.c factorsafety.c fstools.c fsappinit.c 

#define the C object files
#
#This uses suffix replacement within a macro
OBJS = $(SRCS:.c=.o)

#define the executable file
MAIN = myfactorsafety.so

all:	$(MAIN)
	@echo Simple compiler named myfactorsafety has been compiled

$(MAIN): $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) -I$(TCLINC) -L$(TCLLIB) -ltclstub8.6

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

# Instead of using the pkgIndex utility manualy maintain the index file.
# There seems to be no way to get the namespace import routine to 
# work since namespace is not done until a command is invoked.  
# By manually creating the index file we can cause the package to load
# immediatly thus avoiding this problem (and namespace import can be
# invoked directly after the package is required)
# LIB_FILES = $(PROJECT)$(PFTOOLS_SHLIB_SUFFIX) parflow.tcl
#
#pkgIndex.tcl: $(PROJECT)$(PFTOOLS_SHLIB_SUFFIX) Makefile.tcllib
#	@echo 'pkg_mkIndex . $(LIB_FILES)' | tclsh8.0



