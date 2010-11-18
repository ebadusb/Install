/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA520_H
#define UPDATETRIMA520_H

#include "updatetrima5X.h"


class updatetrima520 : public updatetrima5X 
{
public:
   // Default constructor
   updatetrima520();

   // Default destructor
   virtual ~updatetrima520();

private:
   // Copy constructor
   updatetrima520( const updatetrima520 &obj );

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

//
// setconfig.dat update function
//
   virtual void updateSetConfig();

//
// cal.dat update function
//
   virtual void updateCal();

public:


private:


};
#endif // UPDATETRIMA520_H

