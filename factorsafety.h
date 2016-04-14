 /*
 ****************************************************************************
 * Header file for `factor_safety' program
 *

 ****************************************************************************
 */

/* Include guard to prevent double declaration of identifiers */
#ifndef FACTORSAFETY_HEADER
#define FACTORSAFETY_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "databox.h"

/*-----------------------------------------------------------------------
 * function prototypes
 *-----------------------------------------------------------------------*/

/* factorsafety.c */
void ComputeFactorSafety(
   Databox *alpha, Databox *n, Databox *theta_resid, Databox *theta_sat,
   Databox *cohesion,
   Databox *porosity,
   Databox *friction_angle,
   Databox *top,
   Databox *slope_x,
   Databox *slope_y,
   Databox *pressure, 
   Databox *saturation,
   Databox *factor_safety);
   

#endif
