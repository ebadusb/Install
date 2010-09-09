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
fromTrimaVersion = V518;
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
    // If this is a 5.1.7 config file
    if ( datfile.Find("PRODUCT_TEMPLATES","key_plt_amap_single_yield_min") )
    {
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_amap_single_yield_min" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_amap_single_yield_max" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_amap_double_yield_min" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_amap_double_yield_max" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_amap_triple_yield_min" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_amap_triple_yield_max" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_amap_single_conc" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_amap_double_conc" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_amap_triple_conc" );

        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_a" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_b" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_c" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_d" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_e" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_f" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_g" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_h" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_i" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_j" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_k" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_l" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_m" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_n" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_o" );


        // this is from the 5.1 to 5.1.7 upgrade, not sure what to do to go from 5.1.7 to 5.1
        // change these plasma definitions too
        /*
        datfile.RemoveLine("PRODUCT_DEFINITIONS", "key_plasma_c");
        datfile.RemoveLine("PRODUCT_DEFINITIONS", "key_plasma_l");
        datfile.RemoveLine("PRODUCT_DEFINITIONS", "key_plasma_m");

        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_c", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_l", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_m", "0" );
        */

        return true;
    }

    return false;
}


