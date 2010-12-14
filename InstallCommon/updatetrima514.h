/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA514_H
#define UPDATETRIMA514_H

#include "updatetrima5X.h"


class updatetrima514 : public updatetrima5X 
{
public:
   // Default constructor
   updatetrima514();

   // Default destructor
   virtual ~updatetrima514();

private:
   // Copy constructor
   updatetrima514( const updatetrima514 &obj );

protected:

//
// trap_default.dat and trap_override.dat update function 
//
   virtual void updateTrap(TrimaVersion fromVersion);

//
// Update the config file for the version being installed
//
   virtual bool updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion);

public:


private:


};
#endif // UPDATETRIMA514_H

