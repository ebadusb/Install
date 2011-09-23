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

bool updatetrima518 :: updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion)
{
   bool retval = false;

   switch (fromVersion)
   {
      case V510:
      case V512:
      case V513:
      case V514:
      case V515:
      case V516:
         retval = updateConfig510517(datfile);
         break;
      case V517:
      case V518:
         // Do nothing
         break;
      case V520:
      case V521:
      case V522:
         retval = updateConfig52X510(datfile);
         retval |= updateConfig510517(datfile);
         break;
      case V600:
         retval = updateConfig600510(datfile);
         retval |= updateConfig510517(datfile);
         break;
      case V602:
      case V603:
         retval = updateConfig602600(datfile);
         retval |= updateConfig600510(datfile);
         retval |= updateConfig510517(datfile);
         break;
      case V610:
         retval = updateConfig610600(datfile);
         retval |= updateConfig600510(datfile);
         retval |= updateConfig510517(datfile);
         break;
      default:
         break;
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

