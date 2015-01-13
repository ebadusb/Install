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

#ifdef __cplusplus
extern "C" {
#endif

int updateTrima ();

int tarExtract (const char* file /* archive file name */,
                const char* location /* location for extraction */);
#ifdef __cplusplus
};
#endif

installLogStream installLog;


bool findTazRevision (const char* searchFileName, char* tazRevString)
{
   const char* projrevString    = "$ProjectRevision: ";
   int         projrevStringLen = strlen(projrevString);
   int         bytesRead        = 0;
   int         totBytesRead     = 0;
   int         overlap          = 50;        // amount of character overlap so we don't miss anything during the search
   int         baselen          = 10 * 1024; // base size of the read buffer
   char        tmpBuffer[baselen + overlap];
   char        tmpChar;
   char        revBuffer[100];
   int         ctr = 0;
   int         i;
   bool        done = false;

//    cerr << "updatetrimaBase :: findTazRevision" << endl;

   // Open the file for uncompressed reading
   gzFile fromFD = gzopen(searchFileName, "r");

   if ( fromFD >= 0 )
   {
//        cerr << searchFileName << " open" << endl;

      // Read the initial buffer-full, including the overlap area
      bytesRead = gzread(fromFD, tmpBuffer, sizeof(tmpBuffer));

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

bool extractTopLevelFiles ()
{
   //
   // Extract the update files
   installLog << "Extracting updateTrima ...\n";

   if ( tarExtract(UPDATE_PATH "/updateTrima.taz", UPDATE_PATH) == ERROR )
   {
      installLog << "Extraction of top level update files failed\n";
      return false;
   }

   return true;
}

static STATUS mySymFindByName (SYMTAB_ID symTblId, const char* name, char** pValue)
{
   char symName[32];
   strncpy(symName, name, sizeof(symName));
   symName[sizeof(symName) - 1] = '\0';

#if (_WRS_VXWORKS_MAJOR < 6)

   SYM_TYPE symType;
   STATUS   status = symFindByName(symTblId, symName, pValue, &symType);

#else // VxWorks 6.x

   SYMBOL_DESC symDesc;
   symDesc.mask    = SYM_FIND_BY_NAME;
   symDesc.name    = symName;
   symDesc.nameLen = strlen(symName);
   STATUS status = symFind(symTblId, &symDesc);
   *pValue = symDesc.value;

#endif

   return status;
}

//////////////////////////////////////////////////////////////////////////////////////
//  The main line of update
//////////////////////////////////////////////////////////////////////////////////////
int updateTrima ()
{
   versionStruct toVer;
   versionStruct fromVer;

   SYM_TYPE symType;
   int      (* keyboardattachedFunc)(void);

   int retval = 0;

   bool topLevelFilesExtracted = false;
   char revString[50];

   // what does the upgrade work
   installer upgrader;

   // open the install log file
   installLog.open(PNAME_INSTALL_LOG);

   //
   // Make sure we don't interrupt anybody else who is running ...
   //
   taskPrioritySet(taskIdSelf(), TRIMA_PRIORITY);

   // the is a development install if a keyboard is attached or the development_only file is present
   updatetrimaUtils::development_install = false;

   development_only_file = fopen("/machine/update/development_only", "r");
   if (development_only_file)
   {
      installLog << "Found /machine/update/development_only\n";
      updatetrimaUtils::development_install = true;
   }
   else if (mySymFindByName(sysSymTbl, "bootKeyboardAttached", (char**)&keyboardattachedFunc) == OK)
   {
      if (keyboardattachedFunc())
      {
         updatetrimaUtils::development_install = true;
         installLog << "Found a keyboard attached\n";
      }
   }

   if ( updatetrimaUtils::development_install )
   {
      installLog << "----- PERFORMING A DEVELOPMENT INSTALL -----\n";
   }

   installLog << "Reading software revision string for the new version from updateTrima.taz file.\n";

   // Get the revision string from the taz file the easy way - works for new taz files
   if ( findTazRevision(UPDATE_PATH "/updateTrima.taz", revString) )
   {
      if ( updatetrimaUtils::development_install )
      {
         updatetrimaUtils::logToScreen = false;
      }
      installLog << "Software revision string from updateTrima.taz is: " << revString << "\n";

      updatetrimaUtils::logToScreen = true;
   }
   else   // Try getting it the hard way
   {
      installLog << "Can't read software version from updateTrima.taz, uncompressing it to look in trima.taz\n";

      if ( (topLevelFilesExtracted = extractTopLevelFiles()) )
      {
         installLog << "Extracted top level files.\n";
         installLog << "Reading software revision string from trima.taz file.\n";

         if ( findTazRevision(UPDATE_PATH "/trima.taz", revString) )
         {
            if ( updatetrimaUtils::development_install )
            {
               updatetrimaUtils::logToScreen = false;
            }
            installLog << "Software revision string from trima.taz is: " << revString << "\n";

            updatetrimaUtils::logToScreen = true;
         }
         else
         {
            // Can't get the taz version
            installLog << "Can't read the software version from trima.taz file, aborting update.\n";
            return(-1);
         }
      }
      else
      {
         // Can't get the taz version
         installLog << "Can't extract trima.taz from updateTrima.taz, aborting update.\n";
         return(-1);
      }
   }

   // Parse what we got
   if ( !updatetrimaUtils::parseRevision(revString, toVer.majorRev, toVer.minorRev, toVer.buildNum) )
   {
      // Can't parse the string into anything intelligible
      installLog << "Can't determine software version from revision string, aborting update.\n";
      return(-1);
   }
   if ( updatetrimaUtils::development_install )
   {
      updatetrimaUtils::logToScreen = false;
   }
   installLog << "Software version for the new version from updateTrima.taz is: "
              << " " << toVer.majorRev
              << "." << toVer.minorRev
              << "." << toVer.buildNum
              << "\n";

   updatetrimaUtils::logToScreen = true;

   // Read the "from" revision
   installLog << "Reading software revision string of software installed on the machine.\n";
   if ( readProjectRevisionFile(revString) )
   {
      if ( updatetrimaUtils::development_install )
      {
         updatetrimaUtils::logToScreen = false;
      }
      installLog << "Software revision string of software installed on the machine is: " << revString << "\n";

      updatetrimaUtils::logToScreen = true;


      // Parse it
      if ( !updatetrimaUtils::parseRevision(revString, fromVer.majorRev, fromVer.minorRev, fromVer.buildNum) )
      {
         // Can't parse the string into anything intelligible
         installLog << "Can't determine software version from revision string, aborting update.\n";
         return(-1);
      }
   }
   else
   {
      // Can't read the projectrevision file
      installLog << "Can't read the software revision string of software installed on the machine, aborting update.\n";
      return(-1);
   }

   installLog << "Software version for the installed version is: "
              << " " << fromVer.majorRev
              << "." << fromVer.minorRev
              << "." << fromVer.buildNum
              << "\n";

   // Extract the top-level files, if haven't done it already
   if ( retval != -1 && !topLevelFilesExtracted )
   {
      installLog << "Extracting top level files.\n";

      if ( (topLevelFilesExtracted = extractTopLevelFiles()) == false )
      {
         installLog << "Can't extract top level files from updateTrima.taz, aborting update.\n";
         retval = -1;
         goto LEAVEROUTINE;
      }
   }

   // Delete the "special" files if it isn't a development install
   if ( !updatetrimaUtils::development_install )
   {
      installLog << "Deleting special files.";

      for ( int i = 0; i < numInstallSpecialFiles; i++ )
      {
         attrib(installSpecialFiles[i], "-R");
         remove(installSpecialFiles[i]);
      }
   }
   else
   {
      installLog << "Development install - not Deleting special files.\n";
   }

/////////////////////////////////
// do the upgrading
   if ( upgrader.upgrade(fromVer, toVer) )
   {
      retval = -1;
      goto LEAVEROUTINE;
   }


LEAVEROUTINE:

   // Set the special files to writeable if it is a development install
   if ( updatetrimaUtils::development_install )
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
   installLog << "Install summary:\n";
   if ( retval == 0 )
   {
      installLog << "     Install successful\n";
      if ( !updatetrimaUtils::development_install )
      {
         updatetrimaUtils::logToScreen = false;
      }

      installLog << "     Old software version:"
                 << " " << fromVer.majorRev
                 << "." << fromVer.minorRev
                 << "." << fromVer.buildNum
                 << "\n";

      installLog << "     New software version:"
                 << " " << toVer.majorRev
                 << "." << toVer.minorRev
                 << "." << toVer.buildNum
                 << "\n";

      updatetrimaUtils::logToScreen = true;
   }
   else
   {
      installLog << "     Install failed\n";
   }
   installLog << "\n";

   installLog.close();

   return( retval );
}

/* FORMAT HASH 3edea9ec7f39adc1975d782f6b5f15c0 */
