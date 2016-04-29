#include "factorsafety.h"

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


void ComputeFactorSafetyold(
   Databox *alpha, Databox *n, Databox *theta_resid, Databox *theta_sat,
   Databox *cohesion,
   Databox *porosity,
   Databox *friction_angle,
   Databox *top,
   Databox *slope_x,
   Databox *slope_y,
   Databox *pressure, 
   Databox *saturation,
   Databox *factor_safety)
{
   int             i,  j,  k;
   int             nx, ny, nz;
   double          dx, dy, dz;

   int 		   k_top;
   double 	   fs_inf; 

   nx = DataboxNx(pressure);
   ny = DataboxNy(pressure);
   nz = DataboxNz(pressure);

   dx = DataboxDx(pressure);
   dy = DataboxDy(pressure);
   dz = DataboxDz(pressure);

   /* Calculate factor of safety for the top 10 m of the domain */
   int fs_nz =round(10.0/dz);
   fs_inf = 10.0;

   /* Components of factor of safety calculation */
   double uww = 9801.0;
   
   double	  a1, b1, slope, chi, uws, depth;
   double         ff, fw, fc, fs_min, z_fsmin, p_min; 
   double 	  theta_sat_val, theta_resid_val, alpha_val, n_val, saturation_val, fric_angle, press, factor_safety_val, porosity_val, uws_val;
   for (j = 0; j < ny; j++)
   {
      for (i = 0; i < nx; i++)
      {
	 
         /* 
          * Calculate the overall slope for a single cell at the top of the domain.
         */
         slope = (sqrt(pow(*DataboxCoeff(slope_x, i, j, 0),2.0)) + sqrt(pow(*DataboxCoeff(slope_y,i,j,0),2.0)));
         a1 = sin(slope);
         b1 = cos(slope);

         /* Calculate top vertical element */
         k_top = *(DataboxCoeff(top, i, j, 0));
         /* If k is non-negative then we can proceed */
	      /*if (k_top > 0 ) { */

            /* Loop over the top soil layer for which factor of safety calculations are to be performed */
            fs_min = fs_inf;
            z_fsmin = k_top;
            p_min = k_top;
            for (k = k_top; k > (k_top - fs_nz); k--) {
               /* Check if slope is too flat */
               if (fabs(slope) < 1.0e-5) {
                  ff = fs_inf;
               } else {
                  fric_angle = *(DataboxCoeff(friction_angle, i, j, k));
                  ff = slope/fric_angle;
               }
               if ((abs(a1) > 1.0e-5) && (k != k_top)) {
                  /* Initialize Bishop's fs correction for saturated conditions */
                  chi = 1.0;
                  press = *(DataboxCoeff(pressure, i, j, k));
                  if (press < 0.) {
  		     porosity_val = *(DataboxCoeff(porosity, i, j, k));
		     uws = 18000.0;
                     theta_sat_val = *(DataboxCoeff(theta_sat, i, j, k));
                     theta_resid_val = *(DataboxCoeff(theta_resid, i, j, k));
                     alpha_val = *(DataboxCoeff(alpha, i, j, k));
                     n_val = *(DataboxCoeff(n, i, j, k));
		     saturation_val = *(DataboxCoeff(saturation, i, j, k));
                     chi = (saturation_val - theta_resid_val)/(theta_sat_val - theta_resid_val);
                  }
                  depth = k_top - k;
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
         

           *(DataboxCoeff(factor_safety, i, j, k)) = factor_safety_val;
	   } 
      }
   }
} 


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
   double failureDepth)
{
   int             i,  j,  k;
   int             nx, ny, nz;
   double          dx, dy, dz;

   int 		   k_top;
   double 	   fs_inf; 

   nx = DataboxNx(pressure);
   ny = DataboxNy(pressure);
   nz = DataboxNz(pressure);

   dx = DataboxDx(pressure);
   dy = DataboxDy(pressure);
   dz = DataboxDz(pressure);

   /* Calculate factor of safety at the failure surface of the domain */
   int fs_nz =round(failureDepth/dz);
   fs_inf = 10.0;

   /* Components of factor of safety calculation */
      
   double	  a1, b1, slope, chi, uws, uws_val, uwssum, depth, uws_depth, gs;
   double         suctionstress, ff, fw, fc, fs_min, z_fsmin, p_min; 
   double 	  theta_sat_val, theta_resid_val, alpha_val, n_val, saturation_val, fric_angle, press, factor_safety_val, porosity_val;
   double uww = 9801;

   /* Loop over all surface cells (first i (x) then j (y) direction) */
   for (j = 0; j < ny; j++)
   {
      for (i = 0; i < nx; i++)
      {
	  
         /* 
          * Calculate the overall slope for a single cell at the top of the domain.
         */
         slope = (sqrt(pow(*DataboxCoeff(slope_x, i, j, 0),2.0)) + sqrt(pow(*DataboxCoeff(slope_y,i,j,0),2.0)));
         a1 = sin(slope);
         b1 = cos(slope);
	 uwssum = 0;
         /* Calculate top vertical element */
         k_top = *(DataboxCoeff(top, i, j, 0));
         /* If k is non-negative then we can proceed */
	 /* For now, all of the examples will guarantee horizontal x-y cells */
	      /*if (k_top > 0 ) { */

          /* Initialize result variables */
          fs_min = fs_inf;
          z_fsmin = 1.0e25;
          p_min = 100.0;
          uwssum = 0.0;
  	  /* Loop over the top soil layers for which FOS calcs are to be performed */
          for (k = k_top; k > (k_top - fs_nz); k--) {
	     depth = k_top - k;
	     porosity_val = *(DataboxCoeff(porosity, i, j, k));
	     saturation_val = *(DataboxCoeff(saturation, i, j, k));
	     uws_val = *(DataboxCoeff(uws_sat, i, j, k));
             gs = (uws_val/uww - porosity_val)/(1 - porosity_val);
	     press = *(DataboxCoeff(pressure, i, j, k));
	     theta_sat_val = *(DataboxCoeff(theta_sat, i, j, k));
             theta_resid_val = *(DataboxCoeff(theta_resid, i, j, k));
             alpha_val = *(DataboxCoeff(alpha, i, j, k));
             n_val = *(DataboxCoeff(n, i, j, k));
           
	     /* Calculate unit weight of the soil for partially saturated conditions */
	     if (press < 0.) {
		uws = (gs*(1- porosity_val) + saturation_val)*uww;
 	     } else {
                uws = uws_val;
             }
             uwssum = uwssum + uws;
	     uws_depth = uwssum/depth;

             /* Check if slope is too flat */
             if (fabs(slope) < 1.0e-5) {
                ff = fs_inf;
             } else {
                fric_angle = *(DataboxCoeff(friction_angle, i, j, k));
                ff = slope/fric_angle;
             }
             if ((abs(a1) > 1.0e-5) && (k != k_top)) {

                /* Initialize Bishop's fs correction for saturated conditions */
                chi = 1.0;
		
                if (press < 0.) {
                  chi = (saturation_val - theta_resid_val)/(theta_sat_val - theta_resid_val);
                  suctionstress = (-1*press)/(pow((1 + pow((alpha_val*press),n_val)),((n_val-1)/n_val)));
                } else {
                  chi = 1.0;
                  suctionstress = -1*press;
                }  
                fw = -(chi * press * uww * tan(fric_angle))/(uws_depth*a1*b1*depth);
                fc = (*(DataboxCoeff(cohesion, i, j, k)))/(uws_depth*depth*a1*b1);
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
         

           *(DataboxCoeff(factor_safety, i, j, k)) = factor_safety_val;
	   } 
      }
   }
}

