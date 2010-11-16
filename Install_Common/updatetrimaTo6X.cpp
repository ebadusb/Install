/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrimaTo6X.h"

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
updatetrimaTo6X :: updatetrimaTo6X() :
updatetrimaBase()
{
}

// Copy constructor - private so nobody can use it
updatetrimaTo6X :: updatetrimaTo6X( const updatetrimaTo6X &obj )
{
}

// Default destructor
updatetrimaTo6X ::  ~updatetrimaTo6X()
{
}


bool updatetrimaTo6X :: checkPasSettings(CDatFileReader& datfile)
{
    bool returnVal = false;

    for (int prodNum = 0; prodNum < NUM_YIELD_VOLUME; prodNum++)
    {
        // Variable name string
        char varNameStr[64];

        // Find the PAS toggle setting
        sprintf (varNameStr, "key_plt_mss_%d", prodNum+1);

        const char* pasOnStr = datfile.Find("PRODUCT_TEMPLATES", varNameStr);

        // If no PAS toggle setting exists, or the setting is off, then we're presumably safe
        // from the condition we're looking for.
        if (!pasOnStr || !atoi(pasOnStr))
        {
            continue;
        }

        // The setting exists and is on, so see if we violate conditions.
        // Start by looking up percent plasma
        sprintf (varNameStr, "key_plt_pct_carryover_%d", prodNum+1);
        const char* percentPlasmaStr = datfile.Find("PRODUCT_TEMPLATES", varNameStr);
        float percentPlasma = ((percentPlasmaStr)? atof(percentPlasmaStr) : 0.0f);

        // Now find collect vol setting
        char pltVolVarNameStr[64];
        sprintf (pltVolVarNameStr, "key_plt_volume_%d", prodNum+1);
        const char* collectVolStr = datfile.Find("PRODUCT_TEMPLATES", pltVolVarNameStr);
        float collectVol = ((collectVolStr)? atof(collectVolStr) : 0.0f);

        // If one of these settings was 0, something's wierd, so try the next one.
        if (percentPlasma == 0.0f || collectVol == 0.0f)
        {
            printf("Hmm... for PLT%d, percent plasma is %f, and collect vol is %f. Moving on...\n", prodNum+1, percentPlasma, collectVol);
            continue;
        }

        // Figure out the max bag volume.
        float collectVolMax = floor(1200.0f * (percentPlasma / 100.0f));

        if (collectVol > collectVolMax)
        {
            printf("PLT%d has collect vol %f greater than max %f. Adjusting accordingly.\n", prodNum+1, collectVol, collectVolMax);
            datfile.SetFloat( "PRODUCT_TEMPLATES", pltVolVarNameStr, collectVolMax );
            returnVal = true;
        }
    }

    return returnVal;
}


bool updatetrimaTo6X :: checkRasSettings(CDatFileReader& datfile)
{
    bool returnVal = false;
    bool drbcAlertOn = false;
    int drbcThreshold = 275;

    // If the paster RAS setting is off, we don't need to bother with this.
    const char* masterRasOnStr = datfile.Find("PROCEDURE_CONFIG", "key_mss_rbc_on");

    // See if the DRBC alert is on.
    const char* drbcAlertOnStr = datfile.Find("PROCEDURE_CONFIG", "key_drbc_alert");
    if (drbcAlertOnStr && atoi(drbcAlertOnStr))
    {
        drbcAlertOn = true;
    }

    // See if the DRBC alert is on.
    if (drbcAlertOn)
    {
        const char* drbcThreshStr = datfile.Find("PROCEDURE_CONFIG", "key_drbc_threshold");
        if (drbcThreshStr)
        {
            drbcThreshold = atoi(drbcThreshStr);
        }
    }

    for (int prodNum = 0; prodNum < NUM_CRIT_DOSE; prodNum++)
    {
        // Variable name string
        char rasVolVarNameStr[64];
        char rbcDoseVarNameStr[64];

        int dose = 0;
        int rasVol = 0;

        // Find the PAS toggle setting
        sprintf (rasVolVarNameStr, "key_rbc_mss_volume_%d", prodNum+1);
        sprintf (rbcDoseVarNameStr, "key_rbc_dose_%d", prodNum+1);

        const char* rasVolStr = datfile.Find("PRODUCT_TEMPLATES", rasVolVarNameStr);
        const char* rbcDoseStr = datfile.Find("PRODUCT_TEMPLATES", rbcDoseVarNameStr);

        if (rasVolStr)
        {
            rasVol = atoi(rasVolStr);
        }
        else
        {
            continue;
        }

        if (rbcDoseStr)
        {
            dose = atoi(rbcDoseStr);
        }
        else
        {
            continue;
        }

        if (dose >= drbcThreshold && rasVol < 160)
        {
            datfile.SetFloat( "PRODUCT_TEMPLATES", rasVolVarNameStr, 200 );
            returnVal = true;
        }

        if (dose < drbcThreshold && rasVol < 80)
        {
            datfile.SetFloat( "PRODUCT_TEMPLATES", rasVolVarNameStr, 100 );
            returnVal = true;
        }
    }

    return returnVal;
}

