/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
//#include "updatetrimaBase.h"
#include "updatetrima510.h"
#include "updatetrima514.h"
#include "updatetrima517.h"
#include "updatetrima518.h"
#include "updatetrima520.h"
#include "updatetrima600.h"
#include "updatetrima610.h"
#include <map>
#include <fstream>

using namespace std;

// Map of versions to their objects
static map< TrimaVersion, updatetrimaBase * > versionMap; 

// Map of printable version strings
static map< TrimaVersion, string > versionStringMap; 

// Version upgrading from
TrimaVersion fromTrimaVersion;

// Version upgrading to
TrimaVersion toTrimaVersion;

const char *currVersion;
const char *newVersion;

// The matrix of allowed upgrade paths
// Yeah, it's a 2 dimensional array, deal with it
static const bool allowedUpgrade[][NUMBER_OF_VERSIONS] = 
{
    //            To:
    //V510  V512   V513   V514   V515   V516   V517   V518   V520   V521   V522   V600   V601   V610     // From:
    {true,  true,  true,  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true},   // 5.1.0
    {true,  true,  true,  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true},   // 5.1.2
    {true,  true,  true,  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true},   // 5.1.3
    {false, false, false, true,  false, false, false, false, false, false, false, false, false, false},  // 5.1.4
    {false, false, false, false, true,  false, false, false, false, false, false, false, false, true},   // 5.1.5
    {false, false, false, false, false, true,  false, true,  false, false, false, true,  true,  false},  // 5.1.6
    {true,  true,  true,  false, false, false, true,  true,  false, false, false, false, false, false},  // 5.1.7
    {false, false, false, false, false, false, true,  true,  false, false, false, true,  true,  true},   // 5.1.8
    {false, false, false, false, false, false, false, true,  true,  true,  true,  true,  true,  false},  // 5.2.0
    {false, false, false, false, false, false, false, true,  true,  true,  true,  true,  true,  false},  // 5.2.1
    {false, false, false, false, false, false, false, true,  true,  true,  true,  true,  true,  false},  // 5.2.2
    {false, false, false, false, false, true,  false, true,  true,  true,  true,  true,  true,  false},  // 6.0.0
    {false, false, false, false, false, true,  false, true,  true,  true,  true,  true,  true,  false},  // 6.0.1
    {false, false, false, false, true,  false, false, true,  false, false, false, false, false, true}    // 6.1.0
};

// The array of allowed upgrade paths for Python
static const bool allowedPythonUpgrade[] = 
    //            To:
    //V510  V512   V513   V514   V515   V516   V517   V518  V520   V521   V523   V600  V601  V610
    {false, false, false, false, false, false, false, true, false, false, false, true, true, true};   // From anywhere

#ifdef __cplusplus
extern "C" { 
#endif

int updateTrima();

int tarExtract ( const char *file     /* archive file name */, 
                 const char *location /* location for extraction */ );

#ifdef __cplusplus
};
#endif

bool init()
{
    // default the from & to versions
    fromTrimaVersion = V510;
    toTrimaVersion = V510;

    // Load the version object map
    updatetrimaBase *tmpObjPtr = NULL;

    if ( (tmpObjPtr = new updatetrima510) != NULL )
    {
        versionMap[V510] = tmpObjPtr;
        versionMap[V512] = tmpObjPtr;
        versionMap[V513] = tmpObjPtr;
    }
    else
    {
        return false;
    }

    if ( (tmpObjPtr = new updatetrima514) != NULL )
    {
        versionMap[V514] = tmpObjPtr;
        versionMap[V515] = tmpObjPtr;
        versionMap[V516] = tmpObjPtr;
    }
    else
    {
        return false;
    }

    if ( (tmpObjPtr = new updatetrima517) != NULL )
    {
        versionMap[V517] = tmpObjPtr;
    }
    else
    {
        return false;
    }

    if ( (tmpObjPtr = new updatetrima518) != NULL )
    {
        versionMap[V518] = tmpObjPtr;
    }
    else
    {
        return false;
    }

    if ( (tmpObjPtr = new updatetrima520) != NULL )
    {
        versionMap[V520] = tmpObjPtr;
        versionMap[V521] = tmpObjPtr;
        versionMap[V522] = tmpObjPtr;
    }
    else
    {
        return false;
    }

    if ( (tmpObjPtr = new updatetrima600) != NULL )
    {
        versionMap[V600] = tmpObjPtr;
        versionMap[V601] = tmpObjPtr;
    }
    else
    {
        return false;
    }

    if ( (tmpObjPtr = new updatetrima610) != NULL )
    {
        versionMap[V610] = tmpObjPtr;
    }
    else
    {
        return false;
    }

    // Initialize the printable version strings
    versionStringMap[V510] = "5.1.0";
    versionStringMap[V512] = "5.1.2";
    versionStringMap[V513] = "5.1.3";
    versionStringMap[V514] = "5.1.4";
    versionStringMap[V515] = "5.1.5";
    versionStringMap[V516] = "5.1.6";
    versionStringMap[V517] = "5.1.7";
    versionStringMap[V518] = "5.1.8";
    versionStringMap[V520] = "5.2.0";
    versionStringMap[V521] = "5.2.1";
    versionStringMap[V522] = "5.2.2";
    versionStringMap[V600] = "6.0.0";
    versionStringMap[V601] = "6.0.1";
    versionStringMap[V610] = "6.1.0";

    return true;
}

