/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA519_H
#define UPDATETRIMA519_H

#include "updatetrima5X.h"


class updatetrima519 : public updatetrima5X 
{
public:
   // Default constructor
   updatetrima519();

   // Default destructor
   virtual ~updatetrima519();

private:
   // Copy constructor
   updatetrima519( const updatetrima519 &obj );

protected:

//
// trap_default.dat and trap_override.dat update function 
//
   virtual void updateTrap(TrimaVersion fromVersion);

//
// Update the config file for the version being installed
//
   virtual bool updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion);

   // New kernel names for Python board.  Needs special handling
   virtual bool extractUpdateFiles(void);

//
// sw.dat update function
// we force arm=on for 519
//
   virtual void updateSW();

public:


private:


};
#endif // UPDATETRIMA519_H

