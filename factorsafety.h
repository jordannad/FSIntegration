 /*
 ****************************************************************************
 * Header file for `factor_safety' program
 *

 ****************************************************************************
 */

#ifndef FACTORSAFETY_HEADER
#define FACTORSAFETY_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_HDF4
#include <hdf.h>
#endif

#include <tcl.h>

#include "databox.h"

#ifndef NULL
#define NULL ((void *)0)
#endif
#define WS " \t\n"
#define MAX_KEY_SIZE 32


/*-----------------------------------------------------------------------
 * Data structure and accessor macros 
 *-----------------------------------------------------------------------*/

typedef struct
{
   Tcl_HashTable   members;
   GridType        grid_type;
   int             total_members;
   int             num;

} Data;

#define DataMembers(data)   ((data) -> members)
#define DataGridType(data)  ((data) -> grid_type)
#define DataTotalMem(data)  ((data) -> total_members)
#define DataNum(data)       ((data) -> num)
#define DataMember(data, hashkey, entryPtr) \
   ( ((entryPtr = Tcl_FindHashEntry(&DataMembers(data), hashkey)) != 0)	\
   ? (Databox *)Tcl_GetHashValue(entryPtr)				\
   : (Databox *) NULL)
#define FreeData(data) (free((Data *)data))


/*-----------------------------------------------------------------------
 * function prototypes
 *-----------------------------------------------------------------------*/

#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif


/* factorsafety.c */
int FactorSafetyCommand P((ClientData clientData , Tcl_Interp *interp , int argc , char *argv []));
void ComputeFactorSafety(
   Databox *alpha, Databox *n, Databox *theta_resid, Databox *theta_sat,
   Databox *cohesion,
   Databox *porosity,
   Databox *friction_angle,
   Databox *top,
   Databox *slope_x,
   Databox *slope_y,
   Databox *pressure, 
   Databox *factor_safety);
   
#undef P

#endif
