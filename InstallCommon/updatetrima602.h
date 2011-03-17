/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef updatetrima602_H
#define updatetrima602_H

#include "updatetrima6X.h"


class updatetrima602 : public updatetrima6X 
{
public:
   // Default constructor
   updatetrima602();

   // Default destructor
   virtual ~updatetrima602();

private:
   // Copy constructor
   updatetrima602( const updatetrima602 &obj );

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
#endif // updatetrima602_H

