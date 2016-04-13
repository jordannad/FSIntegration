#include "factor_safety.h"

/*-----------------------------------------------------------------------
 * ComputeFactorSafety:
 *
 * Computes the factor of safety for the top 5 m of the land surface
 *
 * Returns a Databox factor_safety with factor of safety values at
 * each (i,j) location.
 *
 *-----------------------------------------------------------------------*/

#include <math.h>


void ComputeFactorSafety(
   Databox *alpha, Databox *n, Databox *theta_resid, Databox *theta_sat,
   Databox *cohesion,
   Databox *porosity,
   Databox *friction_angle,
   Databox *top,
   Databox *slope_x,
   Databox *slope_y,
   Databox *pressure, 
   Databox *factor_safety)
{
   int             i,  j;
   int             nx, ny, nz;
   double          dx, dy, dz;

   /* Pre-assignment of coefficients for 3d datasets */
   double *cohesion_coeff            = DataboxCoeffs(cohesion);
   double *porosity_coeff            = DataboxCoeffs(porosity);
   double *pressure_coeff            = DataboxCoeffs(pressure);
   double *factor_safety_coeff       = DataboxCoeffs(factor_safety);

   nx = DataboxNx(pressure);
   ny = DataboxNy(pressure);
   nz = DataboxNz(pressure);

   dx = DataboxDx(pressure);
   dy = DataboxDy(pressure);
   dz = DataboxDz(pressure);

   /* Calculate factor of safety for the top 10 m of the domain */
   int fs_nz = round(10.0/dz,0);
   double fs_inf = 10.0;

   /* Components of factor of safety calculation */
   double         ff, fw, fc; 

   for (j = 0; j < ny; j++)
   {
      for (i = 0; i < nx; i++)
      {
	 
         /* 
          * Calculate the overall slope for a single cell at the top of the domain.
         */
         double slope = (sqrt(pow(*DataboxCoeff(slope_x, i, j, 0),2.0)) + sqrt(pow(*DataboxCoeff(slope_y,i,j,0),2.0)));
         double a1 = sin(slope)
         double b1 = cos(slope)

         /* Calculate top vertical element */
         int k_top = *(DataboxCoeff(top, i, j, 0));
         /* If k is non-negative then we can proceed */
	      if ( k_top > 0 ) {

            /* Loop over the top soil layer for which factor of safety calculations are to be performed */
            double fs_min = fs_inf;
            double z_fsmin = k_top;
            double p_min = k_top;
            for (int k = k_top; k > (k_top - fs_nx); k--)
               /* Check if slope is too flat */
               if (fabs(slope) < 1.0e-5) {
                  ff = fs_inf;
               } else {
                  double fric_angle = *(DataboxCoeff(friction_angle, i, j, k));
                  ff = slope/fric_angle;
               }
               if ((abs(a1) > 1.0e-5) && (k != k_top)) {
                  /* Initialize Bishop's fs correction for saturated conditions */
                  double chi = 1.0;
                  double press = *(DataboxCoeff(pressure, i, j, k));
                  if (press < 0.) {
                     double theta_sat_val = *(DataboxCoeff(theta_sat, i, j, k));
                     double theta_resid_val = *(DataboxCoeff(theta_resid, i, j, k));
                     double alpha_val = *(DataboxCoeff(alpha, i, j, k));
                     double n_val = *(DataboxCoeff(n, i, j, k));
                     double porosity_val = *(DataboxCoeff(porosity, i, j, k));
                     chi = (porosity_val - theta_resid_val)/(theta_sat_val - theta_resid_val);
                  }
                  double depth = k_top - k;
                  fw = -(chi * press * uww * tan(fric_angle))/(uws*a1*b1*depth);
                  fc = (*(DataboxCoeff(cohesion, i, j, k)))/(uws*depth*a1*b1);
               } else {
                  fw = 0.0;
                  fc = 0.0;
               }
               factor_safety_val = ff + fw + fc;

               /* Frictional strength cannot be less than zero */
               if ((ff + fw) < 0.) {
                   factor_safety_val = fc;
               }
               if (factor_safety_val > fs_inf) {
                  factor_safety_val = fs_inf;
               }
               if (factor_safety_val < fs_min) {
                  fs_min = factor_safety_val;
                  z_fsmin = k;
                  p_min = press;
               }
         

         if (*(DataboxCoeff()))
	     *(DataboxCoeff(factor_safety, i, j, 0)) = 0.0;
	   } else if( k <  nz) {
	    if(*(DataboxCoeff(pressure, i, j, k)) > 0 ) {
	       *(DataboxCoeff(surface_storage, i, j, 0)) =  *(DataboxCoeff(pressure, i, j, k)) * dx * dy;
	    }
	 } else {
	    printf("Error: Index in top (k=%d) is outside of domain (nz=%d)\n", k, nz);
	 }
      }
   }
}

