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
  
void ComputeFactorSafety(Databox *alpha, Databox *n, Databox *theta_resid, Databox *theta_sat,
   Databox *cohesion,
   Databox *porosity,
   Databox *friction_angle,
   Databox *top,
   Databox *slope_x,
   Databox *slope_y,
   Databox *pressure, 
   Databox *saturation,
   Databox *uws_sat,
   Databox *factor_safety,
   double failureDepth);
void ComputeZMin(Databox *factor_safety,  Databox *zmin);
void ComputePressatFSMin(Databox *factor_safety, Databox *pressure, Databox *top, Databox *pressFSMin);

void ComputeLuLikosFS(Databox *alpha, Databox *n, Databox *theta_resid, Databox *theta_sat,
   Databox *cohesion,
   Databox *porosity,
   Databox *friction_angle,
   Databox *top,
   Databox *slope_x,
   Databox *slope_y,
   Databox *pressure, 
   Databox *saturation,
   Databox *uws_sat,
   Databox *factor_safety,
   double failureDepth);

void GetFSSuctionComp(Databox *alpha, Databox *n, Databox *theta_resid, Databox *theta_sat,
   Databox *cohesion,
   Databox *porosity,
   Databox *friction_angle,
   Databox *top,
   Databox *slope_x,
   Databox *slope_y,
   Databox *pressure, 
   Databox *saturation,
   Databox *uws_sat,
   Databox *factor_safety,
   double failureDepth);

void ComputeFSWithRootStrength(Databox *alpha, Databox *n, Databox *theta_resid, Databox *theta_sat,
   Databox *cohesion,
   Databox *root_cohesion,
   Databox *porosity,
   Databox *friction_angle,
   Databox *top,
   Databox *slope_x,
   Databox *slope_y,
   Databox *pressure, 
   Databox *saturation,
   Databox *uws_sat,
   Databox *tree_surcharge,
   Databox *factor_safety,
   double maxfailureDepth);


#endif