bool findTazRevision(const char * searchFileName, char *tazRevString)
{
    char *projrevString = "$ProjectRevision: ";
    int projrevStringLen = strlen(projrevString);
    int bytesRead = 0;
    int totBytesRead = 0;
    int overlap = 50;  // amount of character overlap so we don't miss anything during the search
    int baselen = 10*1024;  // base size of the read buffer
    char tmpBuffer[baselen + overlap];
    char tmpChar;
    char revBuffer[100];
    int ctr = 0;
    int i;
    bool done = false;

//    cerr << "updatetrimaBase :: findTazRevision" << endl;

    // Open the file for uncompressed reading
    gzFile fromFD = gzopen(searchFileName, "r");

    if ( fromFD >= 0 )
    {
//        cerr << searchFileName << " open" << endl;

        // Read the initial buffer-full, including the overlap area
        bytesRead = gzread(fromFD, tmpBuffer, sizeof(tmpBuffer));

        bytesRead -= overlap;
        totBytesRead = bytesRead;

        do
        {
            // Loop through the entire read buffer looking for the find string
            for ( i = 0; i < sizeof(tmpBuffer); i++)
            {
                if ( tmpBuffer[i] == projrevString[ctr] )
                {
                    // Did we find all of the letters in the string?
                    if ( ctr == projrevStringLen - 1 )
                    {
//                        cerr << "Found projrevString" << endl;

                        // Go to next char in the list
                        i++;

//                        cerr << "Copying string; tmpBuffer[i] = " << tmpBuffer[i];

                        // Make sure the first char is a number
                        if ( tmpBuffer[i] >= '0' && tmpBuffer[i] <= '9' )
                        {
                            // Found the string, now copy the version #
                            for ( int j = 0; j < sizeof(revBuffer) && j < sizeof(tmpBuffer)-i && !done; j++ )
                            {
//                                cerr << "." ;
    
                                // Copy until we hit the ending $
                                if ( tmpBuffer[i+j] != '$' )
                                {
                                    revBuffer[j] = tmpBuffer[i+j];
                                }
                                else
                                {
                                    revBuffer[j] = 0;
                                    done = true;

//                                    cerr << " Done reading: i = " << i << " j = " << j << "tmpBuffer[i+j] = " << tmpBuffer[i+j] ;
                                }
                            }
                        }

//                        cerr << endl;
                    }
                    else
                    {
                        ++ctr;
                    }

                }
                else
                {
                    ctr = 0;
                }
            }

            // Reset search buffer, copy the overlap amount to the beginning of the read buffer
            for ( i = 0; i < overlap; i++ )
            {
                tmpBuffer[i] = tmpBuffer[baselen + i];
            }

        }
        // Read the next buffer-full but only the base amount and start writing after the overlap amount
        while( !done && (bytesRead = gzread(fromFD, tmpBuffer+overlap, baselen) > 0) );
    }

//    cerr << "revBuffer: " << revBuffer << endl;
//    cerr << "About to copy rev string" << endl;

    if ( done )
    {
//        cerr << "Copying" << endl;
        strcpy(tazRevString, revBuffer);
        return 1;
    }

    return 0;
}

bool readProjectRevisionFile(char *revString)
{
    char tmpNum[50];

    // File is in the format: "$ProjectRevision: <majorRev>.[<minorRev>.]<build> $"
    ifstream fin;
    fin.open(TRIMA_PROJECT_REVISION_FILE);
    if ( fin.is_open() )
    {
        // Read until the space
        fin.getline(tmpNum, sizeof(tmpNum)-1, ' ');
        string tmpNumString(tmpNum);
        if ( tmpNumString == "$ProjectRevision:" )
        {
            // Read until the closing $
            fin.getline(tmpNum, sizeof(tmpNum)-1, '$');
            strcpy(revString, tmpNum);
        }
        else
        {
            // File formatted wrong
            return false;
        }
    }
    else
    {
        // Can't open the file
        return false;
    }

    return true;
}

