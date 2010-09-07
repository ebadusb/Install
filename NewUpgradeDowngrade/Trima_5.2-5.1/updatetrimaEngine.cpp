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
updatetrimaTo51()
{
fromTrimaVersion = V52;
toTrimaVersion = V51;
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

    // Convert the config file from 5.2 to 5.1
    if (updateConfig52to51(datfile))
    {
        return true;
    }
    else
    {
        return false;
    }

}

