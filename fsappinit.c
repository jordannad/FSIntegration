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

/* 
 * fsappinit.c
 *
 * This file contains routines to add the Factor Safety command (getfactorsafety) to the Tcl
 * interpreter as well as start the appropriate version(command line
 * or GUI and command line).
 *
 */

#include <stdlib.h>

#ifndef _WIN32
#include <sys/param.h>
#endif

#include "factorsafety.h"
#include "fstools.h"

#if defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

/*
 * VC++ has an alternate entry point called DllMain, so we need to rename
 * our entry point.
 */

#if defined(_MSC_VER)
#define EXPORT(a,b) __declspec(dllexport) a b
#define DllEntryPoint DllMain
#else
#define EXPORT(a,b) a b
#endif

EXTERN EXPORT(int,FS_Init) (Tcl_Interp *interp);

/*
 *----------------------------------------------------------------------
 *
 * DllEntryPoint --
 *
 *	This wrapper function is used by Windows to invoke the
 *	initialization code for the DLL.  If we are compiling
 *	with Visual C++, this routine will be renamed to DllMain.
 *	routine.
 *
 * Results:
 *	Returns TRUE;
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#ifdef __WIN32__
BOOL APIENTRY
DllEntryPoint(hInst, reason, reserved)
    HINSTANCE hInst;		/* Library instance handle. */
    DWORD reason;		/* Reason this function is being called. */
    LPVOID reserved;		/* Not used. */
{
    return TRUE;
}
#endif


/* FS_Init - This function is used to add the FS command to Tcl as */
/*            an extension of the Tcl language.  The FS command     */
/*            essentially becomes a new Tcl command executable from the Tcl */
/*            interpreter.                                                */
/*                                                                        */
/* Parameters - Tcl_Interp *interp                                        */
/*                                                                        */
/* Return value - TCL_OK if the FSTools data structures are initialized   */
/*                without error.                                          */
/*                                                                        */
/*                TCL_ERROR if the FSTools data structures cannot be      */
/*                allocated memory.                                       */

EXPORT(int,Factorsafety_Init)(Tcl_Interp *interp)
{
  /*
  if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
    return TCL_ERROR;
  }
  */

   Data *data;

   if ((data = InitFSToolsData()) == NULL) {
      Tcl_SetResult(interp, "Error: Could not initialize data structures for Factor of Safety  function", TCL_STATIC);
      return TCL_ERROR;
   }

   /************************************************************************
     When you add commands here make sure you add the public ones to the
     fstools.tcl with the namespace export command 
     ***********************************************************************/
 
   Tcl_CreateCommand(interp, "FactorSafety::getfactorsafety", (Tcl_CmdProc *)FactorSafetyCommand,
                     (ClientData) data, (Tcl_CmdDeleteProc *) NULL);
   Tcl_CreateCommand(interp, "FactorSafety::getfstop", (Tcl_CmdProc *)FSExtractTopCommand,
                     (ClientData) data, (Tcl_CmdDeleteProc *) NULL);
   Tcl_CreateCommand(interp, "FactorSafety::fspfload", (Tcl_CmdProc *)LoadFSPFCommand,
                     (ClientData) data, (Tcl_CmdDeleteProc *) NULL);
   Tcl_CreateCommand(interp, "FactorSafety::fssave", (Tcl_CmdProc *)SaveFSCommand,
                     (ClientData) data, (Tcl_CmdDeleteProc *) NULL);
   Tcl_CreateCommand(interp, "FactorSafety::getzatfsmin", (Tcl_CmdProc *)ZatFSMinCommand,
                     (ClientData) data, (Tcl_CmdDeleteProc *) NULL);
   Tcl_CreateCommand(interp, "FactorSafety::computepressatfsmin", (Tcl_CmdProc *)PressatFSMinCommand,
                     (ClientData) data, (Tcl_CmdDeleteProc *) NULL);
   
#ifdef SGS
   Tcl_CreateExitHandler((Tcl_ExitProc *)PFTExitProc, (ClientData) data);
#endif

   Tcl_SetVar(interp, "tcl_prompt1", "puts -nonewline {fstools> }",
              TCL_GLOBAL_ONLY);

   return Tcl_PkgProvide(interp, "factorsafety", "1.0");
}




