/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima514.h"

using namespace std;

// Default constructor
updatetrima514 :: updatetrima514() :
updatetrima5X()
{
}

// Copy constructor - private so nobody can use it
updatetrima514 :: updatetrima514( const updatetrima514 &obj )
{
}

// Default destructor
updatetrima514 ::  ~updatetrima514()
{
}

void updatetrima514 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion == V510 )
   {
      copyTrapFiles();
   }

   return;
}

bool updatetrima514 :: updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion)
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
         // Do nothing
         break;
      case V517:
      case V518:
         retval = updateConfig517510(datfile);
         break;
      case V520:
      case V521:
      case V522:
         retval = updateConfig52X510(datfile);
         break;
      case V600:
         retval = updateConfig600510(datfile);
         break;
      case V602:
      case V603:
      case V620:
         retval = updateConfig602600(datfile);
         retval |= updateConfig600510(datfile);
         break;
      case V610:
      case V611:
      case V612:
         retval = updateConfig610600(datfile);
         retval |= updateConfig600510(datfile);
         break;
      default:
         break;
   }

   return retval;
}


