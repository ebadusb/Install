/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
// #include "updatetrimaData.h"
#include "installer.h"
#include "updatetrimaUtils.h"

#include <map>
#include <fstream>
#include <sysSymTbl.h>

using namespace std;

FILE* development_only_file;
bool  development_only;

#ifdef __cplusplus
extern "C" {
#endif

int updateTrima ();

int tarExtract (const char* file /* archive file name */,
                const char* location /* location for extraction */);
#ifdef __cplusplus
};
#endif


bool findTazRevision (const char* searchFileName, char* tazRevString)
{
   char* projrevString    = "$ProjectRevision: ";
   int   projrevStringLen = strlen(projrevString);
   int   bytesRead        = 0;
   int   totBytesRead     = 0;
   int   overlap          = 50;        // amount of character overlap so we don't miss anything during the search
   int   baselen          = 10 * 1024; // base size of the read buffer
   char  tmpBuffer[baselen + overlap];
   char  tmpChar;
   char  revBuffer[100];
   int   ctr  = 0;
   int   i;
   bool  done = false;

//    cerr << "updatetrimaBase :: findTazRevision" << endl;

   // Open the file for uncompressed reading
   gzFile fromFD = gzopen(searchFileName, "r");

   if ( fromFD >= 0 )
   {
//        cerr << searchFileName << " open" << endl;

      // Read the initial buffer-full, including the overlap area
      bytesRead    = gzread(fromFD, tmpBuffer, sizeof(tmpBuffer));

      bytesRead   -= overlap;
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
                     for ( int j = 0; j < sizeof(revBuffer) && j < sizeof(tmpBuffer) - i && !done; j++ )
                     {
//                                cerr << "." ;

                        // Copy until we hit the ending $
                        if ( tmpBuffer[i + j] != '$' )
                        {
                           revBuffer[j] = tmpBuffer[i + j];
                        }
                        else
                        {
                           revBuffer[j] = 0;
                           done         = true;

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
      while ( !done && (bytesRead = gzread(fromFD, tmpBuffer + overlap, baselen) > 0) );
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

bool readProjectRevisionFile (char* revString)
{
   char tmpNum[50];

   // File is in the format: "$ProjectRevision: <majorRev>.[<minorRev>.]<build> $"
   ifstream fin;
   fin.open(TRIMA_PROJECT_REVISION_FILE);
   if ( fin.is_open() )
   {
      // Read until the space
      fin.getline(tmpNum, sizeof(tmpNum) - 1, ' ');
      string tmpNumString(tmpNum);
      if ( tmpNumString == "$ProjectRevision:" )
      {
         // Read until the closing $
         fin.getline(tmpNum, sizeof(tmpNum) - 1, '$');
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

/*
bool parseRevision(const char *revString, int curMajorRev, int curMinorRev, int curBuild)
{
    bool retval = true;
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
return true;

//    return retval;
}
*/

bool extractTopLevelFiles ()
{
   //
   // Extract the update files
   updatetrimaUtils::logger("Extracting updateTrima ...\n");

   if ( tarExtract(UPDATE_PATH "/updateTrima.taz", UPDATE_PATH) == ERROR )
   {
      updatetrimaUtils::logger("Extraction of top level update files failed\n");
      return false;
   }

   return true;
}

//////////////////////////////////////////////////////////////////////////////////////
//  The main line of update
//////////////////////////////////////////////////////////////////////////////////////
int updateTrima ()
{
   updatetrimaUtils::loggingEnabled = true;
   updatetrimaUtils::logToScreen    = true;
   char          logBuff[256];

   versionStruct toVer;
   versionStruct fromVer;

   SYM_TYPE      symType;
   int           (* keyboardattachedFunc)(void);

   int           retval                 = 0;

   bool          topLevelFilesExtracted = false;
   char          revString[50];

   // what does the upgrade work
   installer upgrader;

   updatetrimaUtils::initLogging();
//    cerr << "UpdateTrima" << endl;
   updatetrimaUtils::logger("UpdateTrima\n");

// return 0;

   //////////////////////////////////////
   // version numbering test

//    char vernum[80];
//    strcpy(vernum, INSTALL_VERSION);
//    cerr << "Veraion Number: " << vernum << endl;
// #ifndef INSTALL_VERSION
//    #define INSTALL_VERSION "development build"
// #endif
//    cerr << "Version Number: " << INSTALL_VERSION << endl;
//    goto LEAVEROUTINE;

   //
   // Make sure we don't interrupt anybody else who is running ...
   //
   taskPrioritySet(taskIdSelf(), TRIMA_PRIORITY);

   development_only = false;

   // the is a development install if a keyboard is attached or the development_only file is present
   development_only_file = fopen("/machine/update/development_only", "r");

   development_only      = (development_only_file ? true : false); // Don't hate the conditional operator

   if (symFindByName(sysSymTbl, "bootKeyboardAttached", (char**)&keyboardattachedFunc, &symType) == OK)
   {
      development_only |= (* keyboardattachedFunc)();
   }

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// Skip this stuff
/*
   fromVer.majorRev = 12;
   fromVer.minorRev = 0;
   fromVer.buildNum = 999;

   toVer.majorRev = 12;
   toVer.minorRev = 0;
   toVer.buildNum = 999;
 */
/**/
   updatetrimaUtils::logger("Reading software revision string for the new version from updateTrima.taz file.\n");

   // Get the revision string from the taz file the easy way - works for new taz files
   if ( findTazRevision(UPDATE_PATH "/updateTrima.taz", revString) )
   {
      if ( development_only )
      {
         updatetrimaUtils::logger("Software revision string from updateTrima.taz is: ");
         updatetrimaUtils::logger(revString);
         updatetrimaUtils::logger("\n");
      }
   }
   else  // Try getting it the hard way
   {
      updatetrimaUtils::logger("Can't read software version from updateTrima.taz,");
      updatetrimaUtils::logger("uncompressing it to look in trima.taz\n");

      if ( (topLevelFilesExtracted = extractTopLevelFiles()) )
      {
         updatetrimaUtils::logger("Extracted top level files.\n");
         updatetrimaUtils::logger("Reading software revision string from trima.taz file.\n");

         if ( findTazRevision(UPDATE_PATH "/trima.taz", revString) )
         {
            if ( development_only )
            {
               updatetrimaUtils::logger("Software revision string from trima.taz is: ");
               updatetrimaUtils::logger(revString);
               updatetrimaUtils::logger("\n");
            }
         }
         else
         {
            // Can't get the taz version
            updatetrimaUtils::logger("Can't read the software version from trima.taz file, aborting update.\n");
            return(-1);
         }
      }
      else
      {
         // Can't get the taz version
         updatetrimaUtils::logger("Can't extract trima.taz from updateTrima.taz, aborting update.\n");
         return(-1);
      }
   }

   // Parse what we got
   if ( !updatetrimaUtils::parseRevision(revString, toVer.majorRev, toVer.minorRev, toVer.buildNum) )
   {
      // Can't parse the string into anything intelligible
      updatetrimaUtils::logger("Can't determine software version from revision string, aborting update.\n");
      return(-1);
   }
   if ( development_only )
   {
      updatetrimaUtils::logger("Software version for the new version from updateTrima.taz is: ");
      sprintf(logBuff, "%d.%d.%d\n", toVer.majorRev, toVer.minorRev, toVer.buildNum);
      updatetrimaUtils::logger(logBuff);
      updatetrimaUtils::logger("\n");
   }


   // Read the "from" revision
   updatetrimaUtils::logger("Reading software revision string of software installed on the machine.\n");
   if ( readProjectRevisionFile(revString) )
   {
      if ( development_only )
      {
         updatetrimaUtils::logger("Software revision string of software installed on the machine is: ");
         updatetrimaUtils::logger(revString);
         updatetrimaUtils::logger("\n");
      }

      // Parse it
      if ( !updatetrimaUtils::parseRevision(revString, fromVer.majorRev, fromVer.minorRev, fromVer.buildNum) )
      {
         // Can't parse the string into anything intelligible
         updatetrimaUtils::logger("Can't determine software version from revision string, aborting update.\n");
         return(-1);
      }
   }
   else
   {
      // Can't read the projectrevision file
      updatetrimaUtils::logger("Can't read the software revision string of software installed on the machine, aborting update.\n");
      return(-1);
   }

   updatetrimaUtils::logger("Software version for the installed version is: ");
   sprintf(logBuff, "%d.%d.%d\n", fromVer.majorRev, fromVer.minorRev, fromVer.buildNum);
   updatetrimaUtils::logger(logBuff);
   updatetrimaUtils::logger("\n");

   // Extract the top-level files, if haven't done it already
   if ( retval != -1 && !topLevelFilesExtracted )
   {
      updatetrimaUtils::logger("Extracting top level files.\n");

      if ( (topLevelFilesExtracted = extractTopLevelFiles()) == false )
      {
         updatetrimaUtils::logger("Can't extract top level files from updateTrima.taz, aborting update.\n");
         retval = -1;
         goto LEAVEROUTINE;
      }
   }

   // Delete the "special" files if it isn't a development install
   if ( !development_only )
   {
      updatetrimaUtils::logger("Deleting special files.");

      for ( int i = 0; i < numInstallSpecialFiles; i++ )
      {
         attrib(installSpecialFiles[i], "-R");
         remove(installSpecialFiles[i]);
      }
   }
/**/
/////////////////////////////////
// do the upgrading
   if ( upgrader.upgrade(fromVer, toVer) )
   {
      retval = -1;
      goto LEAVEROUTINE;
   }


LEAVEROUTINE:

   // Set the special files to writeable if it is a development install
   if ( development_only )
   {
      for ( int i = 0; i < numInstallSpecialFiles; i++ )
      {
         attrib(installSpecialFiles[i], "-R");
      }
   }

   // Remove the temp files
   remove(UPDATE_PATH "/bootrom.sys");
   remove(UPDATE_PATH "/vxWorks");
   remove(UPDATE_PATH "/bootrom_ampro.sys");
   remove(UPDATE_PATH "/vxWorks_ampro");
   remove(UPDATE_PATH "/bootrom_versalogic.sys");
   remove(UPDATE_PATH "/vxWorks_versalogic");
   remove(UPDATE_PATH "/vxboot.taz");

   // Write an install summary
   updatetrimaUtils::logger("Install summary:\n");
   if ( retval == 0 )
   {
      updatetrimaUtils::logger("     Install successful\n");
      if ( development_only )
      {
         char tmpBuff[256];
         sprintf(tmpBuff, "     Old software version: %d.%d.%d\n", fromVer.majorRev, fromVer.minorRev, fromVer.buildNum);
         updatetrimaUtils::logger(tmpBuff);
         sprintf(tmpBuff, "     New software version: %d.%d.%d\n", toVer.majorRev, toVer.minorRev, toVer.buildNum);
         updatetrimaUtils::logger(tmpBuff);
      }
   }
   else
   {
      updatetrimaUtils::logger("     Install failed\n");
   }
   updatetrimaUtils::logger("\n");

   updatetrimaUtils::closelogger();

   return( retval );
}

/* FORMAT HASH 87000be222fbc9e393469c1fd0dc4347 */
