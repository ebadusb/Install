/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrimaEngine.h"

using namespace std;

// Default constructor
updatetrimaEngine :: updatetrimaEngine() :
updatetrimaTo6X()
{
fromTrimaVersion = V51;
toTrimaVersion = V61;
}

// Copy constructor - private so nobody can use it
updatetrimaEngine :: updatetrimaEngine( const updatetrimaEngine &obj )
{
}

// Default destructor
updatetrimaEngine ::  ~updatetrimaEngine()
{
}


bool updatetrimaEngine :: updateConfigVersion(CDatFileReader& datfile)
{
    // look for this value to make sure it is the right version of the 
    // config file for this update
    if ( datfile.Find("PRODUCT_TEMPLATES", "key_plt_yield_10") )
    {
        return false;
    }

    cerr << "v5.1 config.dat file found. Conversion needed" << endl;

    // Do the updating that is common to 6.0/6.1
    updatetrimaTo6X::updateConfigVersion(datfile);

    // Operators potentially discard flagged products
    datfile.AddLine( "PROCEDURE_CONFIG", "key_show_pre_aas_flags",   "1" );

    return true;
}



