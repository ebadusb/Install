/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA518_H
#define UPDATETRIMA518_H

#include "updatetrima5X.h"


class updatetrima518 : public updatetrima5X 
{
public:
   // Default constructor
   updatetrima518();

   // Default destructor
   virtual ~updatetrima518();

private:
   // Copy constructor
   updatetrima518( const updatetrima518 &obj );

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

public:


private:


};
#endif // UPDATETRIMA518_H

