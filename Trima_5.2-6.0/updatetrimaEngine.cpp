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
fromTrimaVersion = V52;
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

    bool update = false;

    // Convert the config file from 5.2 to 5.1
    update |= updateConfig52to51(datfile);

    // Now convert the config file from 5.1 to 6.0
    update |= updatetrimaTo6X::updateConfigVersion(datfile);

    return update;

}




