/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "updatetrimaBase.h"

#include "crc.c"
#include <fstream>

//
// The following strings are used to identify ID/copyright
// strings within ROM data
//
static const char * romIDString[] =
{
    "copyright",
    "Copyright",
    "COPYRIGHT",
    "BIOS",
    NULL        // must be last entry
};

// The number of Trima versions
static const int numberOfVersions = 5;

// The array of version names
static const char * TrimaVersionName[] = {"5.1.0", "5.1.7", "5.2", "6.0", "6.1"};

// The array of allowed upgrade paths
static const bool allowedUpgrade[][numberOfVersions] = 
{
    // V51   V517  V52   V60   V61
    {true, true, true, true, true},     // From 5.1.0
    {true, true, false, false, false},  // From 5.1.7
    {true, false, true, true, false},   // From 5.2
    {true, false, false, true, false},  // From 6.0
    {true, false, false, false, true}   // From 6.1
};

using namespace std;

#ifdef __cplusplus
extern "C" { 
#endif

    int cp(const char * from, const char * to);
//int xcopy (const char * src, const char *dest);
    int copyFileContiguous(const char * from, const char * to);
//int unzipFile(const char * from, const char * to);

    int softcrc(const char * options);

#ifdef __cplusplus
};
#endif

extern "C" STATUS xdelete(const char * fileName);


// Default constructor
updatetrimaBase :: updatetrimaBase() :
currVersion(NULL),
newVersion(NULL),
curTrimaVersion(V51),
fromTrimaVersion(V51),
toTrimaVersion(V51)
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

/*
const char * updatetrimaBase :: FindIDString(
                                            const char * memPtr,          // start of memory block
                                            unsigned int memLength,       // length of memory block (in bytes)
                                            const char * pattern,         // pattern to search for
                                            unsigned int & memStart,      // starting byte within memory block for search
                                            unsigned int & stringLength   // length of found string
                                            )
{
    int patternLength = strlen(pattern);
    const char * resultString = NULL;

    while ( memStart < memLength-patternLength &&
            !resultString )
    {
        if ( memPtr[memStart] == pattern[0] &&
             memcmp(&memPtr[memStart], pattern, patternLength) == 0 )
        {
            resultString = &memPtr[memStart];
        }
        else
        {
            memStart += 1;
        }
    }

    if ( resultString )
    {
        //
        // Found the specified pattern.  Extend the string to get the printable
        // characters surrounding it.
        //
        unsigned int   stringStart = memStart;
        while ( stringStart > 0 &&
                isprint(memPtr[stringStart-1]) &&
                (memPtr[stringStart-1] & 0x80) == 0 )
        {
            stringStart -= 1;
        }

        stringLength = patternLength + (memStart-stringStart);
        while ( stringStart+stringLength < memLength &&
                stringLength < MaxIDStringLength &&
                isprint(memPtr[stringStart+stringLength]) &&
                (memPtr[stringStart+stringLength] & 0x80) == 0 )
        {
            stringLength += 1;
        }

        resultString = &memPtr[stringStart];
        memStart = stringStart+stringLength;

        if ( stringStart+stringLength < memLength &&
             resultString[stringLength] != '\0' &&
             resultString[stringLength] != '\r' &&
             resultString[stringLength] != '\n' )
        {
            //
            // Reject strings that do not end at the end of the ROM, or do not
            // end with \0, \n, or \r.
            //
            resultString = NULL;
        }
        else
        {
            //
            // Strip trailing spaces from string
            //
            while ( resultString[stringLength] == ' ' && stringLength > 0 )
            {
                stringLength -= 1;
            }
        }
    }

    return resultString;
}
*/

bool updatetrimaBase :: readProjectrevision()
{
    bool retval = true;
    string projRevText;
    char decimalPoint;
    int curMajorRev;
    int curMinorRev;
    int curBuild;

    ifstream fin;
    fin.open(TRIMA_PROJECT_REVISION_FILE);
    if ( fin.is_open() )
    {
        fin >> projRevText;

        cerr << "projRevText: " << projRevText << endl;

        if ( projRevText == "$ProjectRevision:" )
        {
            fin >> curMajorRev;

            cerr << "curMajorRev: " << curMajorRev << endl;

            if ( curMajorRev > 7 )
            {
                fin >> decimalPoint;

                fin >> curMinorRev;
                cerr << "curMinorRev: " << curMinorRev << endl;

            }

            fin >> decimalPoint;
            fin >> curBuild;

            cerr << "curBuild: " << curBuild << endl;

        }
        else
        {
            cerr << "Projectrevision file formatted incorrectly." << endl;
            retval = false;
        }
    }
    else
    {
        cerr << "Projectrevision file could not be opened." << endl;
        retval = false;
    }

    fin.close();


    switch ( curMajorRev )
    {
    case 9:
        curTrimaVersion = V61;
        break;
    case 8:
        curTrimaVersion = V60;
        break;
    case 7:
        curTrimaVersion = V52;
        break;
    case 6:
        if ( curBuild >= 383 )
        {
            curTrimaVersion = V517;
        }
        else
        {
            curTrimaVersion = V51;
        }
        break;
    default:
        cerr << "Can't determine version of software, assuming 5.1.0" << endl;
    }

    cout << "curTrimaVersion: " << curTrimaVersion << endl;
    cout << "fromTrimaVersion: " << fromTrimaVersion << endl;
    cout << "toTrimaVersion: " << toTrimaVersion << endl;

    return retval;
}

