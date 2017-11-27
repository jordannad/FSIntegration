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


#include <string.h>
#include <unistd.h>
#include <ctype.h>

#ifdef _WIN32
#define strdup _strdup
#endif

#include "math.h"
#include "readdatabox.h"
#include "printdatabox.h"
#include "fstools.h"
#include "factorsafety.h"
#include <tcl/tcl.h>
#include "grid.h"
#include "error.h"
#include "general.h"
#include "top.h"


/**
Get an input string from the input database.  If the key is not
found print an error and exit.

There is no checking on what the string contains, anything other than
NUL is allowed. 

@memo Get a string from the input database
@param interp TCL interpreter with the database
@param key The key to search for
@return The string which matches the search key
*/
int GetInt(Tcl_Interp *interp, char *key)
{
   Tcl_Obj *array_name;
   Tcl_Obj *key_name;
   Tcl_Obj *value;

   int ret;

   array_name = Tcl_NewStringObj("Parflow::PFDB", 13);
   key_name = Tcl_NewStringObj(key, strlen(key));

   if ( (value = Tcl_ObjGetVar2(interp, array_name, key_name, 0)) )
   {
      Tcl_GetIntFromObj(interp, value, &ret);
      return ret;
   }
   else
   {
      return -99999999;
   }
}

/**
Get an input string from the input database.  If the key is not
found print an error and exit.

There is no checking on what the string contains, anything other than
NUL is allowed. 

@memo Get a string from the input database
@param interp TCL interpreter with the database
@param key The key to search for
@return The string which matches the search key
*/
int GetIntDefault(Tcl_Interp *interp, char *key, int def)
{
   Tcl_Obj *array_name;
   Tcl_Obj *key_name;
   Tcl_Obj *value;

   int ret;

   array_name = Tcl_NewStringObj("Parflow::PFDB", 13);
   key_name = Tcl_NewStringObj(key, strlen(key));

   if ( (value = Tcl_ObjGetVar2(interp, array_name, key_name, 0)) )
   {
      Tcl_GetIntFromObj(interp, value, &ret);
      return ret;
   }
   else
   {
      return def;
   }
}

/**
Get an input string from the input database.  If the key is not
found print an error and exit.

There is no checking on what the string contains, anything other than
NUL is allowed. 

@memo Get a string from the input database
@param interp TCL interpreter with the database
@param key The key to search for
@return The string which matches the search key
*/
double GetDouble(Tcl_Interp *interp, char *key)
{
   Tcl_Obj *array_name;
   Tcl_Obj *key_name;
   Tcl_Obj *value;

   double ret;

   array_name = Tcl_NewStringObj("Parflow::PFDB", 13);
   key_name = Tcl_NewStringObj(key, strlen(key));

   if ( (value = Tcl_ObjGetVar2(interp, array_name, key_name, 0)) )
   {
      Tcl_GetDoubleFromObj(interp, value, &ret);
      return ret;
   }
   else
   {
      return -99999999;
   }
}

