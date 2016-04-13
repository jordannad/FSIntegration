#Simple Makefile for Factor of Safety library

#Target entry to build program executable from program files

#Compiler: gcc for C program
CC = gcc

#compiler flags:
# -g     adds debugging information to the executable file
# -Wall  turns on most, but not all compiler warnings
CFLAGS = -g -Wall

LDFLAGS = -fPIC -shared  # linking flags

# define any directories containing header files other than /usr/include
#
INCLUDES =

TCLINC=/usr/include/tcl8.6
TCLLIB=/usr/lib/x86_64-linux-gnu

#define the C header files
HDRS = databox.h error.h factorsafety.h file.h fstools.h general.h \
       grid.h load.h pfload_file.h readdatabox.h region.h tools_io.h usergrid.h

#define the C source files
SRCS = factorsafety.c fstools.c fsappinit.c databox.c error.c file.c \
       grid.c load.c readdatabox.c region.c tools_io.c usergrid.c

#define the C object files
#
#This uses suffix replacement within a macro
OBJS = $(SRCS:.c=.o)

#define the executable file
MAIN = myfactorsafety.so

default: install 

all:	$(MAIN)
		@echo Simple compiler named myfactorsafety has been compiled

$(MAIN):	$(OBJS)
			$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) -I$(TCLINC) -L$(TCLLIB) -ltclstub8.6

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.c.o:
        $(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

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

#
# For cleaning
#
clean:	
	@for DIR in $(SUBDIRS); do if test -d $$DIR; then (cd $$DIR && $(MAKE) $@) ; fi || exit 1; done
	@$(PARFLOW_CLEAN) 

install: all
	@echo "Installing parflow to $(PARFLOW_INSTALL_DIR)"
	@for DIR in $(SUBDIRS); do if test -d $$DIR; then (cd $$DIR && $(MAKE) $@) ; fi || exit 1; done
	@$(INSTALL) -d -m 755 $(PARFLOW_INSTALL_DIR)/lib
	@$(INSTALL) -d -m 755 $(PARFLOW_INSTALL_DIR)/bin
	@for i in $(SCRIPTS); do				        \
	   $(INSTALL) -c -m 755 $(SRCDIR)/$$i $(PARFLOW_INSTALL_DIR)/bin; \
	done
	@for i in bin/*; do				        \
	   $(INSTALL) -c -m 755 $$i $(PARFLOW_INSTALL_DIR)/bin; \
	done
	@for i in $(PARFLOW)/*.tcl; do				        \
	   $(INSTALL) -c -m 644 $$i $(PARFLOW_INSTALL_DIR)/bin; \
	done

veryclean: clean
	@for DIR in $(SUBDIRS); do if test -d $$DIR; then (cd $$DIR && $(MAKE) $@) ; fi || exit 1; done
	@rm -f $(PROJECT)$(PFTOOLS_SHLIB_SUFFIX)
	@rm -f Makefile config/Makefile.config
	@rm -rf include bin lib


