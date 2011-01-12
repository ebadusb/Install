/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrima5X.h"

//#include "targzextract.c"

using namespace std;

#ifdef __cplusplus
extern "C" { 
#endif

    int cp(const char * from, const char * to);
//int xcopy (const char * src, const char *dest);
//int copyFileContiguous(const char * from, const char * to);
//int unzipFile(const char * from, const char * to);

int softcrc(const char * options);

int tarExtract ( const char *file     /* archive file name */, 
                 const char *location /* location for extraction */ );

#ifdef __cplusplus
};
#endif


// Default constructor
updatetrima5X :: updatetrima5X() :
updatetrimaBase()
{
}

// Copy constructor - private so nobody can use it
updatetrima5X :: updatetrima5X( const updatetrima5X &obj )
{
}

// Default destructor
updatetrima5X ::  ~updatetrima5X()
{
}


void updatetrima5X :: updateConfig(TrimaVersion fromVersion)
{
    //
    // Create the dat file reader to retrieve the configuration data.
    //
    CDatFileReader datfile(PNAME_CONFIGDAT);
    if ( datfile.Error() )
    {
        cerr << "Config file read error : " << datfile.Error() << endl;
        return;
    }

    bool writeFile = false;

    writeFile |= updatePostCount(datfile);
    writeFile |= updateConfigVersion(datfile, fromVersion);

    // Note that function calls are done above to avoid the
    // short circuit OR.  We want all to be evaluated.
    if (writeFile)
    {
        datfile.WriteCfgFile(FILE_CONFIG_DAT);
        cerr << "config.dat file converted." << endl;
    }
    else
    {
        cerr << "Up to date config file found. No conversion needed." << endl;
    }
}

bool updatetrima5X :: updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion)
{
    // Default behavior is to do nothing
    return false;
}

