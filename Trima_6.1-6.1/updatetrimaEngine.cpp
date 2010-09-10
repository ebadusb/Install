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
fromTrimaVersion = V61;
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
    // Check if its a 6.0 config file and add this if it is
    // this is used internally only, the customer would never go from 6.0 to 6.1
    if ( !datfile.Find("PROCEDURE_CONFIG", "key_show_pre_aas_flags") )
    {
        datfile.AddLine( "PROCEDURE_CONFIG", "key_show_pre_aas_flags",   "1" );
        return true;
    }

    return false;
}



