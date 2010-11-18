/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA6X_H
#define UPDATETRIMA6X_H

#include "updatetrimaBase.h"


class updatetrima6X : public updatetrimaBase 
{
public:
   // Default constructor
   updatetrima6X();

   // Default destructor
   virtual ~updatetrima6X();

private:
   // Copy constructor
   updatetrima6X( const updatetrima6X &obj );

protected:

// 
// The idea here is that after an upgrade it's possible to end up
// with PLT config settings with a combined PAS + PLT volume greater than the
// total bag capacity of 1200.  That's not good.  This function mitigates that. 
// 
   virtual bool checkPasSettings(CDatFileReader& datfile);

// 
// Make sure that RAS settings are within range, since we changed the RAS
// vol range late in development and it has some funky rules (see IT 9113)
// 
   virtual bool checkRasSettings(CDatFileReader& datfile);

//
// does the file extraction
//
   virtual bool extractUpdateFiles();

//
// Update the config file for the version being installed
//
   virtual bool updateConfigVersion(CDatFileReader& datfile);

//
// Update the config file from 6.X to 5.1.0
//
   virtual int convertTo510(CDatFileReader& datfile);

//
// Main config file update function, calls the others
//
   virtual void updateConfig();

//
// cal.dat update function
//
   virtual void updateCal();

//
// trap_default.dat and trap_override.dat update function for 5.1 to 6.X updates
//
   virtual void updateTrap(TrimaVersion fromVersion);

//
// globvars update function
//
   virtual void updateGlobVars();

//
// CRC update & checking
//
   virtual bool checkCRC();

public:


private:


};
#endif // UPDATETRIMA6X_H

                          
