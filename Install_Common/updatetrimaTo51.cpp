/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrimaTo51.h"

#include "targzextract.c"

using namespace std;

#ifdef __cplusplus
extern "C" { 
#endif

    int cp(const char * from, const char * to);
//int xcopy (const char * src, const char *dest);
//int copyFileContiguous(const char * from, const char * to);
//int unzipFile(const char * from, const char * to);

int softcrc(const char * options);

#ifdef __cplusplus
};
#endif


// Default constructor
updatetrimaTo51 :: updatetrimaTo51() :
updatetrimaBase()
{
}

// Copy constructor - private so nobody can use it
updatetrimaTo51 :: updatetrimaTo51( const updatetrimaTo51 &obj )
{
}

// Default destructor
updatetrimaTo51 ::  ~updatetrimaTo51()
{
}


void updatetrimaTo51 :: updateConfig()
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
    writeFile |= updateConfigVersion(datfile);

    // Note that function calls are done above to avoid the
    // short circuit OR.  We want all four to be evaluated.
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

bool updatetrimaTo51 :: updateConfigVersion(CDatFileReader& datfile)
{
    // Default behavior is to do nothing
    return false;
}

bool updatetrimaTo51 :: updateConfig6Xto51(CDatFileReader& datfile)
{
    bool update = false;
    if (datfile.Find("LANGUAGE_UNIT_CONFIG","key_crit_or_glob") != NULL)
    {
        update = true;
        cerr << "v6.X config.dat file found.  Conversion needed" << endl;
    }

    //////////////////////////////////////////////////////////////////////////////////
    //                 6.X-->5.1 changes
    //////////////////////////////////////////////////////////////////////////////////
    if (update)
    {
        datfile.RemoveLine( "LANGUAGE_UNIT_CONFIG", "key_crit_or_glob" );

        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_show_residual_loss" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_drbc_body_vol" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_plt_mss_split_notif" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_override_pas_bag_vol" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_blood_diversion" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_plasma_rinseback" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_saline_rinseback" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_air_removal" );
        datfile.RemoveLine( "PREDICTION_CONFIG", "key_male_only_plasma" );
        datfile.RemoveLine( "PREDICTION_CONFIG", "key_male_only_plt" );

        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_rbc_mss_volume_1" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_rbc_mss_volume_2" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_rbc_mss_volume_3" );

        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_rbc_mss_1" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_rbc_mss_2" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_rbc_mss_3" );

        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_yield_7" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_volume_7" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_yield_8" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_volume_8" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_yield_9" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_volume_9" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_yield_10" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_volume_10" );

        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_1" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_2" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_3" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_4" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_5" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_6" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_7" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_8" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_9" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_10" );

        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_1" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_2" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_3" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_4" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_5" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_6" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_7" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_8" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_9" );
        datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_plt_mss_10" );

        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_p" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_plasma_p" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_rbc_p" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_blood_type_p" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_time_p" );

        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_q" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_plasma_q" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_rbc_q" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_blood_type_q" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_time_q" );

        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_r" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_plasma_r" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_rbc_r" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_blood_type_r" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_time_r" );

        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_s" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_plasma_s" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_rbc_s" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_blood_type_s" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_time_s" );

        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_t" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_plasma_t" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_rbc_t" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_blood_type_t" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_time_t" );

        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_mss_rbc_on" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_mss_plt_on" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_show_pre_aas_flags" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_rbc_def_bag_vol" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_plt_def_bag_vol" );

        char key[15] = "key_platelet_"; key[14] = 0;
        int value = 0;

        value = datfile.GetInt( "LANGUAGE_UNIT_CONFIG", "key_lang" );
        if (value < 0 || value > 12)
        {
            datfile.SetValue( "LANGUAGE_UNIT_CONFIG", "key_lang", "0" );
            printf("Setting language to english.\n" );
        }

        value = datfile.GetInt( "PROCEDURE_CONFIG", "key_tbv_vol_setting" );
        if (value < 1 || value > 7)
        {
            datfile.SetValue( "PROCEDURE_CONFIG", "key_tbv_vol_setting", "1" );
            printf("Setting key_tbv_vol_setting to '1'.\n" );
        }

        for (key[13] = 'a'; key[13] <= 'o'; ++key[13])
        {
            value = datfile.GetInt( "PRODUCT_DEFINITIONS", key );
            if (value < 0 || value > 6)
            {
                datfile.SetValue( "PRODUCT_DEFINITIONS", key, "0" );
                printf(key );
                printf(" set to product '0'.\n" );
            }
        }

        if (datfile.Find("PROCEDURE_CONFIG", "key_rinseback_protocol") == NULL)
        {
            datfile.AddLine("PROCEDURE_CONFIG", "key_rinseback_protocol", "0");
        }
    }

    return true;
}