bool parseRevision(const char *revString, TrimaVersion &parsedVersion)
{
    bool retval = true;
    int curMajorRev = 0;
    int curMinorRev = 0;
    int curBuild = 0;
    char *tokPtr = NULL;
    char tmpNum[50];

    // Make a copy we can modify
    strcpy(tmpNum, revString);

    // Read the majorRev
    if ( (tokPtr = strtok(tmpNum, ".")) != NULL )
    {
        curMajorRev = atoi(tokPtr);
    }

    // Read the minorRev
    if ( (tokPtr = strtok(NULL, ". ")) != NULL )
    {
        curMinorRev = atoi(tokPtr);
    }

    // Read the build
    if ( (tokPtr = strtok(NULL, ". ")) != NULL )
    {
        curBuild = atoi(tokPtr);
    }
    else
    {
        // If there's no build then the minorRev was the build (the format was <majorRev>.<build>)
        curBuild = curMinorRev;
        curMinorRev = 0;
    }

//    cerr << "Build of software on Trima: " << curMajorRev << "." << curMinorRev << "." << curBuild << endl;


    // Figure out what version of the software based on the revision & build info
    switch ( curMajorRev )
    {
    case 9:
        parsedVersion = V610;
        break;
    case 8:
        if ( curMinorRev == 4 && curBuild >= 5 )
        {
            parsedVersion = V601;
        }
        else
        {
            parsedVersion = V600;
        }
        break;
    case 7:
        switch ( curBuild )
        {
        case 308:
            parsedVersion = V521;
            break;
        case 310:
            parsedVersion = V522;
            break;
        default:
            parsedVersion = V520;
            break;
        }
        break;
    case 6:
        if ( curBuild == 301 )
        {
            parsedVersion = V510;
        }
        else if ( curBuild == 328 )
        {
            parsedVersion = V512;
        }
        else if ( curBuild == 343)
        {
            parsedVersion = V513;
        }
        else if ( curBuild == 363 )
        {
            parsedVersion = V514;
        }
        else if ( curBuild == 365 || ( curMinorRev == 1 && curBuild == 3) )
        {
            parsedVersion = V515;
        }
        else if ( curBuild == 381 )
        {
            parsedVersion = V516;
        }
        else if ( curBuild == 511 )
        {
            parsedVersion = V517;
        }
        else if ( curBuild > 513 )  // This is a guess and needs to be updated when 5.1.8 is released
        {
            parsedVersion = V518;
        }
        break;
    default:
        cerr << "Can't determine version of Trima software, aborting update." << endl;
        retval = false;
    }

    return retval;
}

bool allowedUpgradePath()
{
    bool retval = true;

	FILE *development_only = fopen( "/machine/update/development_only", "r" );

    // Check if this upgrade is allowed, bypass this check if the development_only file exists
    if( allowedUpgrade[fromTrimaVersion][toTrimaVersion] || development_only )
    {
        // I won't let you brick the machine, even if you're a developer
        if ( isVersalogicPython() && !allowedPythonUpgrade[toTrimaVersion] )
        {
            cerr << "The update from " << versionStringMap[fromTrimaVersion] << " to " << versionStringMap[toTrimaVersion] << " is not allowed on a VersaLogic Python Trima." << endl;
            retval = false;
        }
        else 
        {
            cerr << "The update from " << versionStringMap[fromTrimaVersion] << " to " << versionStringMap[toTrimaVersion] << " is allowed." << endl;
        }
    }
    else
    {
        cerr << "The update from " << versionStringMap[fromTrimaVersion] << " to " << versionStringMap[toTrimaVersion] << " is not allowed." << endl;
        retval = false;
    }

    return retval;
}

