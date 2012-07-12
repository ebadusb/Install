/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrimaBase.h"

#include "crc.c"
#include <fstream>
#include <vector>
#include <map>

#include "targzextract.c"

using namespace std;

#ifdef __cplusplus
extern "C" { 
#endif

    int cp(const char * from, const char * to);
    int copyFileContiguous(const char * from, const char * to);

    int softcrc(const char * options);

#ifdef __cplusplus
};
#endif

extern "C" STATUS xdelete(const char * fileName);


// Default constructor
updatetrimaBase :: updatetrimaBase()
{
}

// Copy constructor
updatetrimaBase :: updatetrimaBase( const updatetrimaBase &obj )
{
}

// Default destructor
updatetrimaBase ::  ~updatetrimaBase()
{
}


int updatetrimaBase :: copyFileContiguous(const char * from, const char * to)
{
    int             fromFD = open(from, O_RDONLY, 0644);

    attrib(to, "-R");
    int             toFD = open(to, O_CREAT|O_RDWR, 0644);

    struct stat fileStat;
    char            buffer[512];
    int             result = ERROR; 

    if ( fromFD >= 0 && toFD >= 0 )
    {
        long    bytesCopied = 0;
        int bytesRead;

        fstat(fromFD, &fileStat);
        printf("copying %s to %s: file length = %ld bytes\n", from, to, fileStat.st_size);

        /*
         * Make destination a contiguous file (required for boot image)
         */
        ftruncate(toFD, 0);
        ioctl(toFD, FIOCONTIG, fileStat.st_size);

        while ( (bytesRead = read(fromFD, buffer, 512)) > 0 )
        {
            bytesCopied += write(toFD, buffer, bytesRead);
        }

        if ( bytesCopied != fileStat.st_size)
        {
            printf("Copy failed (%ld bytes written)\n", bytesCopied);
        }
        else
        {
            result = bytesCopied;
        }
    }
    else
    {
        if ( fromFD < 0 )
        {
            perror(from);
        }

        if ( toFD < 0 )
        {
            perror(to);
        }
    }

    if ( fromFD >= 0 )
    {
        close(fromFD);
    }

    if ( toFD >= 0 )
    {
        close(toFD);
    }

    return result;
}

int updatetrimaBase :: unzipFile(const char * from, const char * to)
{
    gzFile fromFD = gzopen(from, "r");
    int    toFD   = open(to, O_CREAT|O_RDWR, 0644);

    if ( fromFD >= 0 && toFD >= 0 )
    {
        int bytesRead;
        int bytesWritten=0;
        int kBytesWritten=0;
        char  buffer[10*1024];

        while ( (bytesRead = gzread(fromFD, buffer, 10*1024)) > 0 )
        {
            bytesWritten += write(toFD, buffer, bytesRead);
            kBytesWritten = bytesWritten/1024;
        }

        printf("\tUncompression complete\n" );
        gzclose(fromFD);
        close(toFD);

        return 1;
    }
    else
    {
        if ( fromFD < 0 )
        {
            perror(from);
        }

        if ( toFD < 0 )
        {
            perror(to);
        }
    }

    return 0;
}



