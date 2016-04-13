 /*BHEADER**********************************************************************

  Copyright (c) 1995-2009, Lawrence Livermore National Security,
  LLC. Produced at the Lawrence Livermore National Laboratory. Written
  by the Parflow Team (see the CONTRIBUTORS file)
  <parflow@lists.llnl.gov> CODE-OCEC-08-103. All rights reserved.

  This file is part of Parflow. For details, see
  http://www.llnl.gov/casc/parflow

  Please read the COPYRIGHT file or Our Notice and the LICENSE file
  for the GNU Lesser General Public License.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License (as published
  by the Free Software Foundation) version 2.1 dated February 1999.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms
  and conditions of the GNU General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA
**********************************************************************EHEADER*/
/*****************************************************************************
 * Program to interactively read, print, etc. different file formats.
 *
 * (C) 1993 Regents of the University of California.
 *
 *----------------------------------------------------------------------------
 * $Revision: 1.48 $
 *
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#include "factorsafety_config.h"   /* Have to figure this out!! */

#include <string.h>
#include <unistd.h>
#include <ctype.h>

#ifdef _WIN32
#define strdup _strdup
#endif

#include "fstools.h"
#include <tcl.h>
#include "grid.h"
#include "error.h"
#include "general.h"


Data    *InitFSToolsData()
{
   Data *new_data;  /* Data structure used to hold data set hash table */

   if ((new_data = (Data*)calloc(1, sizeof (Data))) == NULL)
      return (NULL);

   Tcl_InitHashTable(&DataMembers(new_data), TCL_STRING_KEYS);

   DataGridType(new_data) = cell;
   DataTotalMem(new_data) = 0;
   DataNum(new_data) = 0;

   return new_data;
}

/* Function AddSubgridArray - This function adds a pointer to a new
 * subgrid array to the hash table of subgrid array pointers.  A
 * hash key used to access the pointer is generated automatically.
 *                                                                              
 * Parameters                                                                   
 * ----------                                                                   
 * Data    *data    - The structure containing the hash table                   
 * SubgridArray *databox - Data set pointer to be stored int the hash table          
* char    *label   - Label of used to describe the data set                    
* char    *hashkey - String used as the new data set's hash key                
*                                                                              
* Return value - int - Zero if the space could not be allocated for the        
*                      table entry.  One if the allocation was successful.     
*/

int       AddSubgridArray(
   Data     *data,
   SubgridArray    *subgrid_array,
   char     *label,
   char     *hashkey)
{
   Tcl_HashEntry *entryPtr;  /* Points to new hash table entry         */
   int            new_data;       /* 1 if the hashkey already exists        */
   int            num;       /* The number of the data set to be added */

   num = 0;

   /* Keep tring to find a unique hash key */
   do
   {
      sprintf(hashkey, "subgridarray%d", num); 
      if ((entryPtr = Tcl_CreateHashEntry(&DataMembers(data), hashkey, &new_data))
          == NULL)
         return (0);
      
      num++;

   } while (!new_data);

   /* Truncate the label if it is too large */

   if ((strlen(label) + 1) > MAX_LABEL_SIZE)
      label[MAX_LABEL_SIZE - 1] = 0; 
      
   Tcl_SetHashValue(entryPtr, subgrid_array);

   return (1);
}


/* Function AddData - This function adds a pointer to a new databox to the      */
/* hash table of data set pointers.  A hash key used to access the pointer is   */
/* generated automatically.  The label of the databox is then stored inside     */
/* the databox.                                                                 */
/*                                                                              */
/* Parameters                                                                   */
/* ----------                                                                   */
/* Data    *data    - The structure containing the hash table                   */
/* Databox *databox - Data set pointer to be stored int the hash table          */
/* char    *label   - Label of used to describe the data set                    */
/* char    *hashkey - String used as the new data set's hash key                */
/*                                                                              */
/* Return value - int - Zero if the space could not be allocated for the        */
/*                      table entry.  One if the allocation was successful.     */

int       AddData(
   Data     *data,
   Databox  *databox,
   char     *label,
   char     *hashkey)
{
   Tcl_HashEntry *entryPtr;  /* Points to new hash table entry         */
   int            new_data;       /* 1 if the hashkey already exists        */
   int            num;       /* The number of the data set to be added */

   num = DataNum(data);

   /* Keep tring to find a unique hash key */

   do
   {
      sprintf(hashkey, "dataset%d", num); 
      if ((entryPtr = Tcl_CreateHashEntry(&DataMembers(data), hashkey, &new_data))
          == NULL)
         return (0);
      
      num++;

   } while (!new_data);

   /* Truncate the label if it is too large */

   if ((strlen(label) + 1) > MAX_LABEL_SIZE)
      label[MAX_LABEL_SIZE - 1] = 0; 
      
   strcpy(DataboxLabel(databox), label);
   Tcl_SetHashValue(entryPtr, databox);

   DataNum(data)++;
   DataTotalMem(data)++;

   return (1);

}


/* Function ClientData - This function is an exit procedure the Tcl will        */
/* execute when the `exit' command is entered on the Tcl command line.  This    */
/* routine will deallocate the hash table and the data sets still in memory     */
/* at the time of exit.                                                         */
/*                                                                              */
/* Parameters                                                                   */
/* ----------                                                                   */
/* ClientData clientData - a pointer to the Data structure                      */
/*                                                                              */
/* Return value - void								*/
/*								                */

