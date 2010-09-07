/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrimaEngine.h"

using namespace std;


#ifdef __cplusplus
extern "C" { 
#endif

int updateTrima();

#ifdef __cplusplus
};
#endif

//////////////////////////////////////////////////////////////////////////////////////
//  The main line of update
//////////////////////////////////////////////////////////////////////////////////////
int updateTrima()
{
   //
   // Make sure we don't interrupt anybody else who is running ...
   //
   taskPrioritySet( taskIdSelf(), TRIMA_PRIORITY );

   updatetrimaEngine updater;

   // Kick-off the updating
   if (updater.run() != 0)
   {
       return(-1);
   }
   else
   {
       return(0);
   }
}
                          