void updatetrima5X :: updateCal()
{
// This is overridden in the 5.2 version of updatetrimaEngine
// but it had to be ifdef'd out of 5.2 to prevent compile errors
// due to differences in the 5.1 & 5.2 filenames.h
// (this is ugly but it eliminates another abstraction layer)
//#ifndef __TRIMA52__
    //
    // Create the dat file reader to retrieve the calibration data.
    //
    CDatFileReader datfile(PNAME_CALDAT);
    if ( datfile.Error() )
    {
        cerr << "Calibration file read error : " << datfile.Error() << endl;
        return;
    }
    const std::string tsOriginal [] = {"screen_horizontal_size", "screen_vertical_size", "tsraw_left_edge", "tsraw_right_edge", 
        "tsraw_top_edge", "tsraw_bottom_edge"};

    const char* tsHeader = "TOUCHSCREEN";
    struct stat fileStat;
    if ( stat((char *)PNAME_TCHSCRNDAT, &fileStat) == OK )
    {  // 6.0 spiral 4 file 
        CDatFileReader tscrnFile(PNAME_TCHSCRNDAT);
        datfile.AddSection("[TOUCHSCREEN]");
        for (int i=0; i<=5; i++) //Keep both loops separate. 
        {
            datfile.AddLine(tsHeader, tsOriginal[i].c_str(), tscrnFile.Find(tsHeader, tsOriginal[i].c_str()));
        }
        datfile.Write(PNAME_CALDAT);
        tscrnFile.Write(PNAME_TCHSCRNDAT);

        if ( remove( PNAME_TCHSCRNDAT ) == ERROR )
        {
            printf("Removal of %s file failed\n", PNAME_TCHSCRNDAT );
        }
        return;
    }

    else if ( !datfile.Find("TOUCHSCREEN","screen_horizontal_size") )
    {
        cerr << "pre-v5.1 cal.dat file found.  Conversion needed" << endl;

        FILE * fp;
        fp = fopen("/config/absf.2", "r");

        char x1Str[8];
        char x2Str[8];
        char y1Str[8];
        char y2Str[8];

        if ( fp )
        {
            char buffer[256];
            char *savePtr=0;
            char *p=0;
            int temp;
            fgets( buffer, 256, fp );
            fclose(fp);

            p = strtok_r(buffer," :x",&savePtr); // 0 x position
            p = strtok_r( NULL ," :x",&savePtr); // 0 y position
            p = strtok_r( NULL ," :x",&savePtr); // max x position
            p = strtok_r( NULL ," :x",&savePtr); // max y position
            p = strtok_r( NULL ," :x",&savePtr); // x1 position
            strcpy(x1Str,p);
            p = strtok_r( NULL ," :x",&savePtr); // x2 position
            strcpy(x2Str,p);
            p = strtok_r( NULL ," :x",&savePtr); // y1 position
            strcpy(y1Str,p);
            p = strtok_r( NULL ," :x",&savePtr); // y2 position
            strcpy(y2Str,p);

            attrib("/config/absf.2","-R");
            if ( remove( "/config/absf.2" ) == ERROR )
            {
                printf("Removal of absf.2 file failed\n" );
            }

        }
        else
        {
            //
            // Default values if no absf.2 file ...
            //
            sprintf( x1Str,"%d",90 );
            sprintf( x2Str,"%d",850 );
            sprintf( y1Str,"%d",130 );
            sprintf( y2Str,"%d",830 );
        }


        datfile.AddComment(" ");
        datfile.AddSection("[TOUCHSCREEN]");
        datfile.AddComment("#");
        datfile.AddComment("# Touch screen calibration values");
        datfile.AddComment("#");
        datfile.AddComment("# These values specify the logical screen size in pixels.");
        datfile.AddComment("#");
        datfile.AddLine("screen_horizontal_size", "640");
        datfile.AddLine("screen_vertical_size", "480");
        datfile.AddComment(" ");
        datfile.AddComment("#");
        datfile.AddComment("# These values correspond to the raw touchscreen readings for the specified");
        datfile.AddComment("# screen edges.  They are used along with the screen size specified above");
        datfile.AddComment("# to convert raw touch screen readings to pixel based coordinates.");
        datfile.AddComment("#");
        datfile.AddLine("tsraw_left_edge", x1Str);
        datfile.AddLine("tsraw_right_edge", x2Str);
        datfile.AddLine("tsraw_top_edge", y1Str);
        datfile.AddLine("tsraw_bottom_edge", y2Str);

        datfile.Write(PNAME_CALDAT);

        cerr << "cal.dat file converted." << endl;

        return;
    }

    cerr << "v5.1 cal.dat file found.  No conversion needed" << endl;

//#endif // __TRIMA52__
}

void updatetrima5X :: updateTrap(TrimaVersion fromVersion)
{
    return;
}

void updatetrima5X :: updateGlobVars() 
{
    //
    // Create the dat file reader to retrieve the global environment vars data.
    //
    CDatFileReader datfile(GLOBVARS_FILE,false,true);
    if ( datfile.Error() )
    {
        cerr << GLOBVARS_FILE << " file read error : " << datfile.Error() << endl;
        return;
    }

}

