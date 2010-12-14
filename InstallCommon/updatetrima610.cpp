/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima610.h"

using namespace std;

// Default constructor
updatetrima610 :: updatetrima610() :
updatetrima6X()
{
}

// Copy constructor - private so nobody can use it
updatetrima610 :: updatetrima610( const updatetrima610 &obj )
{
}

// Default destructor
updatetrima610 ::  ~updatetrima610()
{
}

void updatetrima610 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion < V600 )
   {
      copyTrapFiles();
   }

   return;
}

bool updatetrima610 :: updatePostCount(CDatFileReader& datfile)
{
   // Don't update postcount for 6.1.0
   return false;
}

bool updatetrima610 :: updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion)
{
   bool retval = false;

   switch (fromVersion)
   {
      case V510:
      case V512:
      case V513:
      case V514:
      case V515:
      case V516:
         retval = updateConfig5X600(datfile);
         retval |= updateConfig600610(datfile);
         break;
      case V517:
      case V518:
         retval = updateConfig517510(datfile);
         retval |= updateConfig5X600(datfile);
         retval |= updateConfig600610(datfile);
         break;
      case V520:
      case V521:
      case V522:
         retval = updateConfig52X510(datfile);
         retval |= updateConfig5X600(datfile);
         retval |= updateConfig600610(datfile);
         break;
      case V600:
      case V601:
         retval = updateConfig600610(datfile);
         break;
      case V610:
         break;
         // Do nothing
      default:
         break;
   }

   return retval;
}
