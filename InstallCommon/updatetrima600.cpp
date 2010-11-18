/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima600.h"

using namespace std;

// Default constructor
updatetrima600 :: updatetrima600() :
updatetrima6X()
{
}

// Copy constructor - private so nobody can use it
updatetrima600 :: updatetrima600( const updatetrima600 &obj )
{
}

// Default destructor
updatetrima600 ::  ~updatetrima600()
{
}

void updatetrima600 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion < V600 )
   {
      copyTrapFiles();
   }

   return;
}


