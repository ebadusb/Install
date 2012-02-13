/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima520.h"

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
updatetrima520 :: updatetrima520() :
updatetrima5X()
{
}

// Copy constructor - private so nobody can use it
updatetrima520 :: updatetrima520( const updatetrima520 &obj )
{
}

// Default destructor
updatetrima520 ::  ~updatetrima520()
{
}
/*
void updatetrima520 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion == V510 )
   {
      copyTrapFiles();
   }

   return;
}
*/
bool updatetrima520 :: updatePostCount(CDatFileReader& datfile)
{
   // Don't update postcount for 5.2.0
   return false;
}

bool updatetrima520 :: updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion)
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
         retval = updateConfig510520(datfile);
         break;
      case V517:
      case V518:
         retval = updateConfig517510(datfile);
         retval |= updateConfig510520(datfile);
         break;
      case V520:
      case V521:
      case V522:
         // Do nothing
         break;
      case V600:
          retval = updateConfig600510(datfile);
          retval |= updateConfig510520(datfile);
          break;
      case V602:
      case V603:
          retval = updateConfig602600(datfile);
          retval |= updateConfig600510(datfile);
          retval |= updateConfig510520(datfile);
          break;
      case V610:
      case V611:
      case V612:
         retval = updateConfig610600(datfile);
         retval |= updateConfig600510(datfile);
         retval |= updateConfig510520(datfile);
         break;
      default:
         break;
   }

   return retval;
}

void updatetrima520 :: updateSetConfig()
{
    // these are the customer selected sets.... dont overwrite if it exists! 

    currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_SETCONFIG_DAT);
    //  newVersion = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SETCONFIG_DAT);

    // if the file isnt there....
    if (currVersion == NULL)
    {
        fprintf(stdout, "Adding %s ...\n", FILE_SETCONFIG_DAT);
        // attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "-R");

        if ( cp( TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT ) == ERROR )
        {
            fprintf( stdout, "copy of %s failed\n", FILE_SETCONFIG_DAT );
            return;
        }

        attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
        fflush(stdout);
    }
    else
    {
        fprintf(stdout, "%s already exixst ...\n", FILE_SETCONFIG_DAT);
    }
}

#ifdef REMOVE
void updatetrima520 :: updateCal()
{
    //
    // Create the dat file reader to retrieve the calibration data.
    //
    CDatFileReader datfile(PNAME_CALDAT);
    if ( datfile.Error() )
    {
        cerr << "Calibration file read error : " << datfile.Error() << endl;
        return;
    }

    if ( !datfile.Find("TOUCHSCREEN","screen_horizontal_size") )
    {
        cerr << " ... pre-v5.1 cal.dat file found.  Unable to Convert!  ending..." << endl;
        return;
    }
    cerr << "v5.2 " << FILE_CAL_DAT << " file found.  No conversion needed" << endl;
}
#endif // REMOVE
