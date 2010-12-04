/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima518.h"

#ifdef __cplusplus
extern "C" { 
#endif

int tarExtract ( const char *file     /* archive file name */, 
                 const char *location /* location for extraction */ );

#ifdef __cplusplus
};
#endif

using namespace std;

// Default constructor
updatetrima518 :: updatetrima518() :
updatetrima5X()
{
}

// Copy constructor - private so nobody can use it
updatetrima518 :: updatetrima518( const updatetrima518 &obj )
{
}

// Default destructor
updatetrima518 ::  ~updatetrima518()
{
}

void updatetrima518 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion == V510 )
   {
      copyTrapFiles();
   }

   return;
}

int updatetrima518 :: convertTo510(CDatFileReader& datfile)
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

bool updatetrima518 :: updateConfigVersion(CDatFileReader& datfile)
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

bool updatetrima518 :: extractUpdateFiles()
{
   // This is done by the extractTopLevelFiles in updatetrima.cpp
   /*
    //
    // Extract the update files
    printf("Extracting updateTrima ...\n" );

    if ( tarExtract( "/machine/update/updateTrima.taz", "/machine/update" ) == ERROR )
    {
        printf("Extraction of update files failed\n" );
        return false;
    }
*/
    //
    // If we booted up using the default vxWorks image, then
    //  save it for emergency cases ...
    //
    BOOT_PARAMS * params = new BOOT_PARAMS;

    bootStringToStruct(sysBootLine, params);
    bool defaultBootImage = ( strcmp(params->bootFile, "/ata0/vxWorks") == 0 );
    delete params;

    if ( defaultBootImage )
    {
        //
        // Save off the old vxWorks image in case of failure ...
        printf("Saving the old OS image..." );
        attrib(VXBOOT_PATH "/vxWorks.old", "-R");

        if ( cp( VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.old" ) == ERROR )
        {
            printf("Archive of old OS image failed\n" );
        }
    }

    //
    // Store the new files in the proper position
    printf("Extracting the OS image...\n" );
    if ( tarExtract( UPDATE_PATH "/vxboot.taz", UPDATE_PATH ) == ERROR )
    {
        printf("Extraction of OS image failed\n" );
        return false;
    }

    /* make the kernel images writable so they can be overwritten */
    attrib(VXBOOT_PATH "/bootrom.sys", "-R");
    attrib(VXBOOT_PATH "/vxWorks", "-R");



    if (isVersalogicPython())
    {
       if ( copyFileContiguous( UPDATE_PATH "/bootrom.sys",  VXBOOT_PATH "/bootrom.sys" ) == ERROR ||
            copyFileContiguous( UPDATE_PATH "/vxWorks_python", VXBOOT_PATH "/vxWorks"     ) == ERROR )
       {
          fprintf( stderr, "Install of Python OS image failed\n" );
          return false;
       }
    }
    else
    {
       if ( copyFileContiguous( UPDATE_PATH "/bootrom.sys",  VXBOOT_PATH "/bootrom.sys" ) == ERROR ||
            copyFileContiguous( UPDATE_PATH "/vxWorks_orig", VXBOOT_PATH "/vxWorks"     ) == ERROR )
       {
          fprintf( stderr, "Install of OS image failed\n" );
          return false;
       }
    }

    //
    // Clean up source files provided in the TAR file.
    //
    if ( remove( UPDATE_PATH "/bootrom.sys" )    == ERROR ||
         remove( UPDATE_PATH "/vxWorks_orig" )   == ERROR ||
         remove( UPDATE_PATH "/vxWorks_python" ) == ERROR ||
         remove( UPDATE_PATH "/vxboot.taz"  )    == ERROR )
    {
       fprintf( stderr, "Removal of temporary OS image failed\n" );
       return false;
    }


    //
    // Remove existing Trima files
    printf("Removing old Trima files...\n" );
    fileSort(TRIMA_PATH,    FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
    fileSort(SAVEDATA_PATH, FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
    fileSort(TOOLS_PATH,    FILE_SORT_BY_DATE_ASCENDING, update_clean_file);

    // IT 7051.  Remove clinical file if exists.
    remove(CLINICAL_BUILD);
    remove(TEST_BUILD);

    //
    // Uncompress the update file
    printf("Extracting the Trima software files...\n" );
    if ( tarExtract( "/machine/update/trima.taz", "/trima" ) == ERROR )
    {
        printf("Extraction of the Trima software failed.\n" );
        return false;
    }

    if ( remove( "/machine/update/trima.taz" ) == ERROR )
    {
        printf("Removal of Trima archive image failed\n" );
        return false;
    }

    return true;

}

