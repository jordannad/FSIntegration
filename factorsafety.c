#include "factorsafety.h"

/*-----------------------------------------------------------------------
 * ComputeFactorSafety:
 *
 * Computes the factor of safety for the top x m of the land surface
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
   double	  a1, b1, slope, chi, uws, uws_val, uwssum, depth, uws_depth, e, gs;
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
         for (k = k_top; k > (k_top - fs_nz); k--) {
	    depth = (k_top - k)*dz + (dz/2.0); /*Depth below the surface */
            fsDataBoxZ = (fs_nz-1) - (k_top - k);
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
      uws_val = 22000;
            cohesion_val = *(DataboxCoeff(cohesion, i, j, k));
            
            /* Component to assist calculation of depth averaged unit weight */
            e = porosity_val/(1 - porosity_val);
            gs = (((1+e)*uws_val)/uww - e);
	    

            /* These two values are currently unused */
            alpha_val = *(DataboxCoeff(alpha, i, j, k));
            n_val = *(DataboxCoeff(n, i, j, k));
           
	     /* Calculate unit weight of the soil for partially saturated conditions */
	     if (press < 0.) {
		uws = (uww*(gs+ (e*(moisture_content/porosity_val))))/(1+e);
                if ((i == 1) && (j == 1)) {
                  printf("Pressure less than zero: %f. UWS: %f\n", press, uws);
                }
 	     } else {
                uws = uws_val;
                if ((i == 1) && (j == 1)) {
                  printf("Pressure greater than zero: %f. UWS: %f\n", press, uws);
                }
             }
             uwssum = uwssum + uws;
	     uws_depth = uwssum/((depth+(dz/2.0))/dz);
             
             /* Check if slope is too flat */
             if (fabs(slope) < 1.0e-5) {
                ff = fs_inf;
             } else {
                fric_angle = *(DataboxCoeff(friction_angle, i, j, k));
                ff = tan(fric_angle*dg2rad)/tan(slope*dg2rad);
             }

	     /* Consistent with TRIGRS implementation */
             if ((i == 1) && (j == 1)) {
               printf("a1 = %f, k = %d, ktop = %d, absa1 = %f\n", a1, k, k_top, abs(a1));
               if (fabs(a1) > 0.00001) {
                  printf("Abs a1 greater than num m2\n");
               } else {
                  printf("Abs a1 less than num \n");
               }
             }
             if ((fabs(a1) > 0.00001)) {

                /* Initialize Bishop's fs correction for saturated conditions */
                chi = 1.0;
		/* Adjust chi if we have unsaturated conditions */
                if (press < 0.) {
                  chi = (moisture_content - theta_resid_val)/(porosity_val - theta_resid_val);
                  /* suctionstress = (-1*press)/(pow((1 + pow((alpha_val*press),n_val)),((n_val-1)/n_val))); */
                } else {
                  chi = 1.0;
                  /* suctionstress = -1*press; */
                } 

                /* Compute the other factor of safety components */ 
                fw = -(chi * press * uww * tan(fric_angle*dg2rad))/(uws_depth*a1*b1*depth);
                fc = cohesion_val/(uws_depth*depth*a1*b1);

              } else {
                if ((i == 1) && (j == 1)) printf("Here setting fw to 0\n");
                fw = 0.0;
                fc = 0.0;
              }
              factor_safety_val = ff + fw + fc;
              if ((i == 1) && (j == 1)) {
               /*printf("Printing depth at i = 1, j = 1, Depth = %f\n", depth);*/
               printf("Cohesion: %f, First comp: %f, UWS: %f, Slope = %f, Pressure = %f, Chi = %f, FSVal = %f\n", cohesion_val, ff, uws_depth, slope, press, chi, factor_safety_val);
              }
               /* Frictional strength cannot be less than zero */
              if ((ff + fw) < 0.) {
                  factor_safety_val = fc;
                  /*printf("Got here (ff + fw) < 0. on i = %d, j = %d\n", i, j);*/
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


/* New function to compute the depth to minimum factor of safety value */

/* NOTE: This function should return a 2D databox (i.e. nz = 1) */

void ComputeZMin(Databox *factor_safety,  Databox *zmin)
{
   
   int             i,  j,  k;
   int             nx, ny, nz;
   double          dx, dy, dz;
   double          depth, factor_safety_val, fs_min, z_fsmin, p_min;

   nx = DataboxNx(factor_safety);
   ny = DataboxNy(factor_safety);
   nz = DataboxNz(factor_safety);

   dx = DataboxDx(factor_safety);
   dy = DataboxDy(factor_safety);
   dz = DataboxDz(factor_safety);
   printf("Got here in ComputeZMin check dx = %f, dy = %f, dz = %f\n", dx, dy, dz); 
   printf("Got here in ComputeZMin check nx = %d, ny = %d, nz = %d\n", nx, ny, nz);
   /* Calculate factor of safety at the failure surface of the domain */
  

   /* Loop over all surface cells (first i (x) then j (y) direction) */
   for (j = 0; j < ny; j++)
   {
      for (i = 0; i < nx; i++)
      {
         	  
         /* Initialize result variables */
         fs_min = 1000;
         z_fsmin = 1.0e25;
         p_min = 100.0;
         
         /* Loop over the top soil layers for which FOS calcs are to be performed */
         for (k = 0; k < nz; k++) {
	    depth = (k+1)*dz; /*Depth below the surface */
            
            if ((i == 1) && (j == 1)) {
               printf("Printing depth at i = 1, j = 1, k = %d, Depth = %f\n", k,depth);
            } 
            
 	    factor_safety_val = *(DataboxCoeff(factor_safety, i, j, k));

            if (factor_safety_val < fs_min) {
               fs_min = factor_safety_val;
               z_fsmin = (k+1)*dz;
            }
         }

         *(DataboxCoeff(zmin, i, j, 0)) = z_fsmin;
         printf("Printing depth at i = %d, j = %d, Zmin = %f\n", i, j, z_fsmin);
	    
      }
   }
   /*printf("Got here at the end of zmin function, k = %d\n", k);*/

}

/* New function to compute the depth to minimum factor of safety value */

/* NOTE: This function should return a 2D databox (i.e. nz = 1) */

void ComputePressatFSMin(Databox *factor_safety, Databox *pressure, Databox *top, Databox *pressFSMin)
{
   
   int             i,  j,  k, k_top, fs_k;
   int             nx, ny, nz;
   double          dx, dy, dz;
   double          depth, factor_safety_val, pressure_val, fs_min, p_min;
 


   nx = DataboxNx(factor_safety);
   ny = DataboxNy(factor_safety);
   nz = DataboxNz(factor_safety);

   dx = DataboxDx(factor_safety);
   dy = DataboxDy(factor_safety);
   dz = DataboxDz(factor_safety);
   printf("Got here in ComputePressatFSMin check dx = %f, dy = %f, dz = %f\n", dx, dy, dz); 
   printf("Got here in ComputePressatFSMin check nx = %d, ny = %d, nz = %d\n", nx, ny, nz);
   
   /* Loop over all surface cells (first i (x) then j (y) direction) */
   for (j = 0; j < ny; j++)
   {
      for (i = 0; i < nx; i++)
      {
         	  
         /* Initialize result variables */
         fs_min = 1000;
         p_min = 100.0;


         k_top = *(DataboxCoeff(top, i, j, 0));
         
         /* Loop over the top soil layers for which FOS calcs are to be performed */
         /* Loop over the top soil layers for which FOS calcs are to be performed */
         for (k = k_top; k >= (k_top - nz); k--) {
         
	    depth = (k-k_top+1)*dz; /*Depth below the surface */
            fs_k = k - k_top;

            if ((i == 1) && (j == 1)) {
               printf("Printing depth at i = 1, j = 1, k = %d, Depth = %f\n", k,depth);
            } 
            
 	    factor_safety_val = *(DataboxCoeff(factor_safety, i, j, fs_k));
            pressure_val = *(DataboxCoeff(pressure, i, j, k));
            
            if (factor_safety_val < fs_min) {
               fs_min = factor_safety_val;
               p_min = pressure_val;
            }
         }

         *(DataboxCoeff(pressFSMin, i, j, 0)) = p_min;
         printf("Printing depth at i = %d, j = %d, press at fsmin = %f\n", i, j, p_min);
	    
      }
   }
   /*printf("Got here at the end of zmin function, k = %d\n", k);*/

}

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
   double failureDepth)
{
   int             i,  j,  k, l;
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
   double	  a1, b1, slope, chi, uws, uws_val, uwssum, depth, uws_depth, e, gs;
   double         suctionstress, ff, fw, fc;
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
	     
         if (k_top > 0) {
         /* Initialize result variables */
         uwssum = 0.0;
  	 double moisture_content;
         int fsDataBoxZ = 0;

         if ((i == 1) && (j == 1)) {
               printf("Printing at i = 1, j = 1, top = %d\n", k_top);
         }
         /* Loop over the top soil layers for which FOS calcs are to be performed */
         for (k = k_top; k > (k_top - fs_nz); k--) {
	    depth = (k_top - k)*dz + (dz/2.0); /*Depth below the surface */
            fsDataBoxZ = (fs_nz-1) - (k_top - k);
            if ((i == 1) && (j == 1)) {
               printf("Printing at i = 1, j = 1, fsDataBoxnz = %d\n", fsDataBoxZ);
         }
            /* Testing depth- confirms looping from surface to 3.0m below everywhere*/

            if ((i == 32) && (j == 31)) {
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
            e = porosity_val/(1 - porosity_val);
            gs = (((1+e)*uws_val)/uww - e);
            /*gs = ((uws_val/uww - porosity_val)/(1 - porosity_val);) */
	          /*gs = 2.67; */

            /* These two values are currently unused */
            alpha_val = *(DataboxCoeff(alpha, i, j, k));
            n_val = *(DataboxCoeff(n, i, j, k));
           
	     /* Calculate unit weight of the soil for partially saturated conditions */
	     if (press < 0.) {
		/* uws = (gs*(1- porosity_val) + moisture_content)*uww; */
        uws = (uww*(gs+ (e*(moisture_content/porosity_val))))/(1+e);
 	     } else {
                uws = uws_val;
             }
             uwssum = uwssum + uws;
	     uws_depth = uwssum/((depth+(dz/2.0))/dz);

             /* Check if slope is too flat */
             if (fabs(slope) < 1.0e-5) {
                ff = fs_inf;
             } else {
                fric_angle = *(DataboxCoeff(friction_angle, i, j, k));
                ff = tan(fric_angle*dg2rad)/tan(slope*dg2rad);
             }


	     /* Consistent with TRIGRS implementation */
             if ((fabs(a1) > 0.00001)) {
               if ((i == 1) && (j == 1)) {
                 /* printf("Got into abs a1 and k inner loop at depth = %f\n", depth); */
               }
                /* Initialize Bishop's fs correction for saturated conditions */
                chi = 1.0;
		/* Adjust chi if we have unsaturated conditions */
                if (press < 0.) {
                  chi = (moisture_content - theta_resid_val)/(porosity_val - theta_resid_val);
                  suctionstress = (press*uww)/(pow((1.0 + pow((-1.0*alpha_val*press*uww),n_val)),((n_val-1)/n_val)));
                } else {
                  chi = 1.0;
                  suctionstress = press*uww;
                } 

                /* Compute the other factor of safety components */ 
                /*fw = -(chi * press * uww * tan(fric_angle*dg2rad))/(uws_depth*a1*b1*depth);*/
                fw = (-1.0*suctionstress)*(tan(slope*dg2rad) + (1.0/tan(slope*dg2rad)))*tan(fric_angle*dg2rad)/(uws_depth*depth);
                fc = (2*cohesion_val)/(uws_depth*depth*sin(2*slope*dg2rad));
		/* if ((i == 5) && (j == 5)) {
                  double sstressfac = 1/(pow((1 + pow((alpha_val*press*uww),n_val)),((n_val-1)/n_val))); 
                  printf("Chi factor: %f vs. suction stress: %f\n", chi, sstressfac); 
                } */
              } else {
                fw = 0.0;
                fc = 0.0;
              }
              factor_safety_val = ff + fw + fc;
              if ((i == 31) && (j == 30)) {
               /*printf("Printing depth at i = 1, j = 1, Depth = %f\n", depth);*/
               printf("Cohesion: %f, First comp: %f, fw: %f, fc: %f, UWS: %f, Slope = %f, Pressure = %f, moistureC = %f, FSVal = %f\n", cohesion_val, ff, fw, fc, uws_depth, slope, press, moisture_content, factor_safety_val);
              }
               /* Frictional strength cannot be less than zero */
              if ((ff + fw) < 0.) {
                  factor_safety_val = fc;
                  printf("Got here (ff + fw) < 0. on i = %d, j = %d\n", i, j);
              }
              if (factor_safety_val > fs_inf) {
                 factor_safety_val = fs_inf;
              }      
           *(DataboxCoeff(factor_safety, i, j, fsDataBoxZ)) = factor_safety_val;
	   } 
   } else {
      for (l = 0; l < fs_nz; l++) {
        *(DataboxCoeff(factor_safety, i, j, l)) = fs_inf;
      }
   }
      }
   }
}

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
   double maxfailureDepth)
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
   int fs_nz =round(maxfailureDepth/dz);
   /*printf("Got here check fs_nz = %d\n", fs_nz); */
   fs_inf = 10.0;

   /* Components of factor of safety calculation */
   double	  a1, b1, slope, chi, uws, uws_val, uwssum, depth, uws_depth, gs;
   double         suctionstress, ff, fw, fc;
   double 	  theta_sat_val, theta_resid_val, alpha_val, n_val, saturation_val, fric_angle, press, factor_safety_val, porosity_val, cohesion_val, tree_surcharge_val, root_cohesion_val;
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
	    root_cohesion_val = *(DataboxCoeff(root_cohesion, i, j, k));
	    tree_surcharge_val = *(DataboxCoeff(tree_surcharge, i, j, k));

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
                  suctionstress = (press*uww)/(pow((1.0 + pow((-1.0*alpha_val*press*uww),n_val)),((n_val-1)/n_val)));
                } else {
                  chi = 1.0;
                  suctionstress = press*uww;
                } 

                /* Compute the other factor of safety components */ 
                /*fw = -(chi * press * uww * tan(fric_angle*dg2rad))/(uws_depth*a1*b1*depth);*/
                fw = ((-1.0*suctionstress)*tan(fric_angle*dg2rad))/((tree_surcharge_val+uws_depth*depth)*a1*b1);
                fc = (root_cohesion_val + cohesion_val)/((tree_surcharge_val + uws_depth*depth)*a1*b1);
		if ((i == 5) && (j == 5)) {
                  double sstressfac = 1/(pow((1 + pow((alpha_val*press*uww),n_val)),((n_val-1)/n_val)));
                  printf("Chi factor: %f vs. suction stress: %f\n", chi, sstressfac);
                }
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
           *(DataboxCoeff(factor_safety, i, j, fsDataBoxZ)) = factor_safety_val;
	   } 
      }
   }
}