Data    *InitFSToolsData()
{
   
   Data *new_data;  /* Data structure used to hold data set hash table */

   /* Allocate what will eventually hold the hash table */
   if ((new_data = (Data*)calloc(1, sizeof (Data))) == NULL)
      return (NULL);

   /*Initialize the hashtable*/
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
 * routine for `pfload' command
 * Description: One or more arguments are required.  If the first
 *              argument is an option, then it tells what the format of
 *              the following filename is.  If no option is given, then
 *              the filename extension is used to determine the type of
 *              the file.
 * Cmd. syntax: pfload [-option] filename [default_value]
 *-----------------------------------------------------------------------*/

int            LoadFSPFCommand(
   ClientData     clientData,
   Tcl_Interp    *interp,
   int            argc,
   char          *argv[])
{
   printf("I am in here, args: %d", argc);
   Data       *data = (Data *)clientData;

   Databox    *databox;

   char       *filetype, *filename;
   char        newhashkey[MAX_KEY_SIZE];

   double     default_value = 0.0;


   /* Check and see if there is at least one argument following  */
   /* the command.                                               */

   if (argc == 1)
   {
      WrongNumArgsError(interp, LOADPFUSAGE);
      return TCL_ERROR;
   }

   /* Options are preceeded by a dash.  Check to make sure the */
   /* option is valid.                                         */

   if (*argv[1] == '-') 
   {
      /* Skip past the '-' before the file type option */
      filetype = argv[1] + 1;

      if (!IsValidFileType(filetype))
      {
         InvalidOptionError(interp, 1, LOADPFUSAGE);
         return TCL_ERROR;
      }

      /* Make sure a filename follows the option */
      // if (argc == 2 || argc == 3)
      if (argc == 2)
      {
         MissingFilenameError(interp, 1, LOADPFUSAGE);
         return TCL_ERROR;
      }
      else
        filename = argv[2];

      if(argc == 4) {
	 if (Tcl_GetDouble(interp, argv[3], &default_value) == TCL_ERROR)
	 {
	    NotADoubleError(interp, 1, LOADPFUSAGE);
	    return TCL_ERROR;
	 }
      }
	 
   }
   else
   {
      /* If no option is given, then check the extension of the   */
      /* filename.  If the extension on the filename is invalid,  */
      /* then give an error.                                      */

      filename = argv[1];

      /* Make sure the file extension is valid */

      if ((filetype = GetValidFileExtension(filename)) == (char *)NULL)
      {
         InvalidFileExtensionError(interp, 1, LOADPFUSAGE);
         return TCL_ERROR;
      }

      if(argc == 3) {
	 if (Tcl_GetDouble(interp, argv[2], &default_value) == TCL_ERROR)
	 {
	    NotADoubleError(interp, 1, LOADPFUSAGE);
	    return TCL_ERROR;
	 }
      }
   }

   if (strcmp (filetype, "pfb") == 0)
      databox = ReadParflowB(filename, default_value);
   else if (strcmp(filetype, "pfsb") == 0)
      databox = ReadParflowSB(filename, default_value);
   else if (strcmp(filetype, "sa") == 0)
      databox = ReadSimpleA(filename, default_value);
   else if (strcmp(filetype, "sb") == 0)
      databox = ReadSimpleB(filename, default_value);
   else if (strcmp(filetype, "fld") == 0)
      databox = ReadAVSField(filename, default_value);
   else if (strcmp(filetype, "silo") == 0)
      databox = ReadSilo(filename, default_value);
   else
      databox = ReadRealSA(filename, default_value);

   /* Make sure the memory for the data was allocated */

   if (databox)
   {
      /* Make sure the data set pointer was added to */
      /* the hash table successfully.                */

      if (!AddData(data, databox, filename, newhashkey))
         FreeDatabox(databox); 
      else
      {
         Tcl_AppendElement(interp, newhashkey); 
      } 
   }
   else
   {
      ReadWriteError(interp);
      return TCL_ERROR;
   }

   return TCL_OK;

}
   
/*-----------------------------------------------------------------------
 * routine for `computefactorsafety' command
 * Description: Compute the factor of safety
 * 
 * Cmd. syntax: computefactorsafety alpha, n, theta_resid, theta_sat, cohesion, 
                porosity, friction_angle, top, slope_x, slope_y, pressure saturation

 *-----------------------------------------------------------------------*/
int            FactorSafetyCommand(
   ClientData     clientData,
   Tcl_Interp    *interp,
   int            argc,
   char          *argv[])
{

   Data *data = (Data *)clientData;
   Tcl_CmdInfo cmdInfo;
   if (Tcl_GetCommandInfo(interp, "Parflow::pfload", &cmdInfo) == 1) {
      data = cmdInfo.clientData;
      printf("In CmdInfo in fstools factorsafetycommand func... data is: %p\n", data);
   } else {
      printf("ERROR HERE?!?!?!?!\n");
      return TCL_ERROR;
   }
   Tcl_HashEntry *entryPtr;  /* Points to new hash table entry         */
   

   /* Type declarations for variables passed into computefactorsafety function 
    * in factorsafety.c */
   /* inputs */
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
   Databox *saturation;
   Databox *uws_sat;
   double failureDepth;

   /* Outputs */
   Databox *factor_safety;

   /* Hashkeys to databox variables (used by tcl interp?) */
   /* Inputs */
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
   char	      *saturation_hashkey;
   char       *uws_sat_hashkey;

   /* Outputs */
   char       *filename = "factor safety";
   char        factor_safety_hashkey[MAX_KEY_SIZE];
  

   /* Check and see if there are enough arguments following  */
   /* the command.                                           */
   if (argc <= 14)
   {
      printf("Number of arguments = %d\n", argc);
      WrongNumArgsError(interp, FACTORSAFETYUSAGE);
      return TCL_ERROR;
   }

   int k;
   for (k = 1; k < argc; k++) {
    
     printf("Output of argv[%d]= %s \n", k, argv[k]);
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
   saturation_hashkey = argv[12];
   uws_sat_hashkey = argv[13];
  
   if (Tcl_GetDouble(interp, argv[14], &failureDepth) == TCL_ERROR)
   {
     NotADoubleError(interp, 1, FACTORSAFETYUSAGE);
     return TCL_ERROR;
   }
   printf("Got here after double check: %f\n", failureDepth);

   int checkVal = DataTotalMem(data);
   printf("Total number of members in the hashtable: %d\n", checkVal);
   
   if ((alpha = DataMember(data, alpha_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, alpha_hashkey);
      return TCL_ERROR;
   }


   if ((top = DataMember(data, top_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, top_hashkey);
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

    if ((saturation = DataMember(data, saturation_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, saturation_hashkey);
      return TCL_ERROR;
   }

   if ((uws_sat = DataMember(data, uws_sat_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, uws_sat_hashkey);
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
      
      /* Trying out new size of data structure for factor safety databox */
      int fs_nz = round(failureDepth/dz);

      printf("Got here check nz = %d, fs_nz = %d\n", nz, fs_nz);

      /* create the new databox structure for the water table depth  */
      if ( (factor_safety = NewDatabox(nx, ny, fs_nz, x, y, z, dx, dy, dz)) )
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
      top, slope_x, slope_y, pressure, saturation, uws_sat, factor_safety, failureDepth);
      }
      else
      {
    ReadWriteError(interp);
    return TCL_ERROR;
      }
   }

   return TCL_OK;
}


/*-----------------------------------------------------------------------
 * routine for `fsextracttop' command
 * Description: Extract the top cells of a dataset.
 * 
 * Cmd. syntax: fsextracttop top data
 *-----------------------------------------------------------------------*/
int            FSExtractTopCommand(
   ClientData     clientData,
   Tcl_Interp    *interp,
   int            argc,
   char          *argv[])
{
   Tcl_HashEntry *entryPtr;  /* Points to new hash table entry         */
   Data       *data = (Data *)clientData;

   Databox    *top;
   Databox    *databox;
   Databox    *top_values;

   char       *filename = "top values";
   char       *top_hashkey;
   char       *data_hashkey;

   char        newhashkey[MAX_KEY_SIZE];

   /* Check and see if there is at least one argument following  */
   /* the command.                                               */
   if (argc == 2)
   {
      WrongNumArgsError(interp, FSEXTRACTTOPUSAGE);
      return TCL_ERROR;
   }

   top_hashkey = argv[1];
   data_hashkey = argv[2];

   if ((top = DataMember(data, top_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, top_hashkey);
      return TCL_ERROR;
   }

   if ((databox = DataMember(data, data_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, data_hashkey);
      return TCL_ERROR;
   }

   {
      int nx = DataboxNx(databox);
      int ny = DataboxNy(databox);

      double x = DataboxX(databox);
      double y = DataboxY(databox);
      double z = DataboxZ(databox);
      
      double dx = DataboxDx(databox);
      double dy = DataboxDy(databox);
      double dz = DataboxDz(databox);

      /* create the new databox structure for top */
      if ( (top_values = NewDatabox(nx, ny, 1, x, y, z, dx, dy, dz)) )
      {
	 /* Make sure the data set pointer was added to */
	 /* the hash table successfully.                */

	 if (!AddData(data, top_values, filename, newhashkey))
	    FreeDatabox(top); 
	 else
	 {
	    Tcl_AppendElement(interp, newhashkey); 
	 } 

	 ExtractTop(top, databox, top_values);
      }
      else
      {
	 ReadWriteError(interp);
	 return TCL_ERROR;
      }
   }

   return TCL_OK;
}

/*-----------------------------------------------------------------------
 * routine for `fssave' command
 * Description: The first argument to this command is the hashkey of the
 *              dataset to be saved, the second is the format of the 
 *              file the data is to be saved in.
 * Cmd. syntax: fssave dataset -filetype filename
 *-----------------------------------------------------------------------*/

int               SaveFSCommand(
   ClientData        clientData,
   Tcl_Interp       *interp,
   int               argc,
   char             *argv[])
{
   Data          *data = (Data *)clientData;

   char          *filetype, *filename;
   FILE          *fp = NULL;

   char          *hashkey;
   Tcl_HashEntry *entryPtr; 
   Databox       *databox;

    

   /* The command three arguments */

   if (argc != 4)
   {
      WrongNumArgsError(interp, SAVEFSUSAGE);
      return TCL_ERROR;
   }

   hashkey = argv[1];

   /* Make sure the dataset exists */

   if ((databox = DataMember(data, hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, hashkey);
      return TCL_ERROR;
   }

   /* Check for an option specifying the file type */

   if (*argv[2] != '-')
   {
      MissingOptionError(interp, 1, SAVEFSUSAGE);
      return TCL_ERROR;
   }

   filetype = argv[2] + 1;

   /* Validate the file type */

   if (!IsValidFileType(filetype))
   {
      InvalidOptionError(interp, 1, SAVEFSUSAGE);
      return TCL_ERROR;
   }

   filename = argv[3];

   /* Execute the appropriate file printing routine */
   if (strcmp(filetype, "pfb") == 0) {
      /* Make sure the file could be opened */
      if ((fp = fopen(filename, "wb")) == NULL)
      {
	 ReadWriteError(interp);
	 return TCL_ERROR;
      }
      
      PrintParflowB(fp, databox);
   }
   else if (strcmp(filetype, "sa") == 0) {
      /* Make sure the file could be opened */
      if ((fp = fopen(filename, "wb")) == NULL)
      {
	 ReadWriteError(interp);
	 return TCL_ERROR;
      }
      
      PrintSimpleA(fp, databox);
   }


   else if (strcmp(filetype, "sa2d") == 0) {
      /* Make sure the file could be opened */
      if ((fp = fopen(filename, "wb")) == NULL)
      {
         ReadWriteError(interp);
         return TCL_ERROR;
      }

      PrintSimpleA2D(fp, databox);
   }   


   else if (strcmp(filetype, "sb") == 0) {
      /* Make sure the file could be opened */
      if ((fp = fopen(filename, "wb")) == NULL)
      {
	 ReadWriteError(interp);
	 return TCL_ERROR;
      }
      
      PrintSimpleB(fp, databox);
   }
   else if (strcmp(filetype, "fld") == 0) {
      /* Make sure the file could be opened */
      if ((fp = fopen(filename, "wb")) == NULL)
      {
	 ReadWriteError(interp);
	 return TCL_ERROR;
      }
      
      PrintAVSField(fp, databox);
   }
   else if (strcmp(filetype, "vis") == 0) {
      /* Make sure the file could be opened */
      if ((fp = fopen(filename, "wb")) == NULL)
      {
	 ReadWriteError(interp);
	 return TCL_ERROR;
      }
      
      PrintVizamrai(fp, databox);
   }
#ifdef HAVE_SILO
   else if (strcmp(filetype, "silo") == 0) {
      PrintSilo(filename, databox);
   }
#endif

   /* Close the file, if opened */
   if(fp) {
      fclose(fp);
   }
   return TCL_OK;
}   


/*-----------------------------------------------------------------------
 * routine for `computezatfsmin' command
 * Description: Compute the depth associated with the minimum factor of safety
 * 		for shallow rain induced landslides
 * Cmd. syntax: computezatfismin factorsafety

 *-----------------------------------------------------------------------*/
int            ZatFSMinCommand(
   ClientData     clientData,
   Tcl_Interp    *interp,
   int            argc,
   char          *argv[])
{

   /* Grab the parflow data pointer */
   printf("Got here %d \n", argc);
   Data *data = (Data *)clientData;
   Tcl_CmdInfo cmdInfo;
   if (Tcl_GetCommandInfo(interp, "Parflow::pfload", &cmdInfo) == 1) {
      data = cmdInfo.clientData;
      printf("In CmdInfo in fstools computezatfsmincommand func... data is: %p\n", data);
   } else {
      printf("ERROR HERE?!?!?!?!\n");
      return TCL_ERROR;
   }
   Tcl_HashEntry *entryPtr;  /* Points to new hash table entry         */
   

   /* Type declarations for variables passed into computefactorsafety function 
    * in factorsafety.c */
   /* inputs */
   Databox *factor_safety;

   /* Hashkeys to databox variables (used by tcl interp?) */
   /* Inputs */
   char       *factor_safety_hashkey;

   /* Outputs */
   Databox    *zminfs;
   char       *filename = "depth minFS";
   char        zminfs_hashkey[MAX_KEY_SIZE];
  

   /* Check and see if there are enough arguments following  */
   /* the command.                                           */
   if (argc <= 1)
   {
      printf("Number of arguments = %d\n", argc);
      WrongNumArgsError(interp, ZMINUSAGE);
      return TCL_ERROR;
   }

   int k;
   for (k = 0; k < argc; k++) {
    
     printf("Output of argv[%d]= %s \n", k, argv[k]);
   }
   factor_safety_hashkey = argv[1];

   /* int checkVal = DataTotalMem(data);
   printf("Total number of members in the hashtable: %d\n", checkVal); */
   
   if ((factor_safety = DataMember(data, factor_safety_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, factor_safety_hashkey);
      return TCL_ERROR;
   }

    
   {
      int nx = DataboxNx(factor_safety);
      int ny = DataboxNy(factor_safety);
      int nz = 1;

      double x = DataboxX(factor_safety);
      double y = DataboxY(factor_safety);
      double z = DataboxZ(factor_safety);
      
      double dx = DataboxDx(factor_safety);
      double dy = DataboxDy(factor_safety);
      double dz = DataboxDz(factor_safety);
     
      printf("nx = %d, ny = %d, dx = %f, dy = %f, dz = %f\n", nx, ny, dx, dy, dz);
      /* create the new databox structure for the depth where factor of safety is minimum  */
      if ( (zminfs = NewDatabox(nx, ny, 1, x, y, z, dx, dy, dz)) )
      {
    /* Make sure the data set pointer was added to */
    /* the hash table successfully.                */
    if (!AddData(data, zminfs, filename, zminfs_hashkey)) {
       printf("Right before call to FreeDatabox");
       FreeDatabox(zminfs); 
    } else
    {
       Tcl_AppendElement(interp, zminfs_hashkey); 
    } 

    ComputeZMin(factor_safety, zminfs);
    /*printf("Error in free() happens after this point 1");*/
      }
      else
      {
    ReadWriteError(interp);
    return TCL_ERROR;
      }
   }

   /*printf("Error in free() happens after this point 2");*/
   return TCL_OK;
}


/*-----------------------------------------------------------------------
 * routine for `computepressatfsmin' command
 * Description: Compute the depth associated with the minimum factor of safety
 * 		for shallow rain induced landslides
 * Cmd. syntax: computezatfismin factorsafety

 *-----------------------------------------------------------------------*/
int            PressatFSMinCommand(
   ClientData     clientData,
   Tcl_Interp    *interp,
   int            argc,
   char          *argv[])
{

   /* Grab the parflow data pointer */
   printf("Got here in press at fs min command, numargs = %d \n", argc);
   Data *data = (Data *)clientData;
   Tcl_CmdInfo cmdInfo;
   if (Tcl_GetCommandInfo(interp, "Parflow::pfload", &cmdInfo) == 1) {
      data = cmdInfo.clientData;
      printf("In CmdInfo in fstools computezatfsmincommand func... data is: %p\n", data);
   } else {
      printf("ERROR HERE?!?!?!?!\n");
      return TCL_ERROR;
   }
   Tcl_HashEntry *entryPtr;  /* Points to new hash table entry         */
   

   /* Type declarations for variables passed into computefactorsafety function 
    * in factorsafety.c */
   /* inputs */
   Databox *factor_safety;
   Databox *pressure;
   Databox *top;

   /* Hashkeys to databox variables (used by tcl interp?) */
   /* Inputs */
   char       *factor_safety_hashkey;
   char       *pressure_hashkey;
   char       *top_hashkey;

   /* Outputs */
   Databox    *pressatminfs;
   char       *filename = "pressure minFS";
   char        pressatminfs_hashkey[MAX_KEY_SIZE];
  

   /* Check and see if there are enough arguments following  */
   /* the command.                                           */
   if (argc <= 3)
   {
      printf("Number of arguments = %d\n", argc);
      WrongNumArgsError(interp, PRESSATFSMINUSAGE);
      return TCL_ERROR;
   }

   int k;
   for (k = 0; k < argc; k++) {
    
     printf("Output of argv[%d]= %s \n", k, argv[k]);
   }
   factor_safety_hashkey = argv[1];
   pressure_hashkey = argv[2];
   top_hashkey = argv[3];

   /* int checkVal = DataTotalMem(data);
   printf("Total number of members in the hashtable: %d\n", checkVal); */
   
   if ((factor_safety = DataMember(data, factor_safety_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, factor_safety_hashkey);
      return TCL_ERROR;
   }

   if ((pressure = DataMember(data, pressure_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, pressure_hashkey);
      return TCL_ERROR;
   }

   if ((top = DataMember(data, top_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, top_hashkey);
      return TCL_ERROR;
   }

    
   {
      int nx = DataboxNx(factor_safety);
      int ny = DataboxNy(factor_safety);
      int nz = 1;

      double x = DataboxX(factor_safety);
      double y = DataboxY(factor_safety);
      double z = DataboxZ(factor_safety);
      
      double dx = DataboxDx(factor_safety);
      double dy = DataboxDy(factor_safety);
      double dz = DataboxDz(factor_safety);
     
      printf("nx = %d, ny = %d, dx = %f, dy = %f, dz = %f\n", nx, ny, dx, dy, dz);
      /* create the new databox structure for the depth where factor of safety is minimum  */
      if ( (pressatminfs = NewDatabox(nx, ny, 1, x, y, z, dx, dy, dz)) )
      {
    /* Make sure the data set pointer was added to */
    /* the hash table successfully.                */
    if (!AddData(data, pressatminfs, filename, pressatminfs_hashkey)) {
       printf("Right before call to FreeDatabox");
       FreeDatabox(pressatminfs); 
    } else
    {
       Tcl_AppendElement(interp, pressatminfs_hashkey); 
    } 

    ComputePressatFSMin(factor_safety, pressure, top, pressatminfs);
    /*printf("Error in free() happens after this point 1");*/
      }
      else
      {
    ReadWriteError(interp);
    return TCL_ERROR;
      }
   }

   /*printf("Error in free() happens after this point 2");*/
   return TCL_OK;
}


/*-----------------------------------------------------------------------
 * routine for `getlulikosfs' command
 * Description: Compute the factor of safety using alpha and n parameters
 *              Computation should be consistent with TRIGRS
 * 
 * Cmd. syntax: computefactorsafety alpha, n, theta_resid, theta_sat, cohesion, 
                porosity, friction_angle, top, slope_x, slope_y, pressure saturation

 *-----------------------------------------------------------------------*/
int            LuLikosFSCommand(
   ClientData     clientData,
   Tcl_Interp    *interp,
   int            argc,
   char          *argv[])
{

   Data *data = (Data *)clientData;
   Tcl_CmdInfo cmdInfo;
   if (Tcl_GetCommandInfo(interp, "Parflow::pfload", &cmdInfo) == 1) {
      data = cmdInfo.clientData;
      printf("In CmdInfo in fstools factorsafetycommand func... data is: %p\n", data);
   } else {
      printf("ERROR HERE?!?!?!?!\n");
      return TCL_ERROR;
   }
   Tcl_HashEntry *entryPtr;  /* Points to new hash table entry         */
   

   /* Type declarations for variables passed into computefactorsafety function 
    * in factorsafety.c */
   /* inputs */
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
   Databox *saturation;
   Databox *uws_sat;
   double failureDepth;

   /* Outputs */
   Databox *factor_safety;

   /* Hashkeys to databox variables (used by tcl interp?) */
   /* Inputs */
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
   char	      *saturation_hashkey;
   char       *uws_sat_hashkey;

   /* Outputs */
   char       *filename = "factor safety";
   char        factor_safety_hashkey[MAX_KEY_SIZE];
  

   /* Check and see if there are enough arguments following  */
   /* the command.                                           */
   if (argc <= 14)
   {
      printf("Number of arguments = %d\n", argc);
      WrongNumArgsError(interp, FACTORSAFETYUSAGE);
      return TCL_ERROR;
   }

   int k;
   for (k = 1; k < argc; k++) {
    
     printf("Output of argv[%d]= %s \n", k, argv[k]);
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
   saturation_hashkey = argv[12];
   uws_sat_hashkey = argv[13];
  
   if (Tcl_GetDouble(interp, argv[14], &failureDepth) == TCL_ERROR)
   {
     NotADoubleError(interp, 1, FACTORSAFETYUSAGE);
     return TCL_ERROR;
   }
   printf("Got here after double check: %f\n", failureDepth);

   int checkVal = DataTotalMem(data);
   printf("Total number of members in the hashtable: %d\n", checkVal);
   
   if ((alpha = DataMember(data, alpha_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, alpha_hashkey);
      return TCL_ERROR;
   }


   if ((top = DataMember(data, top_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, top_hashkey);
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

    if ((saturation = DataMember(data, saturation_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, saturation_hashkey);
      return TCL_ERROR;
   }

   if ((uws_sat = DataMember(data, uws_sat_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, uws_sat_hashkey);
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
      
      /* Trying out new size of data structure for factor safety databox */
      int fs_nz = round(failureDepth/dz);

      printf("Got here check nz = %d, fs_nz = %d\n", nz, fs_nz);

      /* create the new databox structure for the water table depth  */
      if ( (factor_safety = NewDatabox(nx, ny, fs_nz, x, y, z, dx, dy, dz)) )
      {
    /* Make sure the data set pointer was added to */
    /* the hash table successfully.                */
    if (!AddData(data, factor_safety, filename, factor_safety_hashkey))
       FreeDatabox(factor_safety); 
    else
    {
       Tcl_AppendElement(interp, factor_safety_hashkey); 
    } 

    ComputeLuLikosFS(alpha, n, theta_resid, theta_sat, cohesion, porosity, friction_angle,
      top, slope_x, slope_y, pressure, saturation, uws_sat, factor_safety, failureDepth);
      }
      else
      {
    ReadWriteError(interp);
    return TCL_ERROR;
      }
   }

   return TCL_OK;
}



/*-----------------------------------------------------------------------
 * routine for `getfswithrootstrength' command
 * Description: Compute the factor of safety using alpha and n parameters
 *              Computation should be consistent with TRIGRS
 * 
 * Cmd. syntax: getfswithrootstrength alpha, n, theta_resid, theta_sat, cohesion, 
                root cohesion, porosity, friction_angle, top, slope_x, slope_y, 
                pressure, saturation, uws_sat, tree_surcharge, maxFailureDepth

 *-----------------------------------------------------------------------*/
int            FSWithRootStrengthCommand(
   ClientData     clientData,
   Tcl_Interp    *interp,
   int            argc,
   char          *argv[])
{

   Data *data = (Data *)clientData;
   Tcl_CmdInfo cmdInfo;
   if (Tcl_GetCommandInfo(interp, "Parflow::pfload", &cmdInfo) == 1) {
      data = cmdInfo.clientData;
      printf("In CmdInfo in fstools factorsafetycommand func... data is: %p\n", data);
   } else {
      printf("ERROR HERE?!?!?!?!\n");
      return TCL_ERROR;
   }
   Tcl_HashEntry *entryPtr;  /* Points to new hash table entry         */
   

   /* Type declarations for variables passed into computefactorsafety function 
    * in factorsafety.c */
   /* inputs */
   Databox *alpha;
   Databox *n;
   Databox *theta_resid;
   Databox *theta_sat;
   Databox *cohesion;
   Databox *root_cohesion;
   Databox *porosity;
   Databox *friction_angle;
   Databox *top;
   Databox *slope_x;
   Databox *slope_y;
   Databox *pressure; 
   Databox *saturation;
   Databox *uws_sat;
   Databox *tree_surcharge;
   double maxfailureDepth;

   /* Outputs */
   Databox *factor_safety;

   /* Hashkeys to databox variables (used by tcl interp?) */
   /* Inputs */
   char       *alpha_hashkey;
   char       *n_hashkey;
   char       *theta_resid_hashkey;
   char       *theta_sat_hashkey;
   char       *cohesion_hashkey;
   char       *root_cohesion_hashkey;
   char       *porosity_hashkey;
   char       *friction_angle_hashkey;
   char       *top_hashkey;
   char       *slope_x_hashkey;
   char       *slope_y_hashkey;
   char       *pressure_hashkey; 
   char	      *saturation_hashkey;
   char       *uws_sat_hashkey;
   char       *tree_surcharge_hashkey;

   /* Outputs */
   char       *filename = "factor safety";
   char        factor_safety_hashkey[MAX_KEY_SIZE];
  

   /* Check and see if there are enough arguments following  */
   /* the command.                                           */
   if (argc <= 16)
   {
      printf("Number of arguments = %d\n", argc);
      WrongNumArgsError(interp, FACTORSAFETYUSAGE);
      return TCL_ERROR;
   }

   int k;
   for (k = 1; k < argc; k++) {
    
     printf("Output of argv[%d]= %s \n", k, argv[k]);
   }
   alpha_hashkey = argv[1];
   n_hashkey = argv[2];
   theta_resid_hashkey = argv[3];
   theta_sat_hashkey = argv[4];
   cohesion_hashkey = argv[5];
   root_cohesion_hashkey = argv[6];
   porosity_hashkey = argv[7];
   friction_angle_hashkey = argv[8];
   top_hashkey = argv[9];
   slope_x_hashkey = argv[10];
   slope_y_hashkey = argv[11];
   pressure_hashkey = argv[12];
   saturation_hashkey = argv[13];
   uws_sat_hashkey = argv[14];
   tree_surcharge_hashkey = argv[15];
  
   if (Tcl_GetDouble(interp, argv[16], &maxfailureDepth) == TCL_ERROR)
   {
     NotADoubleError(interp, 1, FACTORSAFETYUSAGE);
     return TCL_ERROR;
   }
   printf("Got here after double check: %f\n", maxfailureDepth);

   int checkVal = DataTotalMem(data);
   printf("Total number of members in the hashtable: %d\n", checkVal);
   
   if ((alpha = DataMember(data, alpha_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, alpha_hashkey);
      return TCL_ERROR;
   }


   if ((top = DataMember(data, top_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, top_hashkey);
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

   if ((root_cohesion = DataMember(data, root_cohesion_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, root_cohesion_hashkey);
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

    if ((saturation = DataMember(data, saturation_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, saturation_hashkey);
      return TCL_ERROR;
   }

   if ((uws_sat = DataMember(data, uws_sat_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, uws_sat_hashkey);
      return TCL_ERROR;
   }

   if ((tree_surcharge = DataMember(data, tree_surcharge_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, tree_surcharge_hashkey);
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
      
      /* Trying out new size of data structure for factor safety databox */
      int fs_nz = round(maxfailureDepth/dz);

      printf("Got here check nz = %d, fs_nz = %d\n", nz, fs_nz);

      /* create the new databox structure for the water table depth  */
      if ( (factor_safety = NewDatabox(nx, ny, fs_nz, x, y, z, dx, dy, dz)) )
      {
    /* Make sure the data set pointer was added to */
    /* the hash table successfully.                */
    if (!AddData(data, factor_safety, filename, factor_safety_hashkey))
       FreeDatabox(factor_safety); 
    else
    {
       Tcl_AppendElement(interp, factor_safety_hashkey); 
    } 

    ComputeFSWithRootStrength(alpha, n, theta_resid, theta_sat, cohesion, root_cohesion, porosity, friction_angle, top, slope_x, slope_y, pressure, saturation, uws_sat, factor_safety, tree_surcharge, maxfailureDepth);
      }
      else
      {
    ReadWriteError(interp);
    return TCL_ERROR;
      }
   }

   return TCL_OK;
}


/*-----------------------------------------------------------------------
 * routine for `getsuctioncompfs' command
 * Description: Compute the factor of safety using alpha and n parameters
 *              Computation should be consistent with TRIGRS
 * 
 * Cmd. syntax: getfssuctioncomp alpha, n, theta_resid, theta_sat, cohesion, 
                porosity, friction_angle, top, slope_x, slope_y, pressure saturation

 *-----------------------------------------------------------------------*/
int            FSSuctionComponentCommand(
   ClientData     clientData,
   Tcl_Interp    *interp,
   int            argc,
   char          *argv[])
{

   Data *data = (Data *)clientData;
   Tcl_CmdInfo cmdInfo;
   if (Tcl_GetCommandInfo(interp, "Parflow::pfload", &cmdInfo) == 1) {
      data = cmdInfo.clientData;
      printf("In CmdInfo in fstools factorsafetycommand func... data is: %p\n", data);
   } else {
      printf("ERROR HERE?!?!?!?!\n");
      return TCL_ERROR;
   }
   Tcl_HashEntry *entryPtr;  /* Points to new hash table entry         */
   

   /* Type declarations for variables passed into computefactorsafety function 
    * in factorsafety.c */
   /* inputs */
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
   Databox *saturation;
   Databox *uws_sat;
   double failureDepth;

   /* Outputs */
   Databox *suction_stressComp;

   /* Hashkeys to databox variables (used by tcl interp?) */
   /* Inputs */
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
   char	      *saturation_hashkey;
   char       *uws_sat_hashkey;

   /* Outputs */
   char       *filename = "fssuctionComp";
   char        suction_stressComp_hashkey[MAX_KEY_SIZE];
  

   /* Check and see if there are enough arguments following  */
   /* the command.                                           */
   if (argc <= 14)
   {
      printf("Number of arguments = %d\n", argc);
      WrongNumArgsError(interp, FACTORSAFETYUSAGE);
      return TCL_ERROR;
   }

   int k;
   for (k = 1; k < argc; k++) {
    
     printf("Output of argv[%d]= %s \n", k, argv[k]);
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
   saturation_hashkey = argv[12];
   uws_sat_hashkey = argv[13];
  
   if (Tcl_GetDouble(interp, argv[14], &failureDepth) == TCL_ERROR)
   {
     NotADoubleError(interp, 1, FACTORSAFETYUSAGE);
     return TCL_ERROR;
   }
   printf("Got here after double check: %f\n", failureDepth);

   int checkVal = DataTotalMem(data);
   printf("Total number of members in the hashtable: %d\n", checkVal);
   
   if ((alpha = DataMember(data, alpha_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, alpha_hashkey);
      return TCL_ERROR;
   }


   if ((top = DataMember(data, top_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, top_hashkey);
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

    if ((saturation = DataMember(data, saturation_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, saturation_hashkey);
      return TCL_ERROR;
   }

   if ((uws_sat = DataMember(data, uws_sat_hashkey, entryPtr)) == NULL)
   {
      SetNonExistantError(interp, uws_sat_hashkey);
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
      
      /* Trying out new size of data structure for factor safety databox */
      int fs_nz = round(failureDepth/dz);

      printf("Got here check nz = %d, fs_nz = %d\n", nz, fs_nz);

      /* create the new databox structure for the water table depth  */
      if ( (suction_stressComp = NewDatabox(nx, ny, fs_nz, x, y, z, dx, dy, dz)) )
      {
    /* Make sure the data set pointer was added to */
    /* the hash table successfully.                */
    if (!AddData(data, suction_stressComp, filename, suction_stressComp_hashkey))
       FreeDatabox(suction_stressComp); 
    else
    {
       Tcl_AppendElement(interp, suction_stressComp_hashkey); 
    } 

    GetFSSuctionComp(alpha, n, theta_resid, theta_sat, cohesion, porosity, friction_angle,
      top, slope_x, slope_y, pressure, saturation, uws_sat, suction_stressComp, failureDepth);
      }
      else
      {
    ReadWriteError(interp);
    return TCL_ERROR;
      }
   }

   return TCL_OK;
}