FileCallBackStatus updatetrimaBase :: update_clean_file(const char * fullPathName)
{
    struct stat fileStat;

    if ( stat((char *)fullPathName, &fileStat) == OK &&
         (fileStat.st_mode & S_IFDIR) != 0 )
    {
        /*
         *  Recurse through sub-directories to make sure all files are removed
         */
        fileSort(fullPathName, FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
    }

    attrib(fullPathName, "-R");
    xdelete(fullPathName);
    return FILE_CALLBACK_CONTINUE;
}

FileCallBackStatus updatetrimaBase :: update_file_set_rdwrite(const char * fullPathName)
{
    struct stat fileStat;

    if ( stat((char *)fullPathName, &fileStat) == OK &&
         (fileStat.st_mode & S_IFDIR) != 0 )
    {
        fileSort(fullPathName, FILE_SORT_BY_DATE_ASCENDING, update_file_set_rdwrite);
    }

    attrib(fullPathName, "-R");
    return FILE_CALLBACK_CONTINUE;
}

FileCallBackStatus updatetrimaBase :: update_file_set_rdonly(const char * fullPathName)
{
    struct stat fileStat;

    if ( stat((char *)fullPathName, &fileStat) == OK &&
         (fileStat.st_mode & S_IFDIR) != 0 )
    {
        fileSort(fullPathName, FILE_SORT_BY_DATE_ASCENDING, update_file_set_rdonly);
        attrib(fullPathName, "-R");
    }
    else
    {
        attrib(fullPathName, "+R");
    }

    return FILE_CALLBACK_CONTINUE;
}

const char * updatetrimaBase :: findSetting(const char * setting, FILE * fp)
{
    char * result = NULL;
    if ( fp )
    {
        char    buffer[256];
        while ( !result && fgets(buffer, 256, fp) )
        {
            char * start = buffer + strspn(buffer, " \t");
            if ( strncmp(start, setting, strlen(setting)) == 0 )
            {
                start += strlen(setting);
                start[strcspn(start, "\n\r")] = '\0';
                result = (char *)malloc(strlen(start)+1);
                strcpy(result, start);
            }
        }
    }
    return result;
}

const char * updatetrimaBase :: findSetting(const char * setting, const char * fileName)
{
    const char * result = NULL;  
    FILE * fp = fopen(fileName, "r");

    if ( fp )
    {
        result = findSetting( setting, fp );
        fclose(fp);
    }

    return result;
}

bool updatetrimaBase :: updatePostCount(CDatFileReader& datfile)
{
    bool bUpdate = false;
    // IT 8741/8737 - Force post count config setting to 100K
    float postCount = datfile.GetFloat("PROCEDURE_CONFIG", "key_post_plat");
    if (postCount < 100000)
    {
        datfile.RemoveLine("PROCEDURE_CONFIG", "key_post_plat");
        datfile.AddLine("PROCEDURE_CONFIG", "key_post_plat", "100000");
        cerr << "Changed donor post count from "<< postCount << " to 100000" << endl;
        bUpdate = true;
    }
    return bUpdate;
}

void updatetrimaBase :: updateRBC()
{
    // Put the rbc.dat file in the correct location.
    attrib(CONFIG_PATH "/" FILE_RBC_DAT, "-R");
    if ( cp( TEMPLATES_PATH "/" FILE_RBC_DAT, CONFIG_PATH "/" FILE_RBC_DAT ) == ERROR )
    {
        printf("copy of rbc.dat failed\n");
        return;
    }
    attrib(CONFIG_PATH "/" FILE_RBC_DAT, "+R");

}

void updatetrimaBase :: updateHW()
{
    // Replace hw.dat if the version number has changed
    currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_HW_DAT);

    if ( isAmpro() )
    {
        newVersion = findSetting("file_version=", TEMPLATES_PATH "/hw_ampro.dat");
    }
    else
    {
        newVersion = findSetting("file_version=", TEMPLATES_PATH "/hw_versalogic.dat");
    }

    if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
    {
        printf("Updating hw.dat to new version %s from existing version %s...\n", newVersion, currVersion);

        attrib(CONFIG_PATH "/" FILE_HW_DAT, "-R");

        if ( isAmpro() )
        {
            if ( cp( TEMPLATES_PATH "/hw_ampro.dat", CONFIG_PATH "/" FILE_HW_DAT ) == ERROR )
            {
                printf("copy of hw_ampro.dat failed\n");
                return;
            }
        }
        else
        {
            if ( cp( TEMPLATES_PATH "/hw_versalogic.dat", CONFIG_PATH "/" FILE_HW_DAT) == ERROR )
            {
                printf("copy of hw_versalogic.dat failed\n");
                return;
            }
        }
        attrib(CONFIG_PATH "/" FILE_HW_DAT, "+R");
        fflush(stdout);
    }

}

