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
   /*printf("Got here check dx = %f, dy = %f, dz = %f\n", dx, dy, dz); */
   /* Calculate factor of safety at the failure surface of the domain */
   int fs_nz =round(failureDepth/dz);
   /*printf("Got here check fs_nz = %d\n", fs_nz); */
   fs_inf = 10.0;

   /* Components of factor of safety calculation */
   double	  a1, b1, slope, chi, uws, uws_val, uwssum, depth, uws_depth, gs;
   double         suctionstress, ff, fw, fc, fs_min, z_fsmin, p_min; 
   double 	  theta_sat_val, theta_resid_val, alpha_val, n_val, saturation_val, fric_angle, press, factor_safety_val, porosity_val, cohesion_val;
   double uww = 9801;
   double slopeoverall;
   double dg2rad = 3.14159265/180; /*Convert an angle in degrees to radians */

   /* Loop over all surface cells (first i (x) then j (y) direction) */
   for (j = 0; j < ny; j++)
   {
      for (i = 0; i < nx; i++)
      {
         	  
         /* 
          * Calculate the overall slope for a single cell at the top of the domain.
         */
         slopeoverall = sqrt(pow(*DataboxCoeff(slope_x, i, j, 0),2.0) + 
			pow(*DataboxCoeff(slope_y,i,j,0),2.0));
         slope = (atan(slopeoverall))*(180.0/3.14159265); /*Slope in degrees */
         /*printf("Printing slopes, i = %d, j = %d, slope (degrees) = %f\n", i, j, slope);*/
         a1 = sin(slope*dg2rad); /*Calculate sin of angle in radians */
         b1 = cos(slope*dg2rad); 
         /*printf("Printing slopes, i = %d, j = %d, a = %f, b = %f\n", i, j, a1, b1);*/


	 
         /* Calculate top vertical element */
         k_top = *(DataboxCoeff(top, i, j, 0));
         /*printf("Printing top element number for i = %d, j = %d, Ans = %d \n", i, j, k_top);*/
         /* If k is non-negative then we can proceed */
	 /* For now, all of the examples will guarantee horizontal x-y cells */
	     

         /* Initialize result variables */
         uwssum = 0;
         fs_min = fs_inf;
         z_fsmin = 1.0e25;
         p_min = 100.0;
         uwssum = 0.0;
  	 double moisture_content;
         int fsDataBoxZ = 0;

         if ((i == 1) && (j == 1)) {
               printf("Printing at i = 1, j = 1, top = %d\n", k_top);
         }
         /* Loop over the top soil layers for which FOS calcs are to be performed */
         for (k = k_top; k >= (k_top - fs_nz); k--) {
	    depth = (k_top - k)*dz; /*Depth below the surface */
            fsDataBoxZ = fs_nz - (k_top - k);
            if ((i == 1) && (j == 1)) {
               printf("Printing at i = 1, j = 1, fsDataBoxnz = %d\n", fsDataBoxZ);
         }
            /* Testing depth- confirms looping from surface to 3.0m below everywhere*/

            if ((i == 1) && (j == 1)) {
               printf("Printing depth at i = 1, j = 1, Depth = %f\n", depth);
            }
            /*Get values from input datasets */
	    porosity_val = *(DataboxCoeff(porosity, i, j, k));
	    saturation_val = *(DataboxCoeff(saturation, i, j, k));
            moisture_content = porosity_val*saturation_val;
            press = *(DataboxCoeff(pressure, i, j, k));
	    theta_sat_val = *(DataboxCoeff(theta_sat, i, j, k));
            theta_resid_val = *(DataboxCoeff(theta_resid, i, j, k));
	    uws_val = *(DataboxCoeff(uws_sat, i, j, k));
            cohesion_val = *(DataboxCoeff(cohesion, i, j, k));

            /* Component to assist calculation of depth averaged unit weight */
            gs = (uws_val/uww - porosity_val)/(1 - porosity_val);
	    

            /* These two values are currently unused */
            alpha_val = *(DataboxCoeff(alpha, i, j, k));
            n_val = *(DataboxCoeff(n, i, j, k));
           
	     /* Calculate unit weight of the soil for partially saturated conditions */
	     if (press < 0.) {
		uws = (gs*(1- porosity_val) + moisture_content)*uww;
 	     } else {
                uws = uws_val;
             }
             uwssum = uwssum + uws;
	     uws_depth = uwssum/(depth/dz);

             /* Check if slope is too flat */
             if (fabs(slope) < 1.0e-5) {
                ff = fs_inf;
             } else {
                fric_angle = *(DataboxCoeff(friction_angle, i, j, k));
                ff = tan(fric_angle*dg2rad)/tan(slope*dg2rad);
             }

	     /* Consistent with TRIGRS implementation */
             if ((abs(a1) > 1.0e-5) && (k != k_top)) {

                /* Initialize Bishop's fs correction for saturated conditions */
                chi = 1.0;
		/* Adjust chi if we have unsaturated conditions */
                if (press < 0.) {
                  chi = (moisture_content - theta_resid_val)/(porosity_val - theta_resid_val);
                  suctionstress = (-1*press)/(pow((1 + pow((alpha_val*press),n_val)),((n_val-1)/n_val)));
                } else {
                  chi = 1.0;
                  suctionstress = -1*press;
                } 

                /* Compute the other factor of safety components */ 
                fw = -(chi * press * uww * tan(fric_angle*dg2rad))/(uws_depth*a1*b1*depth);
                fc = cohesion_val/(uws_depth*depth*a1*b1);

              } else {
                fw = 0.0;
                fc = 0.0;
              }
              factor_safety_val = ff + fw + fc;

               /* Frictional strength cannot be less than zero */
              if ((ff + fw) < 0.) {
                  factor_safety_val = fc;
                  printf("Got here (ff + fw) < 0. on i = %d, j = %d\n", i, j);
              }
              if (factor_safety_val > fs_inf) {
                 factor_safety_val = fs_inf;
              }
              if (factor_safety_val < fs_min) {
                 fs_min = factor_safety_val;
                 z_fsmin = k;
                 p_min = press;
              }
         

           *(DataboxCoeff(factor_safety, i, j, fsDataBoxZ)) = factor_safety_val;
	   } 
      }
   }
}

