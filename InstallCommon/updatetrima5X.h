/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#ifndef UPDATETRIMA5X_H
#define UPDATETRIMA5X_H

#include "updatetrimaBase.h"


class updatetrima5X : public updatetrimaBase 
{
public:
   // Default constructor
   updatetrima5X();

   // Default destructor
   virtual ~updatetrima5X();

private:
   // Copy constructor
   updatetrima5X( const updatetrima5X &obj );

protected:

//
// does the file extraction
//
   virtual bool extractUpdateFiles();

//
// Update the config file for the version being installed
//
   virtual bool updateConfigVersion(CDatFileReader& datfile);

//
// Main config file update function, calls the others
//
   virtual void updateConfig();

//
// cal.dat update function
//
   virtual void updateCal();

//
// trap_default.dat and trap_override.dat update function 
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

//
// check for inconsistent plasma rinseback settings
//
   virtual bool checkPlasmaRB();

public:


private:


};
#endif // UPDATETRIMA5X_H

                          
