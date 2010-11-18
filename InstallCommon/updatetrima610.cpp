/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima610.h"

using namespace std;

// Default constructor
updatetrima610 :: updatetrima610() :
updatetrima6X()
{
}

// Copy constructor - private so nobody can use it
updatetrima610 :: updatetrima610( const updatetrima610 &obj )
{
}

// Default destructor
updatetrima610 ::  ~updatetrima610()
{
}

void updatetrima610 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion < V600 )
   {
      copyTrapFiles();
   }

   return;
}

bool updatetrima610 :: updateConfigVersion(CDatFileReader& datfile)
{
    // look for this value to make sure it is the right version of the 
    // config file for this update
    if ( datfile.Find("PRODUCT_TEMPLATES", "key_plt_yield_10") )
    {
        return false;
    }

    cerr << "v5.1 config.dat file found. Conversion needed" << endl;

    // Do the updating that is common to 6.0/6.1
    updatetrima6X::updateConfigVersion(datfile);

    // Operators potentially discard flagged products
    datfile.AddLine( "PROCEDURE_CONFIG", "key_show_pre_aas_flags",   "1" );

    return true;
}