void updatetrimaTo51 :: updateCal()
{
// This is overridden in the 5.2 version of updatetrimaEngine
// but it had to be ifdef'd out of 5.2 to prevent compile errors
// due to differences in the 5.1 & 5.2 filenames.h
// (this is ugly but it eliminates another abstraction layer)
#ifndef __TRIMA52__
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

#endif // __TRIMA52__
}

void updatetrimaTo51 :: updateTrap()
{
    // No changes to go from 6X to 5.1
    return;
}

void updatetrimaTo51 :: updateGlobVars() 
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

    bool update = updateGlobVars50to51(datfile);
    if (!update)
    {
        cerr << "Up-to-date globvars file found.  No conversion needed" << endl;
    }
}

bool updatetrimaTo51 :: updateGlobVars50to51(CDatFileReader& datfile)
{

    if ( !datfile.Find( "EXTERNALIP" ) )
    {
        cerr << "pre-v5.1 globvars file found.  Conversion needed" << endl;

        // v5.1--------------
        //  MACHINE=1T000XX
        //  SENPID=000000000000
        //  EXTERNALIP=172.21.127.255
        //  EXTERNALBIP=172.21.255.255

        // v5.0--------------
        //  MACHINE=XX
        //  CENPID=000000000000
        //  SENPID=000000000000
        //  ENPID486I=000000000000
        //  AN2IP=172.21.127.255
        //  AN2BIP=172.21.255.255

        string an2ip   = datfile.GetString("AN2IP");
        string an2bip  = datfile.GetString("AN2BIP");
        datfile.RemoveLine("CENPID");
        datfile.RemoveLine("ENPID486I");
        datfile.RemoveLine("AN2IP");
        datfile.RemoveLine("AN2BIP");
        datfile.AddLine("EXTERNALIP",an2ip.c_str());
        datfile.AddLine("EXTERNALBIP",an2bip.c_str());

        datfile.Write(GLOBVARS_FILE);

        cerr << "globvars file converted." << endl;

        return true;
    }

    cerr << "v5.1 globvars file found.  No conversion needed" << endl;
    return false;
}

bool updatetrimaTo51 :: extractUpdateFiles()
{
    //
    // Extract v5 files, if this is an upgrade from v5 ...
    //
    struct stat archiveDir;
    struct stat archive;
    stat( "/machine/v5_archive", &archiveDir );
    stat( "/machine/v5_archive/config/machine.tar", &archive );

    if ( S_ISDIR( archiveDir.st_mode ) &&
         S_ISREG( archive.st_mode ) )
    {
        printf("Found /machine/v5_archive directory\n" );
        printf(" ... Extracting v5.0 machine files\n" );

        //
        // Extract the update files
        printf("Extracting machine.tar ...\n" );
        if ( tarExtract( "/machine/v5_archive/config/machine.tar", "/machine/tmp" ) == ERROR )
        {
            printf("Extraction of v5.0 machine files failed\n" );
            return false;
        }

        if ( cp( "/machine/v5_archive/config/machine.tar", "/machine/v5_archive/config/old_machine.tar" ) == ERROR ||
             remove( "/machine/v5_archive/config/machine.tar" ) == ERROR )
        {
            printf("Move of machine.tar to old_machine.tar failed\n" );
            return false;
        }

        //
        // Move the machine files to the /config partition ....
        printf("Moving machine files ...\n" );
        if ( xcopy( "/machine/tmp/d/machine","/config" ) == ERROR )
        {
            printf("Moving the v5.0 machine files failed\n" );
            return false;
        }
    }

    //
    // Extract the update files
    printf("Extracting updateTrima ...\n" );
    if ( tarExtract( "/machine/update/updateTrima.taz", "/machine/update" ) == ERROR )
    {
        printf("Extraction of update files failed\n" );
        return false;
    }

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
       printf("RKM:  This is a Python!.  Reports %d\n", isVersalogicPython()); taskDelay(sysClkRateGet()*10);

       if ( copyFileContiguous( UPDATE_PATH "/bootrom.sys",  VXBOOT_PATH "/bootrom.sys" ) == ERROR ||
            copyFileContiguous( UPDATE_PATH "/vxWorks_python", VXBOOT_PATH "/vxWorks"     ) == ERROR )
       {
          fprintf( stderr, "Install of Python OS image failed\n" );
          return false;
       }
    }
    else
    {
       printf("RKM:  This is NOT a Python!.  Reports %d\n", isVersalogicPython()); taskDelay(sysClkRateGet()*10);

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

bool updatetrimaTo51 :: checkCRC()
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
    if ( softcrc("-filelist " FILELISTS_PATH "/trima.files -verify  "    TRIMA_PATH      "/trima.crc") != 0 ||
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

