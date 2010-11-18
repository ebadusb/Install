/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima517.h"

using namespace std;

// Default constructor
updatetrima517 :: updatetrima517() :
updatetrima5X()
{
}

// Copy constructor - private so nobody can use it
updatetrima517 :: updatetrima517( const updatetrima517 &obj )
{
}

// Default destructor
updatetrima517 ::  ~updatetrima517()
{
}

void updatetrima517 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion == V510 )
   {
      copyTrapFiles();
   }

   return;
}

int updatetrima517 :: convertTo510(CDatFileReader& datfile)
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

   }

   return(0);
}

bool updatetrima517 :: updateConfigVersion(CDatFileReader& datfile)
{
   bool retval = false;

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

        retval = true;
    }

    return retval;
}

