/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima630.h"

extern bool development_only;

using namespace std;

// Default constructor
updatetrima630 :: updatetrima630() :
updatetrima6X()
{
}

// Copy constructor - private so nobody can use it
updatetrima630 :: updatetrima630( const updatetrima630 &obj )
{
}

// Default destructor
updatetrima630 ::  ~updatetrima630()
{
}

void updatetrima630 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion < V600 )
   {
      copyTrapFiles();
   }

   return;
}

// This overrides the base class to force changes for 6.3 spiral 0
void updatetrima630 :: updateSetConfig()
{
   if ( !development_only )
   {
      if ( cp( TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT ) == ERROR )
      {
         printf("copy of %s failed\n", FILE_SETCONFIG_DAT);
         return;
      }
   
      attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
      fflush(stdout);
   }
}


bool updatetrima630 :: updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion)
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
         retval |= updateConfig602630(datfile);
         break;
      case V517:
      case V518:
      case V519:
         retval = updateConfig517510(datfile);
         retval |= updateConfig5X600(datfile);
         retval |= updateConfig600602(datfile);
         retval |= updateConfig602630(datfile);
         break;
      case V520:
      case V521:
      case V522:
         retval = updateConfig52X510(datfile);
         retval |= updateConfig5X600(datfile);
         retval |= updateConfig600602(datfile);
         retval |= updateConfig602630(datfile);
         break;
      case V600:
      case V603: 
      case V607:
         retval = updateConfig600602(datfile);
         retval |= updateConfig602630(datfile);
         break;
      case V602:
      case V620:
         retval = updateConfig602630(datfile);
         break;
      case V610:
      case V611:
      case V612:
         retval = updateConfig610600(datfile);
         retval |= updateConfig600602(datfile);
         retval |= updateConfig602630(datfile);
         break;
      case V630:
      case V640:
         // Do nothing
         break;
      default:
         break;
   }

   return retval;
}

