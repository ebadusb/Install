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
fromTrimaVersion = V51;
toTrimaVersion = V517;
}

// Copy constructor - private so nobody can use it
updatetrimaEngine :: updatetrimaEngine( const updatetrimaEngine &obj )
{
}

// Default destructor
updatetrimaEngine ::  ~updatetrimaEngine()
{
}

void updatetrimaEngine :: updateTrap()
{

    copyTrapFiles();

    return;
}

bool updatetrimaEngine :: updateConfigVersion(CDatFileReader& datfile)
{
    // If this is a pre 5.1.7 config file
    if ( !datfile.Find("PRODUCT_TEMPLATES","key_plt_amap_single_yield_min") )
    {
        datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_single_yield_min", "3.2" );
        datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_single_yield_max", "4.0" );
        datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_double_yield_min", "6.3" );
        datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_double_yield_max", "8.0" );
        datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_triple_yield_min", "9.5" );
        datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_triple_yield_max", "11.8" );
        datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_single_conc", "1400" );
        datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_double_conc", "1400" );
        datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_triple_conc", "1400" );

        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_a", "0" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_b", "0" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_c", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_d", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_e", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_f", "1" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_g", "1" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_h", "0" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_i", "0" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_j", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_k", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_l", "1" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_m", "1" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_n", "0" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_o", "1" );

        // change these plasma definitions too
        datfile.RemoveLine("PRODUCT_DEFINITIONS", "key_plasma_c");
        datfile.RemoveLine("PRODUCT_DEFINITIONS", "key_plasma_l");
        datfile.RemoveLine("PRODUCT_DEFINITIONS", "key_plasma_m");

        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_c", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_l", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_m", "0" );

        return true;
    }

    return false;
}


