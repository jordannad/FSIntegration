/*BHEADER**********************************************************************

**********************************************************************EHEADER*/
/******************************************************************************
 * Error header file
 *
 *-----------------------------------------------------------------------------
 *
 *****************************************************************************/

#ifndef ERROR_HEADER
#define ERROR_HEADER

#include <stdio.h>
#include <stdarg.h>

#ifdef HAVE_HDF4
#include <hdf.h>
#endif

#include <tcl/tcl.h>
#include "databox.h"

/*------------------------------------------------------------------------
 * Error messages
 *------------------------------------------------------------------------*/

static char *FACTORSAFETYUSAGE = "Usage: getfactorsafety alpha n theta_resid theta_sat cohesion porosity friction_angle top slope_x slope_y pressure saturation failuredepth\n";
static char *FSEXTRACTTOPUSAGE          = "Usage: pfextracttop top dataset\n";
static char *LOADPFUSAGE = "Usage: fsload [-filetype] filename\n file types: pfb pfsb sa sb rsa \n";
static char *SAVEFSUSAGE = "Usage: fssave dataset -filetype filename\n      file types: pfb sa sb \n";
static char *ZMINUSAGE = "Usage: computezatfsmin factorSafety\n";
static char *PRESSATFSMINUSAGE = "Usage: computepressatfsmin factorSafety Pressure Top\n";

/*-----------------------------------------------------------------------
 * function prototypes
 *-----------------------------------------------------------------------*/

#ifdef __STDC__
# define        ANSI_PROTO(s) s
#else
# define ANSI_PROTO(s) ()
#endif

/* Function prototypes for error checking functions */

int  SameDimensions ANSI_PROTO((Databox *databoxp, Databox *databoxq));
int  InRange ANSI_PROTO((int i, int j, int k, Databox *databox));
int  IsValidFileType ANSI_PROTO((char *option));
char *GetValidFileExtension ANSI_PROTO((char *filename));

/* Function prototypes for creating error messages  */
void InvalidOptionError ANSI_PROTO((Tcl_Interp *interp, int argnum, char *usage));
void MissingFilenameError ANSI_PROTO((Tcl_Interp *interp, int argnum, char *usage));
void InvalidFileExtensionError ANSI_PROTO((Tcl_Interp *interp, int argnum, char *usage));

void NotADoubleError ANSI_PROTO((Tcl_Interp *interp, int argnum, char *usage));
void WrongNumArgsError ANSI_PROTO((Tcl_Interp *interp, char *usage));
void MissingOptionError ANSI_PROTO((Tcl_Interp *interp, int argnum, char *usage));
void SetNonExistantError ANSI_PROTO((Tcl_Interp *interp, char *hashkey)); 
void ReadWriteError ANSI_PROTO((Tcl_Interp *interp));
#undef ANSI_PROTO


#endif