void updatetrimaTo6X :: updateConfig()
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
    writeFile |= checkPasSettings(datfile);
    writeFile |= checkRasSettings(datfile);

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

bool updatetrimaTo6X :: updateConfigVersion(CDatFileReader& datfile)
{
    // If this is a 6.X config file then return
    if ( datfile.Find("PRODUCT_TEMPLATES", "key_plt_yield_10") )
    {
        return false;
    }

    // Plasma rinseback
    /// (Note: in 5.1 and 5.2 there was an option called "key_rinseback_setting",
    //  which could be set to 0 (normal) or 1 (plasma).  Now, we have two special
    //  rinseback settings: one for plasma and one for saline.  They're not mutually
    //  exclusive and each has its own button, so "key_rinseback_setting" became
    //  "key_plasma_rinseback", and "key_saline_rinseback" was added.  Both are
    //  0 for off and 1 for on.
    const char* oldRinsebackSetting = datfile.Find("PROCEDURE_CONFIG", "key_rinseback_setting");

    // If plasma rinseback existed and was turned on, turn it on.
    if (oldRinsebackSetting && (atoi(oldRinsebackSetting) == 1))
    {
        datfile.AddLine( "PROCEDURE_CONFIG", "key_plasma_rinseback",  "1" );
    }
    else
    {
        datfile.AddLine( "PROCEDURE_CONFIG", "key_plasma_rinseback",  "0" );
    }

    // Saline Rinseback
    datfile.AddLine( "PROCEDURE_CONFIG", "key_saline_rinseback",     "0" );

    // Split PAS into separate bag form PLT product
    datfile.AddLine( "PROCEDURE_CONFIG", "key_plt_mss_split_notif",  "0" );

    // Show blood diversion graphics
    datfile.AddLine( "PROCEDURE_CONFIG", "key_blood_diversion",      "0" );

    // Master RAS and PAS toggles
    datfile.AddLine( "PROCEDURE_CONFIG", "key_mss_plt_on",           "0" );
    datfile.AddLine( "PROCEDURE_CONFIG", "key_mss_rbc_on",           "0" );

    // DRBC threshold
    datfile.AddLine( "PROCEDURE_CONFIG", "key_drbc_threshold",       "300" );

    // RAS and PAS bag volumes
    datfile.AddLine( "PROCEDURE_CONFIG", "key_plt_def_bag_vol",      "250" );
    datfile.AddLine( "PROCEDURE_CONFIG", "key_rbc_def_bag_vol",      "100" );

    // PAS bag volume pre-metering override allowed
    datfile.AddLine( "PROCEDURE_CONFIG", "key_override_pas_bag_vol", "0" );

    // Toggle-able air evac
    datfile.AddLine( "PROCEDURE_CONFIG", "key_air_removal",            "1" );

    // DRBC TBV limit
    datfile.AddLine( "PROCEDURE_CONFIG", "key_drbc_body_vol",        "3.9" );

    //  PLT product PAS info
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

    // RBC product RAS/PTF info
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_1", "0" ); 
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_volume_1", "80" ); 
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_2", "0" ); 
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_volume_2", "80" ); 
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_3", "0" ); 
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_volume_3", "80" ); 

    // TRALI exclusions
    datfile.AddLine( "PREDICTION_CONFIG", "key_male_only_plt",  "2" );
    datfile.AddLine( "PREDICTION_CONFIG", "key_male_only_plasma",  "2" );

    // Hgb units instead of Hct
    datfile.AddLine( "LANGUAGE_UNIT_CONFIG", "key_crit_or_glob",  "0" );

    // 5 more configurable procedures
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_p",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_p",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_rbc_p",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_blood_type_p",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_time_p",  "120" );

    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_q",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_q",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_rbc_q",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_blood_type_q",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_time_q",  "120" );

    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_r",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_r",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_rbc_r",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_blood_type_r",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_time_r",  "120" );

    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_s",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_s",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_rbc_s",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_blood_type_s",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_time_s",  "120" );

    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_t",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_t",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_rbc_t",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_blood_type_t",  "0" );
    datfile.AddLine( "PRODUCT_DEFINITIONS", "key_time_t",  "120" );

    // Four extra PLT procedures
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_yield_7", "8.0" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_volume_7", "571.4286" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_7", "0" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_7", "50" );

    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_yield_8", "9.0" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_volume_8", "642.8571" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_8", "0" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_8", "50" );

    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_yield_9", "10.0" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_volume_9", "714.2857" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_9", "0" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_9", "50" );

    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_yield_10", "11.0" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_volume_10", "785.7143" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_10", "0" );
    datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_10", "50" );

    datfile.RemoveLine("PROCEDURE_CONFIG", "key_rinseback_protocol");

    return true;
}

