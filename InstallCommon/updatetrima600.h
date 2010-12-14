/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA600_H
#define UPDATETRIMA600_H

#include "updatetrima6X.h"


class updatetrima600 : public updatetrima6X 
{
public:
   // Default constructor
   updatetrima600();

   // Default destructor
   virtual ~updatetrima600();

private:
   // Copy constructor
   updatetrima600( const updatetrima600 &obj );

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
#endif // UPDATETRIMA600_H