void updatetrimaBase :: updateSW()
{
    // Replace sw.dat if the version number has changed
    currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_SW_DAT);
    newVersion = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SW_DAT);

    if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
    {
        printf("Updating sw.dat to new version %s from existing version %s...\n", newVersion, currVersion);
        attrib(CONFIG_PATH "/" FILE_SW_DAT, "-R");

        if ( cp( TEMPLATES_PATH "/" FILE_SW_DAT, CONFIG_PATH "/" FILE_SW_DAT ) == ERROR )
        {
            printf("copy of %s failed\n", FILE_SW_DAT);
            return;
        }

        attrib(CONFIG_PATH "/" FILE_SW_DAT, "+R");
        fflush(stdout);
    }
}

void updatetrimaBase :: updateTerror()
{
    // Replace terror_config.dat if the version number has changed
    currVersion = findSetting("file_version=", TERROR_CONFIG_FILE);
    newVersion = findSetting("file_version=", TEMPLATES_PATH "/terror_config.dat");

    if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
    {
        printf("Updating terror_config.dat to new version %s from existing version %s...\n", newVersion, currVersion);
        attrib(TERROR_CONFIG_FILE, "-R");

        if ( cp( TEMPLATES_PATH "/terror_config.dat", CONFIG_PATH "/terror_config.dat" ) == ERROR )
        {
            printf("copy of terror_config.dat failed\n");
            return;
        }

        attrib(CONFIG_PATH "/terror_config.dat", "+R");
        fflush(stdout);
    }
}

void updatetrimaBase :: updateSounds()
{
    printf("Updating sounds.dat...\n");
    attrib(PNAME_SOUNDSDAT, "-R");

    if ( cp( TEMPLATES_PATH "/" FILE_SOUNDS_DAT, PNAME_SOUNDSDAT ) == ERROR )
    {
        printf("copy of sounds.dat failed\n");
        return;
    }

    attrib(PNAME_SOUNDSDAT, "+R");
    fflush(stderr);
}

void updatetrimaBase :: updateCassette()
{
    // Replace cassette.dat if the version number has changed
    currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_CASSETTE_DAT);
    newVersion = findSetting("file_version=", TEMPLATES_PATH "/" FILE_CASSETTE_DAT);

    if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
    {
        printf("Updating %s to new version %s from existing version %s...\n", FILE_CASSETTE_DAT, newVersion, currVersion);
        attrib(CONFIG_PATH "/" FILE_CASSETTE_DAT, "-R");

        if ( cp( TEMPLATES_PATH "/" FILE_CASSETTE_DAT, CONFIG_PATH "/" FILE_CASSETTE_DAT ) == ERROR )
        {
            printf("copy of %s failed\n", FILE_CASSETTE_DAT);
            return;
        }

        attrib(CONFIG_PATH "/" FILE_CASSETTE_DAT, "+R");
        fflush(stdout);
    }
}

void updatetrimaBase :: updateSetConfig()
{
    // these are the customer selected sets.... dont overwrite if it exists! 
    currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_SETCONFIG_DAT);
    newVersion = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SETCONFIG_DAT);

    if (currVersion == NULL && newVersion != NULL)
    {
        // if the file isnt there....
        printf("Adding %s ...\n", FILE_SETCONFIG_DAT);

        if ( cp( TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT ) == ERROR )
        {
            printf("copy of %s failed\n", FILE_SETCONFIG_DAT);
            return;
        }

        attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
        fflush(stdout);
    }
    else if (currVersion == NULL && newVersion == NULL)
    {
        printf("copy of %s failed, no template file found\n", FILE_SETCONFIG_DAT);
    }
    else
    {
        printf("%s already exists ...\n", FILE_SETCONFIG_DAT);
    }
}

int updatetrimaBase :: verifyCrc(const char* commandLine)
{
    int crcReturnVal = softcrc(commandLine);

    if (crcReturnVal != 0)
    {
        printf("CRC ERROR %d on command line \"%s\"\n", crcReturnVal, commandLine);
    }

    return crcReturnVal;
}