void updatetrimaTo6X :: updateCal()
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

    //For first time installs, copy the touchscreen template file over
    struct stat tsFileStat;
    if ( stat ((char *)PNAME_TCHSCRNDAT, &tsFileStat) == ERROR)
    {
        std::string tsTmpl(TEMPLATES_PATH "/" FILE_TCHSCRN_DAT);
        if ( stat(const_cast<char*>(tsTmpl.c_str()), &tsFileStat) == OK )
        {
            if ( cp(tsTmpl.c_str(), PNAME_TCHSCRNDAT ) == ERROR )
            {
                cerr << "copy of " << tsTmpl << " to " << PNAME_TCHSCRNDAT  << " failed" << endl;
            }
            else
            {
                cerr << "copied" << tsTmpl << " to " << PNAME_TCHSCRNDAT  << " successfully" << endl;
            }
        }
    }


    CDatFileReader tscrnFile(PNAME_TCHSCRNDAT);
    if ( tscrnFile.Error() )
    {
        cerr << "Calibration file read error : " << datfile.Error() << endl;
        return;
    }

    const char* tsHeader = "TOUCHSCREEN";
    const std::string tsAF []= {"a","b","c","d","e","f"};
    const std::string tsOriginal [] = {"screen_horizontal_size", "screen_vertical_size", "tsraw_left_edge", "tsraw_right_edge", 
        "tsraw_top_edge", "tsraw_bottom_edge"};

    // Is it an old 6.0 install (5.8)
    if ( datfile.Find(tsHeader, "a") )
    {
        cerr << "v6.0 " << FILE_CAL_DAT << " old 6.0 cal file found.  Conversion needed" << endl;
        //Move all TOUCHSCREEN related data to touch_screen.dat

        //Transfer values from cal.dat to touch_screen.dat
        for (int i=0; i<=5; i++)
        {
            const char* val = datfile.Find(tsHeader, tsAF[i].c_str());
            if (val)
            {
                tscrnFile.SetValue(tsHeader, tsAF[i].c_str(), val);
                datfile.RemoveLine(tsHeader, tsAF[i].c_str());
            }
            else
                std::cout << tsHeader << ":" << tsOriginal[i] << " not found " << std::endl;
        }
        for (int i=0; i<=5; i++) //Keep both loops separate. 
        {
            const char* val = datfile.Find(tsHeader, tsOriginal[i].c_str());
            if (val)
            {
                tscrnFile.SetValue(tsHeader, tsOriginal[i].c_str(), val);
                datfile.RemoveLine(tsHeader, tsOriginal[i].c_str());
            }
            else
                std::cout << tsHeader << ":" << tsOriginal[i] << " not found " << std::endl;
        }
        datfile.RemoveLine(tsHeader);
    }
    else if ( datfile.Find(tsHeader) )
    {
        cerr << "Pre-v6.0 " << FILE_CAL_DAT << " file found.  Conversion needed" << endl;
        //////////////////////////////////////////////////////////////////////////////////
        //                 5.1/P-->6.0 changes
        //////////////////////////////////////////////////////////////////////////////////
        {
            tscrnFile.SetValue( tsHeader, "a", "0.842105263" );
            tscrnFile.SetValue( tsHeader, "b", "0.0" );
            tscrnFile.SetValue( tsHeader, "c", "-75.7894737" );
            tscrnFile.SetValue( tsHeader, "d", "0.0" );
            tscrnFile.SetValue( tsHeader, "e", "0.685714286" );
            tscrnFile.SetValue( tsHeader, "f", "-89.1428571" );
        }

        const char *val = 0;
        for (int i=0; i<=5; i++)
        {
            val = datfile.Find(tsHeader, tsOriginal[i].c_str());
            if ( val )
            {
                tscrnFile.SetValue(tsHeader, tsOriginal[i].c_str(), val);
                datfile.RemoveLine(tsHeader, tsOriginal[i].c_str());
            }
        }

        while ( datfile.Find(tsHeader) )
        {
            datfile.RemoveLine(tsHeader);
        }
    }
    else
    {
        struct stat fileStat;
        if ( stat((char *)PNAME_TCHSCRNDAT, &fileStat) == OK )
        {
            cerr << "File " << FILE_TCHSCRN_DAT << " present with and up to date " << FILE_CAL_DAT << ". No conversion needed" << endl;
        }
        else
        {
            cerr << " ... pre-v5.1 " << FILE_CAL_DAT << " file found.  Unable to Convert!  ending..." << endl;
        }
        return;
    }

    tscrnFile.Write(PNAME_TCHSCRNDAT);
    datfile.Write(PNAME_CALDAT);

    cerr << FILE_CAL_DAT << " file converted." << endl;
}

