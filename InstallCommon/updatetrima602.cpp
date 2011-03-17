/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima602.h"

using namespace std;

// Default constructor
updatetrima602 :: updatetrima602() :
updatetrima6X()
{
}

// Copy constructor - private so nobody can use it
updatetrima602 :: updatetrima602( const updatetrima602 &obj )
{
}

// Default destructor
updatetrima602 ::  ~updatetrima602()
{
}

void updatetrima602 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion < V600 )
   {
      copyTrapFiles();
   }

   return;
}

bool updatetrima602 :: updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion)
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
         retval |= updateConfig600602(datfile);
         break;
      case V517:
      case V518:
         retval = updateConfig517510(datfile);
         retval |= updateConfig5X600(datfile);
         retval |= updateConfig600602(datfile);
         break;
      case V520:
      case V521:
      case V522:
         retval = updateConfig52X510(datfile);
         retval |= updateConfig5X600(datfile);
         retval |= updateConfig600602(datfile);
         break;
      case V600:
         retval = updateConfig600602(datfile);
         break;
      case V602:
         // Do nothing
         break;
      case V610:
         retval = updateConfig610600(datfile);
         retval |= updateConfig600602(datfile);
         break;
      default:
         break;
   }

   return retval;
}