void updatetrimaBase :: copyTrapFiles()
{
    attrib(TRAP_DEFAULTS_FILE, "-R");

    if (cp(TEMPLATES_PATH "/trap_default.dat", TRAP_DEFAULTS_FILE) == ERROR)
    {
        printf("copy of trap_default.dat\n");
        return;
    }
    attrib(TRAP_DEFAULTS_FILE, "+R");

    attrib(TRAP_OVERRIDE_FILE, "-R");

    if (cp(TEMPLATES_PATH "/trap_override.dat", TRAP_OVERRIDE_FILE) == ERROR)
    {
        printf("copy of trap_override.dat\n");
        return;
    }
    attrib(TRAP_OVERRIDE_FILE, "+R");   

}

bool updatetrimaBase :: updateConfig52X510(CDatFileReader& datfile)
{
    cerr << "updateConfig52X510" << endl;

    // check if 5.2(P) by looking for a new parameter.......
    if ( !datfile.Find("PROCEDURE_CONFIG", "key_mss_plt_on") )
    {
        cerr << "v5.1 config.dat file found.  No conversion needed" << endl;
        return false;
    }
    cerr << "Converting from 52X to 510" << endl;

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

    return true;

}

bool updatetrimaBase :: updateConfig510520(CDatFileReader& datfile)
{
    cerr << "updateConfig510520" << endl;

    // check if 5.P by looking for a new parameter.......
    if ( datfile.Find("PROCEDURE_CONFIG", "key_mss_plt_on") )
    {
        cerr << "v5.2 config.dat file found.  No conversion needed" << endl;
        return false;
    }
    cerr << "Converting from 510 to 520" << endl;

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

    return true;
}

bool updatetrimaBase :: updateConfig510517(CDatFileReader& datfile)
{
   bool retval = false;

   cerr << "updateConfig510517" << endl;

   // If this is a pre 5.1.7 config file
   if ( !datfile.Find("PRODUCT_TEMPLATES","key_plt_amap_single_yield_min") )
   {
       cerr << "Converting from 510 to 517" << endl;

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

   // 5.1.7/8 don't support Japanese, set to English
   int value = 0;
   value = datfile.GetInt( "LANGUAGE_UNIT_CONFIG", "key_lang" );
   if (value == 4)
   {
      datfile.SetValue( "LANGUAGE_UNIT_CONFIG", "key_lang", "0" );
      printf("Setting language to english.\n" );
      retval = true;
   }

   return retval;
}

bool updatetrimaBase :: updateConfig517510(CDatFileReader& datfile)
{
    cerr << "updateConfig517510" << endl;

    // If this is a 5.1.7 config file
    if ( datfile.Find("PRODUCT_TEMPLATES","key_plt_amap_single_yield_min") )
    {
        cerr << "Converting from 517 to 510" << endl;

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
		
        int value = datfile.GetInt( "LANGUAGE_UNIT_CONFIG", "key_lang" );
        if (( value < 0 ) || ( value > 9 ))
        {
           datfile.SetValue( "LANGUAGE_UNIT_CONFIG", "key_lang", "0" );
           printf( "Setting language to english.\n" );
        }

        return true;
    }

    return false;
}

bool updatetrimaBase :: updateConfig5X600(CDatFileReader& datfile)
{
    cerr << "updateConfig5X600" << endl;

    // If this is a 6.X config file then return
    if ( datfile.Find("PRODUCT_TEMPLATES", "key_plt_yield_10") )
    {
        cerr << "Converting config.dat from 5X to 6X - not needed" << endl;
        return false;
    }

    cerr << "Converting from 5X to 600" << endl;

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
    datfile.AddLine( "PROCEDURE_CONFIG", "key_drbc_body_vol",        "4.5" );

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

    datfile.AddLine( "PREDICTION_CONFIG",  "key_complementary_plasma", "0" );

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

bool updatetrimaBase :: updateConfig600510(CDatFileReader& datfile)
{
    cerr << "updateConfig600510" << endl;

   if (datfile.Find("LANGUAGE_UNIT_CONFIG","key_crit_or_glob") == NULL)
   {
      cerr << "Converting config.dat from 600 to 510 - not needed" << endl;
      return false;
   }

   cerr << "Converting from 600 to 510" << endl;

   //////////////////////////////////////////////////////////////////////////////////
   //                 6.0-->5.1 changes
   //////////////////////////////////////////////////////////////////////////////////
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
   datfile.RemoveLine( "PROCEDURE_CONFIG", "key_drbc_threshold" );
   datfile.RemoveLine( "PROCEDURE_CONFIG", "key_show_pre_aas_flags" );
   datfile.RemoveLine( "PROCEDURE_CONFIG", "key_rbc_def_bag_vol" );
   datfile.RemoveLine( "PROCEDURE_CONFIG", "key_plt_def_bag_vol" );

   char key[15] = "key_platelet_"; key[14] = 0;
   int value = 0;

   value = datfile.GetInt( "LANGUAGE_UNIT_CONFIG", "key_lang" );
   if (value < 0 || value > 9)
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
       cerr << "Converting adding key_rinseback_protocol" << endl;

      datfile.AddLine("PROCEDURE_CONFIG", "key_rinseback_protocol", "0");
   }

   return true;
}

bool updatetrimaBase :: updateConfig600602(CDatFileReader& datfile)
{
    cerr << "updateConfig600602" << endl;

    if ( !datfile.Find("PRODUCT_TEMPLATES","key_plt_amap_single_yield_min") )
    {
        cerr << "Converting from 600 to 602" << endl;

        // Add AMAPP stuff
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
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_p", "1" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_q", "1" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_r", "1" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_s", "1" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_t", "1" );

        // change these plasma definitions too
        datfile.RemoveLine("PRODUCT_DEFINITIONS", "key_plasma_c");
        datfile.RemoveLine("PRODUCT_DEFINITIONS", "key_plasma_l");
        datfile.RemoveLine("PRODUCT_DEFINITIONS", "key_plasma_m");

        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_c", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_l", "2" );
        datfile.AddLine( "PRODUCT_DEFINITIONS", "key_plasma_m", "0" );


        datfile.AddLine( "PREDICTION_CONFIG",  "key_complementary_plasma", "0" );

        return true;
    }

    return false;
}

