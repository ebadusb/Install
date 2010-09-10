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
fromTrimaVersion = V60;
toTrimaVersion = V60;
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
    // Check if its a 6.1 config file and remove this if it is
    // this is used internally only, the customer would never go from 6.1 to 6.0
    if ( datfile.Find("PROCEDURE_CONFIG", "key_show_pre_aas_flags") )
    {
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_show_pre_aas_flags" );
        return true;
    }

    return false;
}




