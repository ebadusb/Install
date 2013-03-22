/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA630_H
#define UPDATETRIMA630_H

#include "updatetrima6X.h"


class updatetrima630 : public updatetrima6X 
{
public:
   // Default constructor
   updatetrima630();

   // Default destructor
   virtual ~updatetrima630();

private:
   // Copy constructor
   updatetrima630( const updatetrima630 &obj );

protected:

//
// trap_default.dat and trap_override.dat update function 
//
   virtual void updateTrap(TrimaVersion fromVersion);

//
// Override updatePostCount to do nothing for 6.1.0
//
   bool updatePostCount(CDatFileReader& datfile);

//
// setconfig.dat update function
//
   virtual void updateSetConfig();

//
// Update the config file for the version being installed
//
   virtual bool updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion);

public:


private:


};
#endif // UPDATETRIMA630_H

