/*BHEADER**********************************************************************

**********************************************************************EHEADER*/
/******************************************************************************
 * Error
 *
 * 
 *****************************************************************************/

#include "databox.h"

#include <tcl.h>
#include <string.h>

/*-----------------------------------------------------------------------
 * Error checking routines.
 *-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
 * Check that the grid dimensions are the same.
 *-----------------------------------------------------------------------*/

int      SameDimensions(
   Databox *databoxp,
   Databox *databoxq)
{

   if ((DataboxNx(databoxp) != DataboxNx(databoxq)) ||
       (DataboxNy(databoxp) != DataboxNy(databoxq)) ||
       (DataboxNz(databoxp) != DataboxNz(databoxq))) {
      return 0;

   }

   return 1;

}
  

/*-----------------------------------------------------------------------
 * Make sure a coordinate is within the range of the grid
 *-----------------------------------------------------------------------*/

int     InRange(
   int i, 
   int j, 
   int k,
   Databox *databox)
{

   if ((i < 0 || i >= DataboxNx(databox)) ||
       (j < 0 || j >= DataboxNy(databox)) ||
       (k < 0 || k >= DataboxNz(databox))) {

      return 0;

   }

   return 1;

}


/* Function IsValidFileType - This function is used to make sure a given file */
/* type is a valid one.                                                       */
/*                                                                            */
/* Parameters                                                                 */
/* ----------                                                                 */
/* char *option - The file type option to be validated                        */
/*                                                                            */
/* Return value - int - One if the file type option is valid and zero other-  */
/*                      wise                                                  */

int IsValidFileType (
   char *option)
{
   if (   strcmp(option, "pfb" ) == 0
       || strcmp(option, "pfsb") == 0
       || strcmp(option, "sa"  ) == 0
       || strcmp(option, "sa2d") == 0    // Added @ IMF
       || strcmp(option, "sb"  ) == 0
       || strcmp(option, "fld" ) == 0
       || strcmp(option, "vis" ) == 0
#ifdef HAVE_SILO
       || strcmp(option, "silo" ) == 0
#endif
       || strcmp(option, "rsa" ) == 0 )
      return (1);
   else
      return (0);
}


/* Function GetValidFileExtension - This function is used to determine the    */
/* extension of any given file name and determine if it is valid.             */
/*                                                                            */
/* Parameters                                                                 */
/* ----------                                                                 */
/* char *filename - The filename whose extension will be determined           */
/*                                                                            */
/* Return value - char * - A valid file extension or null if the file's       */
/*                         extension was invalid                              */

char *GetValidFileExtension(
   char *filename)
{
   char *extension;

   /* Point the last character of the string */
   extension = filename + (strlen(filename) - 1);
  
   while (*extension != '.' && extension != filename)
      extension--;

   extension++;
  
   if (IsValidFileType(extension))
      return(extension);

   else
      return (NULL);
}


/*------------------------------------------------------------------------*/
/* These functions append various error messages to the Tcl result.       */
/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 * This function appends a wrong number of arguments error to the Tcl result.
 *------------------------------------------------------------------------*/

void        WrongNumArgsError(
   Tcl_Interp *interp,
   char       *usage)
{
   Tcl_AppendResult(interp, "\nError: Wrong number of arguments\n", usage,
                            (char *) NULL);
}

/*-----------------------------------------------------------------------
 * Assign an undefined dataset error to the tcl result
 *-----------------------------------------------------------------------*/

void        SetNonExistantError(
   Tcl_Interp *interp,
   char       *hashkey)
{
   Tcl_AppendResult(interp, "\nError: `", hashkey, 
                            "' is not a valid set name\n", (char *) NULL);
}

/*-----------------------------------------------------------------------
 * Assign an out of memory error to the tcl result
 *-----------------------------------------------------------------------*/

void	ReadWriteError(Tcl_Interp *interp) 
{
   Tcl_SetResult(interp, "\nError: The file could not be accessed or there is not enough memory available \n", TCL_STATIC);
}

