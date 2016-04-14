# Tcl package index file, version 1.0
# This file tells Tcl how to load my factorsafety package

# Makefile compiles code into factorsafety.so (shared library)

package ifneeded factorsafety 1.0 [list \
	[source [file join $dir factorsafety.tcl]] \
		[load [file join $dir factorsafety[info sharedlibextension]]] ]