bool extractTopLevelFiles()
{
    //
    // Extract the update files
    printf("Extracting updateTrima ...\n");
    if ( tarExtract( UPDATE_PATH "/updateTrima.taz", UPDATE_PATH ) == ERROR )
    {
        printf("Extraction of top level update files failed\n");
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////
//  The main line of update
//////////////////////////////////////////////////////////////////////////////////////
int updateTrima()
{
    int retval = 0;

   //
   // Make sure we don't interrupt anybody else who is running ...
   //
   taskPrioritySet( taskIdSelf(), TRIMA_PRIORITY );

   // Initialize the maps
   if ( init() == false )
   {
      // Can't initialize
      return(-1);
   }

   //
   // Find the software version of the updateTrima.taz file
   //
   bool topLevelFilesExtracted = false;
   char revString[50];

   cerr << "Reading software revision string from updateTrima.taz file." << endl;

   // Get the revision string from the taz file the easy way - works for new taz files
   if ( findTazRevision(UPDATE_PATH "/updateTrima.taz", revString) )
   {
       cerr << "Software revision string from updateTrima.taz is: " << revString << endl;
   }
   else  // Try getting it the hard way
   {
       cerr << "Can't read software version from updateTrima.taz," << endl;
       cerr << "uncompressing it to look in trima.taz." << endl;

       if ( (topLevelFilesExtracted = extractTopLevelFiles()) )
       {
           cerr << "Extracted top level files." << endl;
           cerr << "Reading software revision string from trima.taz file." << endl;

           if ( findTazRevision(UPDATE_PATH "/trima.taz", revString) )
           {
               cerr << "Software revision string from trima.taz is: " << revString << endl;
           }
           else
           {
               // Can't get the taz version
               cerr << "Can't read the software version from trima.taz file, aborting update." << endl;
               return(-1);
           }
       }
       else
       {
           // Can't get the taz version
           cerr << "Can't extract trima.taz from updateTrima.taz, aborting update." << endl;
           return(-1);
       }
   }

   // Parse what we got
   if ( !parseRevision(revString, toTrimaVersion) )
   {
       // Can't parse the string into anything intelligible
       cerr << "Can't determine software version from revision string, aborting update." << endl;
       return(-1);
   }
   cerr << "Software version from updateTrima.taz is: " << versionStringMap[toTrimaVersion] << endl;


   // Read the "from" revision
   cerr << "Reading software revision string of software installed on the machine." << endl;
   if ( readProjectRevisionFile(revString) )
   {
       cerr << "Software revision string of software installed on the machine is: " << revString << endl;

       // Parse it
       if ( !parseRevision(revString, fromTrimaVersion) )
       {
           // Can't parse the string into anything intelligible
           cerr << "Can't determine software version from revision string, aborting update." << endl;
           return(-1);
       }
   }
   else
   {
       // Can't read the projectrevision file
       cerr << "Can't read the software revision string of software installed on the machine, aborting update." << endl;
       return(-1);
   }

   cerr << "Software version of software installed on the machine is: " << versionStringMap[fromTrimaVersion] << endl;


   // Checking hardware type
   if ( isAmpro() )
   {
       cerr << "This is an Ampro Trima." << endl;
   }
   else if ( isVersalogicPython() )
   {
       cerr << "This is a Versalogic Python Trima." << endl;
   }
   else if ( isVersalogic() )
   {
       cerr << "This is a Versalogic Trima." << endl;
   }
   else
   {
       cerr << "Can't determine hardware type, aborting update." << endl;
       return(-1);
   }

   // Check if this upgrade is allowed
   if ( !allowedUpgradePath() )
   {
       return(-1);
   }

   // Extract the top-level files, if haven't done it already
   if ( retval != -1 && !topLevelFilesExtracted )
   {
       if ( (topLevelFilesExtracted = extractTopLevelFiles()) == false )
       {
           cerr << "Can't extract top level files from updateTrima.taz, aborting update." << endl;
           retval = -1;
       }
   }

   // Do the upgrading
   if ( versionMap[toTrimaVersion] != NULL && versionMap[toTrimaVersion]->upgrade(fromTrimaVersion) == 0 )
   {
       cerr << "Upgraded from " << versionStringMap[fromTrimaVersion] << " to " << versionStringMap[toTrimaVersion] << endl;
   }
   else
   {
       retval = -1;
   }

   // Remove the temp files
   remove( UPDATE_PATH "/bootrom.sys" );
   remove( UPDATE_PATH "/vxWorks" );
   remove( UPDATE_PATH "/bootrom_ampro.sys" );
   remove( UPDATE_PATH "/vxWorks_ampro" );
   remove( UPDATE_PATH "/bootrom_versalogic.sys" );
   remove( UPDATE_PATH "/vxWorks_versalogic" );
   remove( UPDATE_PATH "/vxboot.taz" );


   // Write an install summary
   cerr << endl << "Install summary:" << endl;
   if ( retval == 0 )
   {
       cerr << "     Install successful" << endl;
       cerr << "     Old software version: " << versionStringMap[fromTrimaVersion] << endl;
       cerr << "     New software version: " << versionStringMap[toTrimaVersion] << endl;
   }
   else
   {
       cerr << "     Install failed" << endl;
   }
   cerr << endl;

   return( retval );
}
                          