bool updatetrimaBase :: allowedUpgradePath()
{
    bool retval = true;

    if ( curTrimaVersion != fromTrimaVersion )
    {
        cerr << "The current software version on the Trima (" << TrimaVersionName[curTrimaVersion] << ") does not match this upgrade script." << endl;
        cerr << "This upgrade script is for " << TrimaVersionName[fromTrimaVersion] << "  Use the correct upgrade script." << endl;
        retval = false;
    }

    if( allowedUpgrade[curTrimaVersion][toTrimaVersion] )
    {
        cerr << "The upgrade from " << TrimaVersionName[curTrimaVersion] << " to " << TrimaVersionName[toTrimaVersion] << " is allowed." << endl;
    }
    else
    {
        cerr << "The upgrade from " << TrimaVersionName[curTrimaVersion] << " to " << TrimaVersionName[toTrimaVersion] << " is not allowed." << endl;
        retval = false;
    }

    return retval;
}

/*
bool updatetrimaBase :: IsVendor( const char * vendor )
{
    const unsigned int BIOSAddress = 0xf0000;
    const unsigned int BIOSLength = 0x10000;
    char logData[MaxIDStringLength];

    const char * romPtr = (const char *)(BIOSAddress);
    int  stringIdx = 0;
    while ( romIDString[stringIdx] )
    {
        unsigned int  searchStartIdx = 0;
        unsigned int  stringLength; 
        const char *  stringInfo = FindIDString(romPtr, BIOSLength, romIDString[stringIdx], searchStartIdx, stringLength);

        while ( stringInfo )
        {
            if ( stringLength < MaxIDStringLength )
            {
                if ( strstr(stringInfo, vendor ) != NULL )
                {
                    return true;
                }
            }

            stringInfo = FindIDString(romPtr, BIOSLength, romIDString[stringIdx], searchStartIdx, stringLength);
        }

        stringIdx += 1;
    }
    return false;
}
*/

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

//    if ( IsVendor( "Ampro" ) )
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
//        if ( IsVendor( "Ampro" ) )
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
    else if (currVersion != NULL && newVersion != NULL && strcmp(newVersion, currVersion) < 0 )
    {
        // Override the file
        printf("Overriding %s ...\n", FILE_SETCONFIG_DAT);
        attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "-R");

        if ( cp( TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT ) == ERROR )
        {
            printf("copy of %s failed\n", FILE_SETCONFIG_DAT);
            return;
        }

        attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
        fflush(stdout);
    }
    else if (currVersion != NULL && newVersion == NULL)
    {
        // Remove the file
        printf("Removing %s ...\n", FILE_SETCONFIG_DAT);
        attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "-R");
        remove(CONFIG_PATH "/" FILE_SETCONFIG_DAT);
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


bool updatetrimaBase :: updateConfig52to51(CDatFileReader& datfile)
{
    // check if 5.P by looking for a new parameter.......
    if ( !datfile.Find("PROCEDURE_CONFIG", "key_mss_plt_on") )
    {
        cerr << "v5.1 config.dat file found.  No conversion needed" << endl;
        return false;
    }
    cerr << "v5.2 config.dat file found.  Conversion needed" << endl;

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

    return true;
}

// Define the following variable if you want your output
// to go to a file.
//#define OUTPUTFILE "/machine/log/updateLog.slick"

//////////////////////////////////////////////////////////////////////////////////////
//  The main line of update
//////////////////////////////////////////////////////////////////////////////////////
int updatetrimaBase :: run()
{

#ifdef OUTPUTFILE
    freopen( OUTPUTFILE, "w", stdout );
#endif

#ifdef JHTEMP

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
    updateConfig();
    updateCal();
    updateGlobVars();
    updateRBC();
    updateHW();
    updateSW();
    updateTerror();
    updateSounds();
    updateCassette();
    updateSetConfig();

    updateTrap();

    // update & check the CRCs
    if ( !checkCRC() )
    {
        return(-1);
    }

    printf("Trima software update complete.\n");

    // Delete the update script so that it doesn't run again on the subsequent boot if the GTS guy
    // is still holding down the buttons.
    printf("Removing update script.\n");
    remove( UPDATE_PATH "/updatetrima" );
    remove( UPDATE_PATH "/updatetrima.taz" );

    fflush( stdout );

#ifdef OUTPUTFILE
    freopen("CON", "w", stdout);
#endif

#endif //JHTEMP
    readProjectrevision();

    if ( !allowedUpgradePath() )
    {
        cerr << "Aborting upgrade." << endl;
    }

    if ( isAmpro() )
    {
        cerr << "This is an Ampro Trima." << endl;
    }

    if ( isVersalogic() )
    {
        cerr << "This is a Versalogic Trima." << endl;
    }

    if ( isVersalogicPython() )
    {
        cerr << "This is a Versalogic Python Trima." << endl;
    }



    trimaSysStartDiskSync();

    return(0);
}

