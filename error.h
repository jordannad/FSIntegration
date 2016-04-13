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

#include <tcl.h>
#include "databox.h"

static char *COMPUTEFACTORSAFETY          = "Usage: computefactorsafety alpha n theta_resid theta_sat \n
										cohesion porosity friction_angle top slope_x slope_y pressure\n";

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

void WrongNumArgsError ANSI_PROTO((Tcl_Interp *interp, char *usage));
void SetNonExistantError ANSI_PROTO((Tcl_Interp *interp, char *hashkey)); 

#undef ANSI_PROTO


#endif