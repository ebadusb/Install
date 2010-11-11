/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrimaTo51.h"


class updatetrimaEngine : public updatetrimaTo51 
{
public:
   // Default constructor
   updatetrimaEngine();

   // Default destructor
   virtual ~updatetrimaEngine();

private:
   // Copy constructor
   updatetrimaEngine( const updatetrimaEngine &obj );

protected:

//
   virtual void updateTrap();

   //
   // Update the config file for the version being installed
   //
   virtual bool updateConfigVersion(CDatFileReader& datfile);

   // New kernel names for Python board.  Needs special handling
   virtual bool extractUpdateFiles(void);

public:


private:


};

                          