bool updatetrima5X :: extractUpdateFiles()
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

    /* make the kernel images writeable so they can be overwritten */
    attrib(VXBOOT_PATH "/bootrom.sys", "-R");
    attrib(VXBOOT_PATH "/vxWorks", "-R");

    if ( copyFileContiguous( UPDATE_PATH "/bootrom.sys", VXBOOT_PATH "/bootrom.sys" ) == ERROR ||
         copyFileContiguous( UPDATE_PATH "/vxWorks"    , VXBOOT_PATH "/vxWorks"     ) == ERROR )
    {
        printf("Install of OS image failed\n" );
        return false;
    }

    if ( remove( UPDATE_PATH "/bootrom.sys" ) == ERROR ||
         remove( UPDATE_PATH "/vxWorks"     ) == ERROR ||
         remove( UPDATE_PATH "/vxboot.taz"  ) == ERROR )
    {
        printf("Removal of temporary OS image failed\n" );
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

bool updatetrima5X :: checkCRC()
{

    // Set permissions in config directory
    fileSort(CONFIG_PATH, FILE_SORT_BY_DATE_ASCENDING, update_file_set_rdwrite);

    //
    // Update configuration CRC values
    mkdir(CONFIG_CRC_PATH);

    softcrc("-filelist " FILELISTS_PATH "/caldat.files    -update " CONFIG_CRC_PATH "/caldat.crc");
    softcrc("-filelist " FILELISTS_PATH "/config.files    -update " CONFIG_CRC_PATH "/config.crc");
    softcrc("-filelist " FILELISTS_PATH "/hwdat.files     -update " CONFIG_CRC_PATH "/hwdat.crc");
    softcrc("-filelist " FILELISTS_PATH "/machine.files   -update " CONFIG_CRC_PATH "/machine.crc");
    softcrc("-filelist " FILELISTS_PATH "/rbcdat.files    -update " CONFIG_CRC_PATH "/rbcdat.crc");
    softcrc("-filelist " FILELISTS_PATH "/terrordat.files -update " CONFIG_CRC_PATH "/terrordat.crc");

    // Verify the installation CRC values
    if ( softcrc("-filelist " FILELISTS_PATH "/trima.files -verify  "    TRIMA_PATH      "/trima.crc -caseSensitive") != 0 ||
         softcrc("-filelist " FILELISTS_PATH "/safety.files -verify "    TRIMA_PATH      "/safety.crc") != 0 ||
         softcrc("-filelist " FILELISTS_PATH "/caldat.files -verify "    CONFIG_CRC_PATH "/caldat.crc") != 0 ||
         softcrc("-filelist " FILELISTS_PATH "/config.files -verify "    CONFIG_CRC_PATH "/config.crc") != 0 ||
         softcrc("-filelist " FILELISTS_PATH "/hwdat.files -verify "     CONFIG_CRC_PATH "/hwdat.crc") != 0 ||
         softcrc("-filelist " FILELISTS_PATH "/machine.files -verify "   CONFIG_CRC_PATH "/machine.crc") != 0 ||
         softcrc("-filelist " FILELISTS_PATH "/rbcdat.files -verify "    CONFIG_CRC_PATH "/rbcdat.crc") != 0 ||
         softcrc("-filelist " FILELISTS_PATH "/terrordat.files -verify " CONFIG_CRC_PATH "/terrordat.crc") != 0)
    {
        printf("CRC check of installed software failed\n");
        return false;
    }

    return true;
}

bool updatetrima5X :: checkPlasmaRB()
{
    bool retval = false;
    const char *configData;
    const char *swData;

    // Get the rinseback setting from sw.dat
    swData = findSetting("allow_plasma_rinseback=", PNAME_SWDAT);

    if (swData) cerr << "sw.dat allow_plasma_rinseback = " << swData << endl;

    // If rinseback is turned off in sw.dat make sure it is off in config.dat
    if ( swData != NULL && swData[0] == '0' )
    {
        // Get the rinseback setting from config.dat
        configData = findSetting("key_plasma_rinseback=", PNAME_CONFIGDAT);
//        if (configData) cerr << "config.dat key_plasma_rinseback = " << configData << endl;

        // If it's on, turn it off
        if ( configData != NULL && configData[0] == '1' )
        {
            CDatFileReader datfile(PNAME_CONFIGDAT);
            if ( datfile.Error() )
            {
                cerr << "Can't open config.dat to modify key_plasma_rinseback" << endl;
            }
            else
            {
                // Turn off plasma rinseback
                if ( datfile.SetValue("PROCEDURE_CONFIG", "key_plasma_rinseback", "0") == true)
                {
                    datfile.WriteCfgFile(FILE_CONFIG_DAT);
//                    cerr << "Set key_plasma_rinseback to 0" << endl;
                    retval = true;

                    configData = findSetting("key_plasma_rinseback=", PNAME_CONFIGDAT);
//                    if (configData) cerr << "config.dat key_plasma_rinseback = " << configData << endl;

                }
                else
                {
//                    cerr << "Couldn't set key_plasma_rinseback to 0" << endl;
                    retval = false;
                }
            }
        }
    }
    return retval;
}