bool updatetrimaBase :: updateConfig602600(CDatFileReader& datfile)
{
    cerr << "updateConfig602600" << endl;

    if ( datfile.Find("PRODUCT_TEMPLATES","key_plt_amap_single_yield_min") )
    {
        cerr << "Converting from 602 to 600" << endl;
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
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_p" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_q" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_r" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_s" );
        datfile.RemoveLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_t" );


        datfile.RemoveLine( "PREDICTION_CONFIG",  "key_complementary_plasma" );

        return true;
    }

    return false;
}

bool updatetrimaBase :: updateConfig600610(CDatFileReader& datfile)
{
    cerr << "updateConfig600610" << endl;

    // The only difference is that 6.1.X has the key_show_pre_aas_flags
    if ( !datfile.Find("PROCEDURE_CONFIG", "key_show_pre_aas_flags") )
    {
        cerr << "Converting from 600 to 610" << endl;

        // Operators potentially discard flagged products
        datfile.AddLine( "PROCEDURE_CONFIG", "key_show_pre_aas_flags",   "1" );
        return true;
    }

    return false;
}

bool updatetrimaBase :: updateConfig610600(CDatFileReader& datfile)
{
    cerr << "updateConfig610600" << endl;

    // The only difference is that 6.1.X has the key_show_pre_aas_flags
    if ( datfile.Find("PROCEDURE_CONFIG", "key_show_pre_aas_flags") )
    {
        cerr << "Converting from 610 to 600" << endl;

        // Operators potentially discard flagged products
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_show_pre_aas_flags" );
        return true;
    }

    return false;
}

