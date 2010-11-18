/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA517_H
#define UPDATETRIMA517_H

#include "updatetrima5X.h"


class updatetrima517 : public updatetrima5X 
{
public:
   // Default constructor
   updatetrima517();

   // Default destructor
   virtual ~updatetrima517();

private:
   // Copy constructor
   updatetrima517( const updatetrima517 &obj );

protected:

//
// trap_default.dat and trap_override.dat update function 
//
   virtual void updateTrap(TrimaVersion fromVersion);

//
// Convert files to 510
//
   virtual int convertTo510(CDatFileReader& datfile);

//
// Update the config file from 5.1.0 to the version being installed
//
   virtual bool updateConfigVersion(CDatFileReader& datfile);

public:


private:


};
#endif // UPDATETRIMA517_H

