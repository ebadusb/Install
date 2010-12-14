/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA510_H
#define UPDATETRIMA510_H

#include "updatetrima5X.h"


class updatetrima510 : public updatetrima5X 
{
public:
   // Default constructor
   updatetrima510();

   // Default destructor
   virtual ~updatetrima510();

private:
   // Copy constructor
   updatetrima510( const updatetrima510 &obj );

protected:

//
// Update the config file for the version being installed
//
   virtual bool updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion);

public:


private:


};
#endif // UPDATETRIMA510_H

                          
