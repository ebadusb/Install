/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA610_H
#define UPDATETRIMA610_H

#include "updatetrima6X.h"


class updatetrima610 : public updatetrima6X 
{
public:
   // Default constructor
   updatetrima610();

   // Default destructor
   virtual ~updatetrima610();

private:
   // Copy constructor
   updatetrima610( const updatetrima610 &obj );

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
// Update the config file for the version being installed
//
   virtual bool updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion);

public:


private:


};
#endif // UPDATETRIMA610_H

