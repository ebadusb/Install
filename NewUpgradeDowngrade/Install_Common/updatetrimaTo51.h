/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrimaBase.h"


class updatetrimaTo51 : public updatetrimaBase 
{
public:
   // Default constructor
   updatetrimaTo51();

   // Default destructor
   virtual ~updatetrimaTo51();

private:
   // Copy constructor
   updatetrimaTo51( const updatetrimaTo51 &obj );

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
// Update the config file from 6.X to 5.1
//
   virtual bool updateConfig6Xto51(CDatFileReader& datfile);

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
// since this is for 5.1 to 6.X updates this one does nothing
//
   virtual void updateTrap();

//
// globvars update function
//
   virtual void updateGlobVars();

//
// 5.0 to 5.1 specific globvars update function
//
   virtual bool updateGlobVars50to51(CDatFileReader& datfile);

//
// CRC update & checking
//
   virtual bool checkCRC();

public:


private:


};

                          