void               PFTExitProc(
   ClientData clientData)
{
   Tcl_HashSearch  search;
   Tcl_HashEntry  *entryPtr;
   Databox        *databox;

   Data       *data = (Data *)clientData;

   entryPtr = Tcl_FirstHashEntry(&DataMembers(data), &search);

   /* Free the dynamic array in each data box */
   /* and free each data box.                 */

   while (entryPtr != NULL)
   {
      databox = (Databox *)Tcl_GetHashValue(entryPtr);
      FreeDatabox(databox);

      entryPtr = Tcl_NextHashEntry(&search);
   }

   /* Free the hash table */

   Tcl_DeleteHashTable(&DataMembers(data));

   /* Free the struct that was allocated during initialization */

   FreeData(data);
}


   
/*-----------------------------------------------------------------------
 * routine for `computefactorsafety' command
 * Description: Compute the factor of safety
 * 
 * Cmd. syntax: computefactorsafety alpha, n, theta_resid, theta_sat, cohesion, 
                porosity, friction_angle, top, slope_x, slope_y, pressure

 *-----------------------------------------------------------------------*/
int            FactorSafetyCommand(
   ClientData     clientData,
   Tcl_Interp    *interp,
   int            argc,
   char          *argv[])
{
   Tcl_HashEntry *entryPtr;  /* Points to new hash table entry         */
   Data       *data = (Data *)clientData;

   Databox *alpha;
   Databox *n;
   Databox *theta_resid;
   Databox *theta_sat;
   Databox *cohesion;
   Databox *porosity;
   Databox *friction_angle;
   Databox *top;
   Databox *slope_x;
   Databox *slope_y;
   Databox *pressure; 

   char       *filename = "factor safety";
   char       *alpha_hashkey;
   char       *n_hashkey;
   char       *theta_resid_hashkey;
   char       *theta_sat_hashkey;
   char       *cohesion_hashkey;
   char       *porosity_hashkey;
   char       *friction_angle_hashkey;
   char       *top_hashkey;
   char       *slope_x_hashkey;
   char       *slope_y_hashkey;
   char       *pressure_hashkey; 

   char        factor_safety_hashkey[MAX_KEY_SIZE];

   /* Check and see if there is at least one argument following  */
   /* the command.                                               */
   if (argc <= 11)
   {
      WrongNumArgsError(interp, FACTORSAFETYUSAGE);
      return TCL_ERROR;
   }

   alpha_hashkey = argv[1];
   n_hashkey = argv[2];
   theta_resid_hashkey = argv[3];
   theta_sat_hashkey = argv[4];
   cohesion_hashkey = argv[5];
   porosity_hashkey = argv[6];
   friction_angle_hashkey = argv[7];
   top_hashkey = argv[8];
   slope_x_hashkey = argv[9];
   slope_y_hashkey = argv[10];
   pressure_hashkey = argv[11];


   if ((top = DataMember(data, top_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, top_hashkey);
      return TCL_ERROR;
   }

    if ((alpha = DataMember(data, alpha_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, alpha_hashkey);
      return TCL_ERROR;
   }

    if ((n = DataMember(data, n_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, n_hashkey);
      return TCL_ERROR;
   }

    if ((theta_resid = DataMember(data, theta_resid_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, theta_resid_hashkey);
      return TCL_ERROR;
   }

   if ((theta_sat = DataMember(data, theta_sat_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, theta_sat_hashkey);
      return TCL_ERROR;
   }

   if ((cohesion = DataMember(data, cohesion_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, cohesion_hashkey);
      return TCL_ERROR;
   }

   if ((friction_angle = DataMember(data, friction_angle_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, friction_angle_hashkey);
      return TCL_ERROR;
   }

   if ((porosity = DataMember(data, porosity_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, porosity_hashkey);
      return TCL_ERROR;
   }

   if ((slope_x = DataMember(data, slope_x_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, slope_x_hashkey);
      return TCL_ERROR;
   }

   if ((slope_y = DataMember(data, slope_y_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, slope_y_hashkey);
      return TCL_ERROR;
   }

   if ((pressure = DataMember(data, pressure_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, pressure_hashkey);
      return TCL_ERROR;
   }

   {
      int nx = DataboxNx(pressure);
      int ny = DataboxNy(pressure);
      int nz = DataboxNz(pressure);

      double x = DataboxX(pressure);
      double y = DataboxY(pressure);
      double z = DataboxZ(pressure);
      
      double dx = DataboxDx(pressure);
      double dy = DataboxDy(pressure);
      double dz = DataboxDz(pressure);

      /* create the new databox structure for the water table depth  */
      if ( (factor_safety = NewDatabox(nx, ny, nz, x, y, z, dx, dy, dz)) )
      {
    /* Make sure the data set pointer was added to */
    /* the hash table successfully.                */
    if (!AddData(data, factor_safety, filename, factor_safety_hashkey))
       FreeDatabox(factor_safety); 
    else
    {
       Tcl_AppendElement(interp, factor_safety_hashkey); 
    } 

    ComputeFactorSafety(alpha, n, theta_resid, theta_sat, cohesion, porosity, friction_angle,
      top, slope_x, slope_y, pressure, factor_safety);
      }
      else
      {
    ReadWriteError(interp);
    return TCL_ERROR;
      }
   }

   return TCL_OK;
}

   