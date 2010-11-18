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

void updatetrima520 :: updateTrap(TrimaVersion fromVersion)
{
   if ( fromVersion == V510 )
   {
      copyTrapFiles();
   }

   return;
}

int updatetrima520 :: convertTo510(CDatFileReader& datfile)
{
   // check if 5.2 by looking for a new parameter.......
   if ( datfile.Find("PROCEDURE_CONFIG", "key_mss_plt_on") )
   {
      //////////////////////////////////////////////////////////////////////////////////
      //                 5.P (5.2)-->5.1 changes
      //////////////////////////////////////////////////////////////////////////////////
   
      datfile.RemoveLine( "PROCEDURE_CONFIG", "key_plt_mss_split_notif" );
      datfile.RemoveLine( "PROCEDURE_CONFIG", "key_override_pas_bag_vol" );
      datfile.RemoveLine( "PROCEDURE_CONFIG", "key_blood_diversion" );
      datfile.RemoveLine( "PROCEDURE_CONFIG", "key_mss_plt_on" );
      datfile.RemoveLine( "PROCEDURE_CONFIG", "key_plt_def_bag_vol" );
   
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_1" );
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_1" ); 
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_2" );
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_2" ); 
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_3" );
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_3" ); 
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_4" );
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_4" ); 
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_5" );
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_5" ); 
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_6" );
      datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_6" ); 
   
      cerr << "config.dat file converted." << endl;
   }

   return(0);
}

bool updatetrima520 :: updateConfigVersion(CDatFileReader& datfile)
{
   bool retval = false;

   // check if 5.P by looking for a new parameter.......
   if ( !datfile.Find("PROCEDURE_CONFIG", "key_mss_plt_on") )
   {
   
      //////////////////////////////////////////////////////////////////////////////////
      //                 5.1-->5.P (5.2) changes
      //////////////////////////////////////////////////////////////////////////////////
   
      datfile.AddLine( "PROCEDURE_CONFIG", "key_plt_mss_split_notif",  "0" );
      datfile.AddLine( "PROCEDURE_CONFIG", "key_override_pas_bag_vol", "0" );
      datfile.AddLine( "PROCEDURE_CONFIG", "key_blood_diversion",      "0" );
      datfile.AddLine( "PROCEDURE_CONFIG", "key_mss_plt_on",           "1" );
      datfile.AddLine( "PROCEDURE_CONFIG", "key_plt_def_bag_vol",      "250" );
   
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_1",            "0" );
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_1", "50" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_2",            "0" );
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_2", "50" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_3",            "0" );
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_3", "50" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_4",            "0" );
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_4", "50" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_5",            "0" );
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_5", "50" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_6",            "0" );
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_6", "50" ); 
   
      cerr << "config.dat file converted." << endl;
   
      retval = true;
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