bool updatetrimaBase :: updateConfig602630(CDatFileReader& datfile)
{
    cerr << "updateConfig602630" << endl;

    // if there isn't a key_plasma_ratio flag it hasn't been updated to 6.3
    if ( !datfile.Find("PROCEDURE_CONFIG", "key_plasma_ratio") )
    {
        cerr << "Converting from 602 to 630" << endl;

        datfile.SetValue( "LANGUAGE_UNIT_CONFIG", "key_height", "0" );
        datfile.SetValue( "LANGUAGE_UNIT_CONFIG", "key_weight", "0" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_proc_time", "100" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_custom_ratio", "10" );

        datfile.AddLine( "PROCEDURE_CONFIG",  "key_plasma_ratio", "13" );

        datfile.SetValue( "PROCEDURE_CONFIG", "key_ac_rate", "1" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_post_crit", "30" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_tbv_vol_setting", "49" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_tbv_percent", "12" );

        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_weight_1_setting" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_weight_2_setting" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_weight_3_setting" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_weight_1_less_than_vol" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_weight_2_less_than_vol" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_weight_3_less_than_vol" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_weight_3_greater_than_vol" );

        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_setting",   "175" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_less_than_vol",   "500.0" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_greater_than_vol",   "600.0" );

        datfile.AddLine( "PROCEDURE_CONFIG", "key_max_plasma_pls",   "600" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_max_plasma_plt",   "400" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_max_plasma_pls_setting",   "1" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_max_plasma_plt_setting",   "1" );

        datfile.SetValue( "PROCEDURE_CONFIG", "key_max_draw_flow", "0" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_air_removal", "0" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_blood_diversion", "1" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_mas_rbc_on", "0" );

        datfile.SetValue( "PREDICTION_CONFIG", "key_pls_amap_minimum", "100" );
        datfile.SetValue( "PREDICTION_CONFIG", "key_inlet_management", "1" );
        datfile.SetValue( "PREDICTION_CONFIG", "key_return_management", "1" );
        datfile.AddLine( "PREDICTION_CONFIG", "key_ffp_volume",   "470" );

        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_1", "2.2" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_1", "220" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_2", "2.3" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_2", "220,3065" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_3", "3.2" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_3", "260" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_4", "3.3" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_4", "260" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_5", "4" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_5", "260" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_6", "4.2" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_6", "260" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_a", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_a", "7" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_a", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_a", "80" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_b", "2" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_b", "7" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_b", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_b", "80" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_c", "3" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_c", "7" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_c", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_c", "80" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_d", "4" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_d", "7" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_d", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_d", "80" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_e", "5" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_e", "7" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_e", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_e", "80" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_f", "6" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_f", "7" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_f", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_f", "80" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_g", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_g", "0" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_h", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_h", "0" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_i", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_i", "0" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_j", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_j", "0" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_k", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_k", "0" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_l", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_l", "0" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_m", "0" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_n", "0" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_o", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_o", "0" );


        return true;
    }

    return false;
}

bool updatetrimaBase :: updateConfig630602(CDatFileReader& datfile)
{
    cerr << "updateConfig630602" << endl;

    // if there is a key_plasma_ratio flag it hasn't been updated to 6.02
    if ( datfile.Find("PROCEDURE_CONFIG", "key_plasma_ratio") )
    {
        cerr << "Converting from 630 to 602" << endl;

        datfile.SetValue( "LANGUAGE_UNIT_CONFIG", "key_height", "1" );
        datfile.SetValue( "LANGUAGE_UNIT_CONFIG", "key_weight", "1" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_proc_time", "150" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_custom_ratio", "11" );

        datfile.RemoveLine( "PROCEDURE_CONFIG",  "key_plasma_ratio" );

        datfile.SetValue( "PROCEDURE_CONFIG", "key_ac_rate", "4" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_post_crit", "32" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_tbv_vol_setting", "1" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_tbv_percent", "15" );

        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_1_setting", "175" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_2_setting", "175" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_3_setting", "175" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_1_less_than_vol", "500.0" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_2_less_than_vol", "500.0" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_3_less_than_vol", "500.0" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_3_greater_than_vol", "600.0" );

        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_setting",   "175" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_less_than_vol",   "500.0" );
        datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_greater_than_vol",   "600.0" );

        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_max_plasma_pls" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_max_plasma_plt" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_max_plasma_pls_setting" );
        datfile.RemoveLine( "PROCEDURE_CONFIG", "key_max_plasma_plt_setting" );

        datfile.SetValue( "PROCEDURE_CONFIG", "key_max_draw_flow", "2" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_air_removal", "1" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_blood_diversion", "0" );
        datfile.SetValue( "PROCEDURE_CONFIG", "key_mas_rbc_on", "1" );

        datfile.SetValue( "PREDICTION_CONFIG", "key_pls_amap_minimum", "150" );
        datfile.SetValue( "PREDICTION_CONFIG", "key_inlet_management", "6" );
        datfile.SetValue( "PREDICTION_CONFIG", "key_return_management", "3" );
        datfile.RemoveLine( "PREDICTION_CONFIG", "key_ffp_volume" );

        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_1", "2.0" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_1", "142.8571" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_2", "3.0" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_2", "214.2857" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_3", "4.0" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_3", "285.7143" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_4", "5.0" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_4", "357.1429" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_5", "6.0" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_5", "428.5714" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_yield_6", "7.0" );
        datfile.SetValue( "PRODUCT_TEMPLATE", "key_plt_volume_6", "500" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_a", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_a", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_a", "3" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_a", "120" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_b", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_b", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_b", "3" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_b", "120" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_c", "5" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_c", "2" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_c", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_c", "120" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_d", "3" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_d", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_d", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_d", "120" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_e", "3" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_e", "0" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_e", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_e", "120" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_f", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_f", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_f", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_time_f", "120" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_g", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_g", "1" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_h", "2" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_h", "1" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_i", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_rbc_i", "1" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_j", "5" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_j", "1" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_k", "5" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_k", "6" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_l", "3" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_l", "2" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_m", "3" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_n", "3" );

        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_platelet_o", "1" );
        datfile.SetValue( "PRODUCT_DEFINITIONS", "key_plasma_o", "8" );

        return true;
    }

    return false;
}

bool updatetrimaBase :: checkPlasmaRB()
{
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////
//  The main line of update
//////////////////////////////////////////////////////////////////////////////////////
int updatetrimaBase :: upgrade(TrimaVersion fromVersion)
{

#ifdef OUTPUTFILE
    freopen( OUTPUTFILE, "w", stdout );
#endif


    update_file_set_rdwrite(CONFIG_PATH);

    //
    // Make /vxboot and /trima partitions writable
    DOS_VOLUME_DESC_ID  pVolDesc;

    char* vxboot = VXBOOT_PATH;
    pVolDesc = dosFsVolDescGet((void*)vxboot, NULL);
    if ( !pVolDesc )
    {
        perror(VXBOOT_PATH);
        return(-1);
    }

    cbioModeSet(pVolDesc->pCbio, O_RDWR);

    char* trima = TRIMA_PATH;  
    pVolDesc = dosFsVolDescGet((void*)trima, NULL);
    if ( !pVolDesc )
    {
        perror(TRIMA_PATH);
        return(-1);
    }

    cbioModeSet(pVolDesc->pCbio, O_RDWR);   

    // extract the update files
    if ( !extractUpdateFiles() )
    {
        return(-1);
    }

    // Update the configuration files ...
    updateConfig(fromVersion);
    updateCal();
    updateGlobVars();
    updateRBC();
    updateHW();
    updateSW();
    updateTerror();
    updateSounds();
    updateCassette();
    updateSetConfig();

    updateTrap(fromVersion);

    checkPlasmaRB();

    // update & check the CRCs
    if ( !checkCRC() )
    {
        return(-1);
    }

    printf("Trima software update complete.\n");

    // Delete the update script so that it doesn't run again on the subsequent boot if the GTS guy
    // is still holding down the buttons.
    printf("Removing update script.\n");
    remove( UPDATE_PATH "/projectrevision" );
    remove( UPDATE_PATH "/updatetrima" );
    remove( UPDATE_PATH "/updatetrima.taz" );

    fflush( stdout );

#ifdef OUTPUTFILE
    freopen("CON", "w", stdout);
#endif

    trimaSysStartDiskSync();

    return(0);
}

