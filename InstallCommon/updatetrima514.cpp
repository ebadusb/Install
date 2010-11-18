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