void updatetrimaTo6X :: updateTrap()
{
    // Since this is a 5.1 to 6.0 upgrade, we must override the trap config files.

    copyTrapFiles();
}

void updatetrimaTo6X :: updateGlobVars() 
{
    // Create the dat file reader to retrieve the global environment vars data.
    CDatFileReader datfile(GLOBVARS_FILE,false,true);
    if ( datfile.Error() )
    {
        cerr << GLOBVARS_FILE << " file read error : " << datfile.Error() << endl;
        return;
    }

    if ( !datfile.Find( "EXTERNALIP" ) )
    {
        cerr << "pre-v5.1 globvars file found Unable to Convert... ending" << endl;
        return;
    }

    cerr << "v5.2 globvars file found.  No conversion needed" << endl;
}

bool updatetrimaTo6X :: extractUpdateFiles()
{
    //
    // Extract the update files
    printf("Extracting updateTrima ...\n");
    if ( tarExtract( UPDATE_PATH "/updateTrima.taz", UPDATE_PATH ) == ERROR )
    {
        printf("Extraction of update files failed\n");
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
        printf("Saving the old OS image...\n");
        attrib(VXBOOT_PATH "/vxWorks.old", "-R");

        if ( cp( VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.old" ) == ERROR )
        {
            printf("Archive of old OS image failed\n");
        }

        // Save off backup vxWorks image in case of massive failure ...
        printf("Saving the backup OS image...\n");
        attrib(VXBOOT_PATH "/vxWorks.bak", "-R");

        if ( cp( VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.bak" ) == ERROR )
        {
            printf("Archive of backup OS image failed\n");
        }
    }

    //
    // Store the new files in the proper position
    printf("Extracting the OS image...\n");

    if ( tarExtract( UPDATE_PATH "/vxboot.taz", UPDATE_PATH ) == ERROR )
    {
        printf("Extraction of OS image failed\n");
        return false;
    }

    if ( attrib(VXBOOT_PATH "/bootrom.sys", "-R") == ERROR )
    {
        perror(VXBOOT_PATH "/bootrom.sys");
    }

    if ( attrib(VXBOOT_PATH "/vxWorks", "-R") == ERROR )
    {
        perror(VXBOOT_PATH "/vxWorks");
    }

    //
    // Copy over the control images depending on the board type.
    //
//    if ( IsVendor( "Ampro" ) )
    if ( isAmpro() )
    {
        printf("Copying Control Ampro bootrom.sys and vxworks to %s\n", VXBOOT_PATH);

        if ( copyFileContiguous( UPDATE_PATH "/bootrom_ampro.sys", VXBOOT_PATH "/bootrom.sys" ) == ERROR ||
             copyFileContiguous( UPDATE_PATH "/vxWorks_ampro"    , VXBOOT_PATH "/vxWorks"     ) == ERROR  )
        {
            printf("Install of OS image failed\n" );
            return false;
        }
    }
    else
    {
        printf("Copying Control Versalogic bootrom.sys and vxworks to %s\n", VXBOOT_PATH);

        if ( copyFileContiguous( UPDATE_PATH "/bootrom_versalogic.sys", VXBOOT_PATH "/bootrom.sys" ) == ERROR ||
             copyFileContiguous( UPDATE_PATH "/vxWorks_versalogic"    , VXBOOT_PATH "/vxWorks"     ) == ERROR  )
        {
            printf("Install of OS image failed\n" );
            return false;
        }
    }

    //
    // Clean up source files provided in the TAR file.
    //
    if ( remove( UPDATE_PATH "/bootrom.sys" ) == ERROR ||
         remove( UPDATE_PATH "/vxWorks"     ) == ERROR ||
         remove( UPDATE_PATH "/bootrom_ampro.sys" ) == ERROR ||
         remove( UPDATE_PATH "/vxWorks_ampro"     ) == ERROR ||
         remove( UPDATE_PATH "/bootrom_versalogic.sys" ) == ERROR || 
         remove( UPDATE_PATH "/vxWorks_versalogic"     ) == ERROR ||
         remove( UPDATE_PATH "/vxboot.taz"  ) == ERROR )
    {
        printf("Removal of temporary OS image failed\n" );
        return false;
    }

    //
    // Remove existing Trima files
    printf("Removing old Trima files...\n");
    fileSort(TRIMA_PATH,      FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
    fileSort(SAVEDATA_PATH,   FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
    fileSort(TOOLS_PATH,      FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
    fileSort(STRING_DIRECTORY,FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
    fileSort(GRAPHICS_PATH,   FILE_SORT_BY_DATE_ASCENDING, update_clean_file);

    // IT 7051.  Remove clinical file if exists.
    remove(CLINICAL_BUILD);
    remove(TEST_BUILD);

    //
    // Uncompress the update file
    printf("Extracting the Trima software files...\n");

    if ( tarExtract( UPDATE_PATH "/trima.taz", TRIMA_PATH ) == ERROR )
    {
        printf("Extraction of the Trima software failed.\n");
        return false;
    }

    if ( remove( UPDATE_PATH "/trima.taz" ) == ERROR )
    {
        printf("Removal of Trima archive image failed\n");
        return false;
    }

    //
    // Uncompress the update file
    printf("Extracting the string.info files...\n");

    if ( tarExtract( UPDATE_PATH "/strings.taz", STRING_DIRECTORY ) == ERROR )
    {
        printf("Extraction of the string.info files failed.\n");
        return false;
    }

    if ( remove( UPDATE_PATH "/strings.taz" ) == ERROR )
    {
        printf("Removal of string archive image failed\n");
        return false;
    }

    //
    // Uncompress the update file
    printf("Extracting the font files...\n");

    if ( tarExtract( UPDATE_PATH "/fonts.taz", DROP_IN_FONTS_DIR ) == ERROR )
    {
        printf("Extraction of the font files failed.\n");
        return false;
    }

    if ( remove( UPDATE_PATH "/fonts.taz" ) == ERROR )
    {
        printf("Removal of font archive image failed\n");
        return false;
    }

    //
    // Uncompress the update file
    printf("Extracting the data files...\n");

    if ( tarExtract( UPDATE_PATH "/data.taz", DATA_DIRECTORY ) == ERROR )
    {
        printf("Extraction of the data files failed.\n");
        return false;
    }

    if ( remove( UPDATE_PATH "/data.taz" ) == ERROR )
    {
        printf("Removal of data archive image failed\n");
        return false;
    }

    //
    // Uncompress the update file
    printf("Extracting the graphics files...\n");

    if ( tarExtract( UPDATE_PATH "/graphics.taz", GRAPHICS_PATH ) == ERROR )
    {
        printf("Extraction of the graphics files failed.\n");
        return false;
    }

    if ( remove( UPDATE_PATH "/graphics.taz" ) == ERROR )
    {
        printf("Removal of graphics archive image failed\n");
        return false;
    }

    //
    // Copy over the safety images depending on the board type.
    //
//    if ( IsVendor( "Ampro" ) )
    if ( isAmpro() )
    {
        printf("Copying Safety Ampro bootrom.sys and vxworks to %s\n", SAFETY_BOOT_PATH);

        if ( cp( SAFETY_BOOT_PATH "/bootrom_ampro.sys", SAFETY_BOOTROM_IMAGE )    == ERROR ||
             cp( SAFETY_BOOT_PATH "/vxWorks_ampro"    , SAFETY_VXWORKS_IMAGE )    == ERROR )
        {
            printf("Install of OS image failed\n" );
            return false;
        }
    }
    else
    {
        printf("Copying Safety Versalogic bootrom.sys and vxworks to %s\n", SAFETY_BOOT_PATH);

        if ( cp( SAFETY_BOOT_PATH "/vxWorks_versalogic"      , SAFETY_VXWORKS_IMAGE )     == ERROR ||
             cp( SAFETY_BOOT_PATH "/bootrom_versa_bootp.sys" , SAFETY_BOOTROM_IMAGE )     == ERROR ||
             cp( SAFETY_BOOT_PATH "/vxWorks_versalogic_pxe"  , SAFETY_VXWORKS_PXE_IMAGE ) == ERROR ||
             cp( SAFETY_BOOT_PATH "/bootrom_versa_pxe.sys"   , SAFETY_BOOTROM_PXE_IMAGE ) == ERROR )
        {
            printf("Install of OS image failed\n" );
            return false;
        }
    }

    if ( remove( SAFETY_BOOT_PATH "/bootrom_ampro.sys" ) == ERROR ||
         remove( SAFETY_BOOT_PATH "/bootrom_versa_bootp.sys"  )               == ERROR ||
         remove( SAFETY_BOOT_PATH "/bootrom_versa_pxe.sys"    )               == ERROR ||
         remove( SAFETY_BOOT_PATH "/vxWorks_ampro"     ) == ERROR ||
         remove( SAFETY_BOOT_PATH "/vxWorks_versalogic"     ) == ERROR ||
         remove( SAFETY_BOOT_PATH "/vxWorks_versalogic_pxe" ) == ERROR )
    {
        printf("Removal of temporary OS image failed\n" );
        return false;
    }

    //Uncompress the optional tools archive if it exists
    struct stat fileStat;

    if ( stat((char *)UPDATE_PATH "/engr_tools.taz", &fileStat) == OK )
    {
        printf("Extracting the engr tools files...\n");

        if (tarExtract(UPDATE_PATH "/engr_tools.taz", ROOT "/machine/tools") == ERROR)
        {
            printf("Extraction of the Tools files failed.\n");
        }

        if (remove(UPDATE_PATH "/engr_tools.taz") == ERROR )
        {
            printf("Removal of Tools archive image failed\n");
        }
    }

    return true;
}

bool updatetrimaTo6X :: checkCRC()
{
    //
    // Update configuration CRC values
    mkdir(CONFIG_CRC_PATH);

    softcrc("-filelist " FILELISTS_PATH "/caldat.files    -update " CONFIG_CRC_PATH  "/caldat.crc"       );
    softcrc("-filelist " FILELISTS_PATH "/config.files    -update " CONFIG_CRC_PATH  "/config.crc"       );
    softcrc("-filelist " FILELISTS_PATH "/hwdat.files     -update " CONFIG_CRC_PATH  "/hwdat.crc"        );
    softcrc("-filelist " FILELISTS_PATH "/rbcdat.files    -update " CONFIG_CRC_PATH  "/rbcdat.crc"       );
    softcrc("-filelist " FILELISTS_PATH "/terrordat.files -update " CONFIG_CRC_PATH  "/terrordat.crc"    );
    softcrc("-filelist " FILELISTS_PATH "/cassette.files  -update " CONFIG_CRC_PATH  "/cassette.crc"     );
    softcrc("-filelist " FILELISTS_PATH "/setconfig.files -update " CONFIG_CRC_PATH  "/setconfig.crc"    );
    softcrc("-filelist " FILELISTS_PATH "/graphics.files	-update "   PNAME_GUI_GRAPHICS_CRC              );
    softcrc("-filelist " FILELISTS_PATH "/strings.files		-update "   PNAME_STRING_CRC               );
    softcrc("-filelist " FILELISTS_PATH "/fonts.files		-update "   PNAME_FONT_CRC                 );
    softcrc("-filelist " FILELISTS_PATH "/data.files		-update "   PNAME_DATA_CRC                 );
    softcrc("-filelist " FILELISTS_PATH "/safety.files		-update "   TRIMA_PATH      "/safety.crc"       );
    softcrc("-filelist " FILELISTS_PATH "/trima.files     -update " TRIMA_PATH       "/trima.crc"        );
    softcrc("-filelist " FILELISTS_PATH "/machine.files		-update "   CONFIG_CRC_PATH "/machine.crc"      );

    // Set permissions in config directory
    update_file_set_rdonly(CONFIG_PATH);

    // Verify the installation CRC values
    if (verifyCrc("-filelist " FILELISTS_PATH "/caldat.files	-verify "   CONFIG_CRC_PATH "/caldat.crc"   ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/config.files	-verify "   CONFIG_CRC_PATH "/config.crc"   ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/hwdat.files		-verify "   CONFIG_CRC_PATH "/hwdat.crc"    ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/rbcdat.files	-verify "   CONFIG_CRC_PATH "/rbcdat.crc"   ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/terrordat.files	-verify "   CONFIG_CRC_PATH "/terrordat.crc") ||
        verifyCrc("-filelist " FILELISTS_PATH "/cassette.files	-verify "   CONFIG_CRC_PATH "/cassette.crc" ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/setconfig.files	-verify "   CONFIG_CRC_PATH "/setconfig.crc") ||
        verifyCrc("-filelist " FILELISTS_PATH "/graphics.files	-verify "   PNAME_GUI_GRAPHICS_CRC          ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/strings.files   -verify "   PNAME_STRING_CRC           ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/fonts.files     -verify "   PNAME_FONT_CRC             ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/data.files      -verify "   PNAME_DATA_CRC             ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/safety.files	-verify "   TRIMA_PATH      "/safety.crc"   ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/trima.files		-verify "   TRIMA_PATH      "/trima.crc"    ) ||
        verifyCrc("-filelist " FILELISTS_PATH "/machine.files	-verify "   CONFIG_CRC_PATH "/machine.crc"  ))
    {
        return false;
    }

    return true;
}