/********************************************************************************
 * FUNCTION TO RETURN THE SUCTION STRESS COMPONENT OF THE FACTOR OF SAFETY
 ********************************************************************************/

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
   Databox *suction_stressComp,
   double failureDepth) {


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
   double         suctionstress, ff, fw, fc;
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
                  suctionstress = (press*uww)/(pow((1.0 + pow((-1.0*alpha_val*press*uww),n_val)),((n_val-1)/n_val)));
                } else {
                  chi = 1.0;
                  suctionstress = press*uww;
                } 

                /* Compute the other factor of safety components */ 
                /*fw = -(chi * press * uww * tan(fric_angle*dg2rad))/(uws_depth*a1*b1*depth);*/
                fw = (-1.0*suctionstress)*(tan(slope*dg2rad) + (1.0/tan(slope*dg2rad)))*tan(fric_angle*dg2rad)/(uws_depth*depth);
                fc = (2*cohesion_val)/(uws_depth*depth*sin(2*slope*dg2rad));
		if ((i == 5) && (j == 5)) {
                  double sstressfac = 1/(pow((1 + pow((alpha_val*press*uww),n_val)),((n_val-1)/n_val)));
                  printf("Chi factor: %f vs. suction stress: %f\n", chi, sstressfac);
                }
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
           *(DataboxCoeff(suction_stressComp, i, j, fsDataBoxZ)) = fw;
	   } 
      }
   }
}


