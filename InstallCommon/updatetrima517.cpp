/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima517.h"

using namespace std;

// Default constructor
updatetrima517 :: updatetrima517() :
updatetrima5X()
{
}

// Copy constructor - private so nobody can use it
updatetrima517 :: updatetrima517( const updatetrima517 &obj )
{
}

// Default destructor
updatetrima517 ::  ~updatetrima517()
{
}

void updatetrima517 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion == V510 )
   {
      copyTrapFiles();
   }

   return;
}

bool updatetrima517 :: updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion)
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
         retval = updateConfig510517(datfile);
         break;
      case V517:
      case V518:
         // Do nothing
         break;
      case V520:
      case V521:
      case V522:
         retval = updateConfig52X510(datfile);
         retval |= updateConfig510517(datfile);
         break;
      case V600:
         retval = updateConfig600510(datfile);
         retval |= updateConfig510517(datfile);
         break;
      case V602:
      case V603:
      case V620:
         retval = updateConfig602600(datfile);
         retval |= updateConfig600510(datfile);
         retval |= updateConfig510517(datfile);
         break;
      case V610:
      case V611:
      case V612:
         retval = updateConfig610600(datfile);
         retval |= updateConfig600510(datfile);
         retval |= updateConfig510517(datfile);
         break;
      default:
         break;
   }

   return retval;
}

