/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "installer.h"

// #include "crc.c"
#include "crcgen.h"
#include <iostream>
#include <fstream>

// #include "zlib.h"
#include "targzextract.c"
#include "updatecassette_dat.h"

#include "bloodtype.h"
#include "cfg_ids.h"
#include "configdef.h"

#include <stdlib.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

int cp (const char* from, const char* to);
int copyFileContiguous (const char* from, const char* to);

int softcrc (const char* options);

#ifdef __cplusplus
};
#endif

extern "C" STATUS xdelete (const char* fileName);

extern installLogStream installLog;

// Default constructor
installer::installer()
   : currVersion(NULL),
     newVersion(NULL),
     newBuildRef(0),
     prevBuildRef(0),
     is64To7Upgrade(false)
{}

// Copy constructor
installer::installer(const installer& obj)
{}

// Default destructor
installer::~installer()
{}

bool installer::replaceDatfileLine (const char* datFileName, const char* optionName, const char* newVal, bool addCRC)
{
   bool retval     = true;
   bool madeChange = false;
   char destDatFileName[256];
   char oldDatFileName[256];

   char  inputBuffer[256];
   char  workingBuffer[256];
   char  baseNewlineBuf[256]; // buffer containing option with new value
   char  newlineBuf[256];     // buffer containing option with new value and CRC appended
   char* outputLine = inputBuffer;

   sprintf(destDatFileName, "%s.new", datFileName);
   sprintf(oldDatFileName, "%s.old", datFileName);

   FILE* fpSource = fopen(datFileName, "r");
   FILE* fpDest   = fopen(destDatFileName, "w");

   // if either file didn't open, bail out
   if ( !fpSource || !fpDest )
   {
      if (!fpSource) installLog << "fpSource " << datFileName << " failed to open\n";
      if (!fpDest) installLog << "fpDest " << destDatFileName << " failed to open\n";

      retval = false;
      fclose(fpSource);
      fclose(fpDest);
      goto LEAVEROUTINE;
   }

   memset(inputBuffer, 0, sizeof(inputBuffer));

   while ( fgets(inputBuffer, 256, fpSource) )
   {
      // init working buffer
      memset(workingBuffer, 0, sizeof(workingBuffer));

      // make a copy to work on
      strncpy(workingBuffer, inputBuffer, strlen(inputBuffer));

      char* start = workingBuffer + strspn(workingBuffer, " \t");    // get rid of leading white space

      // find the beginning of the value and the length to use in the compare
      char* valStart   = start + strcspn(start, " =");
      int   origValLen = valStart - start;
      int   maxValLen  = (origValLen > strlen(optionName) ? origValLen : strlen(optionName));

      // did we find the option?
      if ( strncmp(start, optionName, maxValLen) == 0 )
      {
         // now look for the existing value
         start += strlen(optionName)+1;
         start[strcspn(start, ",\n\r")] = '\0'; // stop it at the comma in front of the crc & get rid of trailing cr/lf's

         // is the value not equal to the new value?
         if ( strncmp(start, newVal, strlen(newVal)) != 0 )
         {
            installLog << "For option " << optionName << ", current Val " << start << " != newVal " << newVal << ", replacing.\n";

            unsigned long crcval = 0;
            long          buflen = 0;

            // create the updated line
            buflen = sprintf(baseNewlineBuf, "%s=%s", optionName, newVal);

            if (addCRC)
            {
               // calc the crc for the line
               crcgen32(&crcval, (const unsigned char*)baseNewlineBuf, buflen);

               // add the crc to the line
               buflen = sprintf(newlineBuf, "%s,%lx\n", baseNewlineBuf, crcval);
            }
            else
            {
               buflen = sprintf(newlineBuf, "%s\n", baseNewlineBuf);
            }

            // set the new line to be the one written
            outputLine = newlineBuf;

            // set this so we know we've made a change
            madeChange = true;
         }
         else
         {
            installLog << "For option " << optionName << ", current Val " << start << " == newVal " << newVal << ", no update needed.\n";
         }
      }
      else
      {
         outputLine = inputBuffer;
      }

      // write the line to the dest file
      fprintf(fpDest, "%s", outputLine);

      // reset the output pointer in case we changed it
      outputLine = inputBuffer;

      // re-init input buffer
      memset(inputBuffer, 0, sizeof(inputBuffer));
   }

   // close 'em up
   fflush(fpDest);
   fclose(fpSource);
   fclose(fpDest);

   // if we made a change then copy the new file over the old one,
   // otherwise throw away the new file since it should be the same as the old one
   if ( madeChange )
   {
      if ( mv(datFileName, oldDatFileName) == ERROR )
      {
         // installLog << "move existing file failed\n";
         retval = false;
      }
      else if ( mv(destDatFileName, datFileName) == ERROR )
      {
         // installLog << "move new file to existing file failed\n";
         mv(oldDatFileName, datFileName);
         retval = false;
      }
   }
   else
   {
      rm(destDatFileName);
   }

LEAVEROUTINE: ;     // this extra semicolon cleans up auto-indenting after the label

   return( retval);
}



const char* installer::findSetting (const char* setting, FILE* fp)
{
   char* result = NULL;
   if ( fp )
   {
      char buffer[256];
      while ( !result && fgets(buffer, 256, fp) )
      {
         char* start = buffer + strspn(buffer, "# \t");
         if ( strncmp(start, setting, strlen(setting)) == 0 )
         {
            start += strlen(setting);

            start[strcspn(start, "\n\r")] = '\0';

            result = (char*)malloc(strlen(start) + 1);

            strcpy(result, start);
         }
      }
   }
   return result;
}

const char* installer::findSetting (const char* setting, const char* fileName)
{
   const char* result = NULL;
   FILE*       fp     = fopen(fileName, "r");

   if ( fp )
   {
      result = findSetting(setting, fp);
      fclose(fp);
   }

   return result;
}

bool installer::replaceCassette (const char* refStr, unsigned int tubingSetCode, const char* barcodeStr)
{
   bool retval = true;

   if (!AdminUpdateCassetteDat::fileOK())
   {
      AdminUpdateCassetteDat::read();
   }

   if ( AdminUpdateCassetteDat::fileOK() )
   {
      for (UPDATE_CASSETTE_VECTOR_ITERATOR iter = AdminUpdateCassetteDat::begin(); iter != AdminUpdateCassetteDat::end(); ++iter)
      {
         if (strcmp((*iter)->RefNum(), refStr) == 0 && (*iter)->AdminCode() != tubingSetCode)
         {
            AdminUpdateCassetteDat::erase(iter);
            AdminUpdateCassetteDat::addCassette(refStr, tubingSetCode, barcodeStr);

            if (!AdminUpdateCassetteDat::updateCassetteFile())
            {
               retval = false;
            }
            break;
         }
      }
   }
   else
   {
      installLog << "Couldn't read setconfig.dat so can't replace a line in it\n";
      retval = false;
   }

   return( retval );
}

// This compares the set_config.dat vs the non-template cassette.dat so do it AFTER updateCassette() and updateSetConfig()
bool installer::validateSetConfig (versionStruct& toVer)
{
   bool retval      = true;
   bool madeChanges = false;
   bool deleteItem  = false;

   installLog << "validateSetConfig\n";

   // look to see if we're installing 5.1 and quit if we are because 5.1 doesn't use the cassette files
   struct stat fileStat;

   if ( stat((const char*)TEMPLATES_PATH "/" FILE_CASSETTE_DAT, &fileStat) != OK )
   {
      installLog << "Installing 5.1, ignoring cassette file validation\n";
      goto LEAVEROUTINE;
   }

   installLog << "majorRev " << toVer.majorRev << "\n";

   // Admin is /config/setconfig.dat
   if (!AdminUpdateCassetteDat::fileOK())
   {
      AdminUpdateCassetteDat::read();
   }

   // Master is /config/cassette.dat
   if (!MasterUpdateCassetteDat::fileOK())
   {
      MasterUpdateCassetteDat::read();
   }

   if ( AdminUpdateCassetteDat::fileOK() && MasterUpdateCassetteDat::fileOK() )
   {
      UPDATE_CASSETTE_VECTOR_ITERATOR iter = AdminUpdateCassetteDat::begin();
      while (iter != AdminUpdateCassetteDat::end())
      {
         UPDATE_CASSETTE_MAP_ITERATOR foundCassette = MasterUpdateCassetteDat::find((*iter)->RefNum());


         if ( UpdateCassetteDat::isTwoPConnectorCassette(atoi((*iter)->RefNum()) ) )
         {
            installLog << "Removing 2p connector ref  -- Cassette ref #: " << (*iter)->RefNum() << " removed from seconfig.dat\n";
            deleteItem = true;
         }
         else if (foundCassette == MasterUpdateCassetteDat::end()) // not in cassette.dat
         {
            // didn't find the cassette from set_config.dat in cassette.dat so delete it
            installLog << "Cassette ref #: " << (*iter)->RefNum() << " not found in cassette.dat\n";
            deleteItem = true;
         }
         else if ( foundCassette->second->AdminCode() != (*iter)->AdminCode() ||
                   strcmp(foundCassette->second->BarcodeNum(), (*iter)->BarcodeNum()) != 0 )
         {
            installLog << "cassette ref #: " << (*iter)->RefNum() << " has different admin or barcode, deleting\n", (int)((*iter)->RefNum()) ;
            installLog << "cassette.dat admin code: " << (int)(foundCassette->second->AdminCode())
                       << " barcode: " << foundCassette->second->BarcodeNum()
                       << " setconfig.dat admin code: " << (int)((*iter)->AdminCode())
                       << " barcode: " << (*iter)->BarcodeNum() << "\n";
            deleteItem = true;
         }



         if (deleteItem)
         {
            installLog << "Cassette ref #: " << (*iter)->RefNum() << " entry is removed\n";
            iter        = AdminUpdateCassetteDat::erase(iter);
            madeChanges = true;
         }
         else
         {
            iter++;
         }

         deleteItem = false;
      }

      if (madeChanges)
      {
         AdminUpdateCassetteDat::updateCassetteFile();
      }
   }
   else
   {
      installLog << "Couldn't read cassette.dat or setconfig.dat so nothing to validate\n";
      retval = false;
   }

   if ( !madeChanges )
   {
      installLog << "All cassettes in setconfig.dat passed validation with cassette.dat\n";
   }

LEAVEROUTINE: ;

   return( retval );
}


bool installer::updatePostCount ()
{
   bool bUpdate = false;

   CDatFileReader datfile(PNAME_CONFIGDAT);
   if ( datfile.Error() )
   {
      installLog << "Config file read error : " << datfile.Error() << "\n";
      return bUpdate;
   }

   // IT 8741/8737 - Force post count config setting to 100K
   float postCount = datfile.GetFloat("PROCEDURE_CONFIG", "key_post_plat");
   if (postCount < 100000)
   {
      datfile.RemoveLine("PROCEDURE_CONFIG", "key_post_plat");
      datfile.AddLine("PROCEDURE_CONFIG", "key_post_plat", "100000");
      installLog << "Changed donor post count from " << postCount << " to 100000\n";
      bUpdate = true;
   }

   if ( bUpdate )
   {
      datfile.WriteCfgFile(FILE_CONFIG_DAT);
   }

   return bUpdate;
}

void installer::updateRBC ()
{
   // Put the rbc.dat file in the correct location.
   attrib(CONFIG_PATH "/" FILE_RBC_DAT, "-R");
   if ( cp(TEMPLATES_PATH "/" FILE_RBC_DAT, CONFIG_PATH "/" FILE_RBC_DAT) == ERROR )
   {
      installLog << "copy of rbc.dat failed\n";
      return;
   }
   attrib(CONFIG_PATH "/" FILE_RBC_DAT, "+R");

}

void installer::updateGlobVars ()
{
   currVersion = findSetting("file_version=", GLOBVARS_FILE);
   newVersion  = findSetting("file_version=", GLOBVARS_DEFAULT_FILE);

   // Create the dat file reader to retrieve the global environment vars data.
   CDatFileReader datfile(GLOBVARS_FILE, false, true);
   if ( datfile.Error() )
   {
      installLog << GLOBVARS_FILE << " file read error : " << datfile.Error() << "\n";
      return;
   }

   string machineID;
   string externalIP;
   string externalBIP;

   if ( !datfile.Find("EXTERNALIP") )
   {
      installLog << "pre-v5.1 globvars file found Unable to Convert... ending\n";
      return;
   }
   else
   {
      // Need to change the globvars?
      if ((newVersion != currVersion && (newVersion == NULL || currVersion == NULL)) // one is NULL but noth both
          || (newVersion && currVersion && strcmp(newVersion, currVersion) != 0))    // neither NULL, strings are different
      {
         installLog << "Need to update globvars to correct format.\n";

         // get existing data for conversion
         if (!datfile.Find("MACHINE") || !datfile.Find("EXTERNALIP") || !datfile.Find("EXTERNALBIP"))
         {
            installLog << "Error: unable to read globvars to convert to new version.\n";
            return;
         }
         else
         {
            machineID   = datfile.GetString("MACHINE");
            externalIP  = datfile.GetString("EXTERNALIP");
            externalBIP = datfile.GetString("EXTERNALBIP");
         }

         attrib(GLOBVARS_FILE ".gvtemp", "-R");
         rm(GLOBVARS_FILE ".gvtemp");

         // copy the new version
         if ( cp(GLOBVARS_DEFAULT_FILE, GLOBVARS_FILE ".gvtemp") == ERROR )
         {
            installLog << "Error: unable to copy globvars.default\n";
            return;
         }

         // update new version with data
         replaceDatfileLine (GLOBVARS_FILE ".gvtemp", "MACHINE", machineID.c_str(), false);
         replaceDatfileLine (GLOBVARS_FILE ".gvtemp", "EXTERNALIP", externalIP.c_str(), false);
         replaceDatfileLine (GLOBVARS_FILE ".gvtemp", "EXTERNALBIP", externalBIP.c_str(), false);

         attrib(GLOBVARS_FILE, "-R");  // do this just in case

         if ( cp(GLOBVARS_FILE ".gvtemp", GLOBVARS_FILE) == ERROR )
         {
            installLog << "Error: unable to update globvars\n";
         }

         rm(GLOBVARS_FILE ".gvtemp");
      }

   }
   return;
}

void installer::updateHW ()
{
   bool hwdatFileName = false;  // is the versalogic file named hw.dat?

   // Replace hw.dat if the version number has changed
   currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_HW_DAT);

   if ( isAmpro() )
   {
      newVersion = findSetting("file_version=", TEMPLATES_PATH "/hw_ampro.dat");
      if ( newVersion == NULL )
      {
         installLog << "Could not find hw_ampro.dat\n";
         return;
      }
   }
   else
   {
      // look for versalogic_hw.dat first then try hw.dat if it isn't found
      newVersion = findSetting("file_version=", TEMPLATES_PATH "/hw_versalogic.dat");
      if ( newVersion == NULL )
      {
         installLog << "Could not find hw_versalogic.dat, trying hw.dat\n";

         newVersion = findSetting("file_version=", TEMPLATES_PATH "/hw.dat");
         if ( newVersion == NULL )
         {
            installLog << "Could not find hw.dat\n";
            return;
         }
         else
         {
            hwdatFileName = true;
         }
      }
   }

   if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
   {
      installLog << "Updating hw.dat to new version " << newVersion << " from existing version " << currVersion << "\n";

      attrib(CONFIG_PATH "/" FILE_HW_DAT, "-R");

      if ( isAmpro() )
      {
         if ( cp(TEMPLATES_PATH "/hw_ampro.dat", CONFIG_PATH "/" FILE_HW_DAT) == ERROR )
         {
            installLog << "copy of hw_ampro.dat failed\n";
            return;
         }
      }
      else
      {
         if ( hwdatFileName )
         {
            if ( cp(TEMPLATES_PATH "/hw.dat", CONFIG_PATH "/" FILE_HW_DAT) == ERROR )
            {
               installLog << "copy of hw.dat failed\n";
               return;
            }
         }
         else
         {
            if ( cp(TEMPLATES_PATH "/hw_versalogic.dat", CONFIG_PATH "/" FILE_HW_DAT) == ERROR )
            {
               installLog << "copy of hw_versalogic.dat failed\n";
               return;
            }
         }
      }
      attrib(CONFIG_PATH "/" FILE_HW_DAT, "+R");
      fflush(stdout);
   }

}


void installer::updateAppServer ()
{
   // these are the customer selected sets.... dont overwrite if it exists!
   currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_APPSERVER_DAT);
   newVersion  = findSetting("file_version=", TEMPLATES_PATH "/" FILE_APPSERVER_DAT);

   if ( newVersion == NULL )
   {
      installLog << "Ignoring app_server.dat\n";
   }
   else if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
   {
      installLog << "Updating app_server.dat to new version " << newVersion;
      if (currVersion)
      {
         installLog << " from existing version " << currVersion << "\n";
      }
      installLog << "\n";

      attrib(CONFIG_PATH "/" FILE_APPSERVER_DAT, "-R");

      if ( cp(TEMPLATES_PATH "/" FILE_APPSERVER_DAT, CONFIG_PATH "/" FILE_APPSERVER_DAT) == ERROR )
      {
         installLog << "copy of " << FILE_APPSERVER_DAT << " failed\n";
         return;
      }

      attrib(CONFIG_PATH "/" FILE_APPSERVER_DAT, "+R");
      fflush(stdout);
   }
}

bool installer::appendSerialNumToZipFile (const char* filename, bool sectionHdr)
{
   bool retval = false;  // assume it didn't work

   installLog << "Appending serial number to " << filename << "\n";

   std::string tempFile = filename;
   tempFile += ".temp";

   std::string bakFile = filename;
   bakFile += ".bak";

   struct stat fileStat;

   if ( stat((char*)filename, &fileStat) == OK )
   {
      installLog << filename << " exists.\n ";
      cp(filename, "/machine/install/f0.txt"); // IT17453 debug - TODO remove
      // delete any existing temp file
      if ( stat((char*)(tempFile.c_str()), &fileStat) == OK )
      {
         installLog << "temp file " << tempFile << " exists.\n ";
         remove(tempFile.c_str());
         if ( stat((char*)(tempFile.c_str()), &fileStat) == OK )
         {
            installLog << "remove of " <<  tempFile.c_str() << " failed.\n";
         }
      }

      // unzip the file to a temp file
      if ( updatetrimaUtils::unzipFile(filename, tempFile.c_str()) )
      {
         cp(tempFile.c_str(), "/machine/install/f1.txt"); // IT17453 debug - TODO remove
         // open the temp file for appending
         FILE* tempfp = fopen(tempFile.c_str(), "a");
         if ( tempfp )
         {
            installLog << "temp file " << tempFile << " opened.\n ";
            // get the machine name from globvars
            const char* machineName = NULL;
            machineName = findSetting("MACHINE=", CONFIG_PATH "/globvars");

            if ( machineName )
            {
               installLog << "machine name is " << machineName << "\n";
               unsigned long crcval = 0;
               long          buflen = 0;
               char          serialnumBuf[256];
               char          serialnumBufLine[256];

               // create the base serial number line
               buflen = sprintf(serialnumBuf, "serial_number=%s", machineName);

               // calc the crc for the line
               crcgen32(&crcval, (const unsigned char*)serialnumBuf, buflen);

               // add the crc to the line
               buflen = sprintf(serialnumBufLine, "%s,%lx", serialnumBuf, crcval);

               // write the section header to the file, if requested
               if (sectionHdr)
               {
                  installLog << "adding section header\n";
                  fprintf(tempfp, "\n[MACHINE_ID]");
               }

               // write the machine ID to the file
               installLog << "adding serial number: " << serialnumBufLine << "\n";
               fprintf(tempfp, "\n%s\n", serialnumBufLine);

               fflush(tempfp);

               // close the temp file
               fclose(tempfp);
               cp(tempFile.c_str(), "/machine/install/f2.txt"); // IT17453 debug - TODO remove

               if ( cp(filename, bakFile.c_str()) == ERROR )
               {
                  installLog << "Creation of backup file failed\n";
               }
               cp(bakFile.c_str(), "/machine/install/f3.txt");  // IT17453 debug - TODO remove
               cp(filename, "/machine/install/f4.txt");         // IT17453 debug - TODO remove
               cp(tempFile.c_str(), "/machine/install/f5.txt"); // IT17453 debug - TODO remove

               // delete the original file
               attrib(filename, "-R");
               remove(filename);
               if ( stat((char*)filename, &fileStat) == OK )
               {
                  installLog << "remove of " <<  filename << " failed.\n";
               }

               // zip the temp file to the original file name
               if (updatetrimaUtils::zipFile(tempFile.c_str(), filename) == 0)
               {
                  // zip failed
                  installLog << "zip of " << filename << " failed, restoring original\n";
                  cp(bakFile.c_str(), filename);
               }
               else
               {
                  retval = true;
                  cp(tempFile.c_str(), "/machine/install/f6.txt"); // IT17453 debug - TODO remove
                  cp(filename, "/machine/install/f7.txt");         // IT17453 debug - TODO remove
               }

               // clean up files (I created them, so they must be writable)
               remove(tempFile.c_str());
               if ( stat((char*)(tempFile.c_str()), &fileStat) == OK )
               {
                  installLog << "remove of " <<  tempFile.c_str() << " failed.\n";
               }

               remove(bakFile.c_str());
               if ( stat((char*)(bakFile.c_str()), &fileStat) == OK )
               {
                  installLog << "remove of " <<  bakFile.c_str() << " failed.\n";
               }
            }
            else     // couldn't get a serial number
            {
               installLog << "couldn't get a serial number from globvars\n";

               // close & delete the temp file
               fclose(tempfp);
               remove(tempFile.c_str());
               if ( stat((char*)(tempFile.c_str()), &fileStat) == OK )
               {
                  installLog << "remove of " <<  tempFile.c_str() << " failed.\n";
               }
            }
         }
         else
         {
            installLog << "couldn't open " << tempFile.c_str() << " for append\n";

            // delete the temp file
            remove(tempFile.c_str());
            if ( stat((char*)(tempFile.c_str()), &fileStat) == OK )
            {
               installLog << "remove of " <<  tempFile.c_str() << " failed.\n";
            }
         }
      }
      else   // couldn't unzip file
      {
         installLog << "unzip of " << filename << " failed\n";
      }
   }
   else  // filename doesn't exist
   {
      installLog << filename << " doesn't exist\n";
   }

   return retval;
}

void installer::updateSW ()
{
   // Look if there is a features.bin and use it instead of sw.dat
   struct stat featuresFileStat;

   if ( stat((const char*)TEMPLATES_PATH "/" FILE_FEATURES, &featuresFileStat) == OK )
   {
      installLog << TEMPLATES_PATH << "/" << FILE_FEATURES << " does exist.\n";

      if ( appendSerialNumToZipFile(TEMPLATES_PATH "/" FILE_FEATURES, true) )
      {
         installLog << "appendSerialNumToZipFile succeeded.\n";

         // copy features.bin to the config directory
         attrib(PNAME_FEATURES, "-R");
         if ( cp(TEMPLATES_PATH "/" FILE_FEATURES, PNAME_FEATURES) == ERROR )
         {
            installLog << "copy of " << FILE_FEATURES << " failed\n";
         }

         attrib(PNAME_FEATURES, "+R");
      }
      else
      {
         installLog << "appendSerialNumToZipFile failed\n";
      }

      // remove the old sw.dat files
      attrib(PNAME_SWDAT, "-R");
      remove(PNAME_SWDAT);
      if ( stat((const char*)PNAME_SWDAT, &featuresFileStat) == OK )
      {
         installLog << "Delete of " << PNAME_SWDAT << " failed\n";
      }

      attrib(TEMPLATES_PATH, "-R");
      remove(TEMPLATES_PATH "/" FILE_SW_DAT);
      if ( stat((const char*)TEMPLATES_PATH "/" FILE_SW_DAT, &featuresFileStat) == OK )
      {
         installLog << "Delete of " << TEMPLATES_PATH << "/" << FILE_SW_DAT << " failed\n";
      }

   }
   else
   {
      installLog << TEMPLATES_PATH << "/" << FILE_FEATURES << " does not exist.\n";

      // Not installing features.bin so delete any that exist in config directory
      if ( stat((const char*)PNAME_FEATURES, &featuresFileStat) == OK )
      {
         installLog << "removing " << PNAME_FEATURES << "\n";

         attrib(PNAME_FEATURES, "-R");
         remove(PNAME_FEATURES);
         if ( stat((const char*)PNAME_FEATURES, &featuresFileStat) == OK )
         {
            installLog << "remove of " << PNAME_FEATURES << " failed\n";
         }
      }

      // Replace sw.dat if the version number has changed
      currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_SW_DAT);
      newVersion  = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SW_DAT);

      if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
      {
         installLog << "Updating sw.dat to new version " << newVersion;
         if (currVersion)
         {
            installLog << " from existing version " << currVersion << "\n";
         }
         installLog << "\n";

         attrib(CONFIG_PATH "/" FILE_SW_DAT, "-R");

         if ( cp(TEMPLATES_PATH "/" FILE_SW_DAT, CONFIG_PATH "/" FILE_SW_DAT) == ERROR )
         {
            installLog << "copy of " << FILE_SW_DAT << " failed\n";
            goto LEAVEROUTINE;
         }

         attrib(CONFIG_PATH "/" FILE_SW_DAT, "+R");
         fflush(stdout);
      }
   }

   // stuff to do every time you exit
LEAVEROUTINE:

   return;
}

void installer::updateTerror ()
{
   // Replace terror_config.dat if the version number has changed
   currVersion = findSetting("file_version=", TERROR_CONFIG_FILE);
   newVersion  = findSetting("file_version=", TEMPLATES_PATH "/terror_config.dat");

   if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
   {
      installLog << "Updating terror_config.dat to new version " << newVersion << " from existing version " << newVersion << "\n";
      attrib(TERROR_CONFIG_FILE, "-R");

      if ( cp(TEMPLATES_PATH "/terror_config.dat", CONFIG_PATH "/terror_config.dat") == ERROR )
      {
         installLog << "copy of terror_config.dat failed\n";
         return;
      }

      attrib(CONFIG_PATH "/terror_config.dat", "+R");
      fflush(stdout);
   }
}

void installer::updateSounds ()
{
   installLog << "Updating sounds.dat...\n";
   attrib(PNAME_SOUNDSDAT, "-R");

   if ( cp(TEMPLATES_PATH "/" FILE_SOUNDS_DAT, PNAME_SOUNDSDAT) == ERROR )
   {
      installLog << "copy of sounds.dat failed\n";
      return;
   }

   attrib(PNAME_SOUNDSDAT, "+R");
   fflush(stderr);
}

void installer::updateBarcodeCategories ()
{
   struct stat fileStat;

   // if there's a barcode file in templates install it, otherwise delete any in config
   if ( stat( (const char*)TEMPLATES_PATH "/" FILE_BARCODECATEGORY_DAT, &fileStat) == OK )
   {
      // Replace existing one if the version number has changed
      currVersion = findSetting("file_version=", PNAME_BARCODECATEGORYDAT);
      newVersion  = findSetting("file_version=", TEMPLATES_PATH "/" FILE_BARCODECATEGORY_DAT);

      if ( !currVersion || strcmp(newVersion, currVersion) != 0 )
      {
         if ( currVersion == NULL )
         {
            currVersion = "(NON EXISTANT)";
         }
         installLog << "Updating " << FILE_BARCODECATEGORY_DAT << " to new version " << newVersion << " from existing version " << currVersion << "\n";

         attrib(PNAME_BARCODECATEGORYDAT, "-R");

         if ( cp(TEMPLATES_PATH "/" FILE_BARCODECATEGORY_DAT, PNAME_BARCODECATEGORYDAT) == ERROR )
         {
            installLog << "copy of " << FILE_BARCODECATEGORY_DAT << " failed\n";
         }
         else
         {
            installLog << "copy of " << FILE_BARCODECATEGORY_DAT << " succeeded\n";
         }

         attrib(PNAME_BARCODECATEGORYDAT, "+R");
         fflush(stdout);
      }
   }
   else
   {
      attrib(PNAME_BARCODECATEGORYDAT, "-R");
      remove(PNAME_BARCODECATEGORYDAT);
   }
}

void installer::updateRTSConfig ()
{
   struct stat fileStat;

   // if there's a config file in templates install it, otherwise delete any in config
   if ( stat( (const char*)TEMPLATES_PATH "/" FILE_RTSCONFIG_DAT, &fileStat) == OK )
   {
      // Replace existing one if the version number has changed
      currVersion = findSetting("file_version=", PNAME_RTSCONFIGDAT);
      newVersion  = findSetting("file_version=", TEMPLATES_PATH "/" FILE_RTSCONFIG_DAT);

      if ( !currVersion || strcmp(newVersion, currVersion) != 0 )
      {
         if ( currVersion == NULL )
         {
            currVersion = "(NON EXISTANT)";
         }
         installLog << "Updating " << FILE_RTSCONFIG_DAT << " to new version " << newVersion << " from existing version " << currVersion << "\n";

         attrib(PNAME_RTSCONFIGDAT, "-R");
         if ( cp(TEMPLATES_PATH "/" FILE_RTSCONFIG_DAT, PNAME_RTSCONFIGDAT) == ERROR )
         {
            installLog << "copy of " << FILE_RTSCONFIG_DAT << " failed\n";
         }
         else
         {
            installLog << "copy of " << FILE_RTSCONFIG_DAT << " succeeded\n";
         }

         attrib(PNAME_RTSCONFIGDAT, "+R");
         fflush(stdout);
      }
   }
   else
   {
      attrib(PNAME_RTSCONFIGDAT, "-R");
      remove(PNAME_RTSCONFIGDAT);
   }
}

void installer::updateCassette ()
{
   installLog << "updateCassette\n";
   // Replace cassette.dat if the version number has changed
   currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_CASSETTE_DAT);
   newVersion  = findSetting("file_version=", TEMPLATES_PATH "/" FILE_CASSETTE_DAT);

   // if the new version is from before we used cassette.dat, remove it from /config
   if (newVersion == NULL)
   {
      installLog << "Installing 5.1, ignoring cassette.dat\n";
   }
   else if ( !currVersion || strcmp(newVersion, currVersion) != 0 )
   {
      installLog << "Updating " << FILE_CASSETTE_DAT << " to new version " << newVersion << " from existing version " << currVersion << "\n";

      attrib(CONFIG_PATH "/" FILE_CASSETTE_DAT, "-R");

      if ( cp(TEMPLATES_PATH "/" FILE_CASSETTE_DAT, CONFIG_PATH "/" FILE_CASSETTE_DAT) == ERROR )
      {
         installLog << "copy of " << FILE_CASSETTE_DAT << " failed\n";
      }
      else
      {
         installLog << "copy of " << FILE_CASSETTE_DAT << " succeeded\n";
      }

      attrib(CONFIG_PATH "/" FILE_CASSETTE_DAT, "+R");
      fflush(stdout);
   }
}

void installer::updateSetConfig ()
{
   installLog << "updateSetConfig\n";
   // these are the customer selected sets.... dont overwrite if it exists!
   currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_SETCONFIG_DAT);
   newVersion  = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SETCONFIG_DAT);

   // if the new version is from before we used setconfig.dat, remove it from /config
   if ( newVersion == NULL)
   {
      installLog << "Installing 5.1, ignoring setconfig.dat\n";
   }
   else if ( currVersion == NULL )
   {
      // if the file isnt there....
      installLog << "Adding " << FILE_SETCONFIG_DAT << "\n";

      if ( cp(TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT) == ERROR )
      {
         installLog << "copy of " << FILE_SETCONFIG_DAT << " failed\n";
         return;
      }

      attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
      fflush(stdout);
   }
   else
   {
      installLog << FILE_SETCONFIG_DAT << " already exists, not copying a new one.\n";

      // do maintenance on the setconfig.dat file
      // replace 80537 with the new admin code
      if ( replaceCassette("80537", 2370507, "010502058380537") )
      {
         installLog << "Completed maintenance update to setconfig.dat\n";
      }
      else
      {
         installLog << "Unable to make maintenance update to setconfig.dat\n";
      }
   }
}

void installer::updateVista ()
{
   std::string vistaipTmpl(TEMPLATES_PATH "/" FILE_VISTA_IP_DAT);
   struct stat vistaipTemplFileStat;

   // if there's a vista ip template file we have to make sure the config file is updated
   if ( stat(const_cast<char*>(vistaipTmpl.c_str()), &vistaipTemplFileStat) == OK )
   {
      // For first time installs, copy the template file over to config
      struct stat vistaipFileStat;
      if ( stat ((const char*)PNAME_VISTAIPDAT, &vistaipFileStat) == ERROR)
      {
         if ( cp(vistaipTmpl.c_str(), PNAME_VISTAIPDAT) == ERROR )
         {
            installLog << "copy of " << vistaipTmpl.c_str() << " to ";
            installLog << PNAME_VISTAIPDAT << " failed\n";
         }
         else
         {
            installLog << "copied " << vistaipTmpl.c_str() <<" to ";
            installLog << PNAME_VISTAIPDAT << " successfully\n";
         }
      }
      else    // the config already exists so see if we need to update it
      {
         std::string confFormatVerStr  = "";
         std::string templFormatVerStr = "";

         // create the file readers
         CDatFileReader vistaipConfigFile(PNAME_VISTAIPDAT);
         CDatFileReader vistaipTemplFile(vistaipTmpl.c_str());

         if ( !vistaipConfigFile.Error() && !vistaipTemplFile.Error() )
         {
            // get the format versions
            confFormatVerStr  = vistaipConfigFile.GetString("file_version");
            templFormatVerStr = vistaipTemplFile.GetString("file_version");

            fourPartVersion confFormatVer(confFormatVerStr.c_str());
            fourPartVersion templFormatVer(templFormatVerStr.c_str());

            // the version of the template is newer, so update the config file
            if ( templFormatVer > confFormatVer )
            {
               vistaipConfigFile.SetValue("VISTA", "VISTA_DIRECT_SEND_IP", vistaipTemplFile.GetString("VISTA", "VISTA_DIRECT_SEND_IP").c_str());
               vistaipConfigFile.SetValue("VISTA", "VISTA_DIRECT_SEND_PORT", vistaipTemplFile.GetString("VISTA", "VISTA_DIRECT_SEND_PORT").c_str());
               vistaipConfigFile.Write(PNAME_VISTAIPDAT);

               installLog << PNAME_VISTAIPDAT << " file updated.\n";
            }
         }
         else
         {
            installLog << FILE_VISTA_IP_DAT << " file reader creation error\n";
         }
      }
   }

}

int installer::verifyCrc (const char* commandLine)
{
   int crcReturnVal = softcrc(commandLine);

   if (crcReturnVal != 0)
   {
      printf("CRC ERROR %d on command line \"%s\"\n", crcReturnVal, commandLine);
      installLog << "CRC ERROR " << crcReturnVal;
      installLog << " on command line <" << commandLine << ">\n";
   }

   return crcReturnVal;
}


void installer::copyTrapFiles ()
{
   attrib(TRAP_DEFAULTS_FILE, "-R");

   if (cp(TEMPLATES_PATH "/trap_default.dat", TRAP_DEFAULTS_FILE) == ERROR)
   {
      installLog << "copy of trap_default.dat\n";
      return;
   }
   attrib(TRAP_DEFAULTS_FILE, "+R");

   attrib(TRAP_OVERRIDE_FILE, "-R");

   if (cp(TEMPLATES_PATH "/trap_override.dat", TRAP_OVERRIDE_FILE) == ERROR)
   {
      installLog << "copy of trap_override.dat\n";
      return;
   }
   attrib(TRAP_OVERRIDE_FILE, "+R");

}

void installer::updateCal5 ()
{
   //
   // Create the dat file reader to retrieve the calibration data.
   //
   CDatFileReader datfile(PNAME_CALDAT);
   if ( datfile.Error() )
   {
      installLog << "Calibration file read error : " << datfile.Error() << "\n";
      return;
   }
   const std::string tsOriginal [] = {"screen_horizontal_size", "screen_vertical_size", "tsraw_left_edge", "tsraw_right_edge",
                                      "tsraw_top_edge", "tsraw_bottom_edge"};

   const char* tsHeader = "TOUCHSCREEN";
   struct stat fileStat;
   if ( stat((const char*)PNAME_TCHSCRNDAT, &fileStat) == OK )
   {   // 6.0 spiral 4 file
      CDatFileReader tscrnFile(PNAME_TCHSCRNDAT);
      datfile.AddSection("[TOUCHSCREEN]");
      for (int i = 0; i<=5; i++) // Keep both loops separate.
      {
         datfile.AddLine(tsHeader, tsOriginal[i].c_str(), tscrnFile.Find(tsHeader, tsOriginal[i].c_str()));
      }
      datfile.Write(PNAME_CALDAT);
      tscrnFile.Write(PNAME_TCHSCRNDAT);

      if ( remove(PNAME_TCHSCRNDAT) == ERROR )
      {
         installLog << "Removal of " << PNAME_TCHSCRNDAT << " file failed\n";
      }
      return;
   }

   else if ( !datfile.Find("TOUCHSCREEN", "screen_horizontal_size") )
   {
      installLog << "pre-v5.1 cal.dat file found.  Conversion needed\n";

      FILE* fp;
      fp = fopen("/config/absf.2", "r");

      char x1Str[8];
      char x2Str[8];
      char y1Str[8];
      char y2Str[8];

      if ( fp )
      {
         char  buffer[256];
         char* savePtr = 0;
         char* p       = 0;
         int   temp;
         fgets(buffer, 256, fp);
         fclose(fp);

         p = strtok_r(buffer, " :x", &savePtr);  // 0 x position
         p = strtok_r(NULL, " :x", &savePtr);    // 0 y position
         p = strtok_r(NULL, " :x", &savePtr);    // max x position
         p = strtok_r(NULL, " :x", &savePtr);    // max y position
         p = strtok_r(NULL, " :x", &savePtr);    // x1 position
         strcpy(x1Str, p);
         p = strtok_r(NULL, " :x", &savePtr);    // x2 position
         strcpy(x2Str, p);
         p = strtok_r(NULL, " :x", &savePtr);    // y1 position
         strcpy(y1Str, p);
         p = strtok_r(NULL, " :x", &savePtr);    // y2 position
         strcpy(y2Str, p);

         attrib("/config/absf.2", "-R");
         if ( remove("/config/absf.2") == ERROR )
         {
            installLog << "Removal of absf.2 file failed\n";
         }

      }
      else
      {
         //
         // Default values if no absf.2 file ...
         //
         sprintf(x1Str, "%d", 90);
         sprintf(x2Str, "%d", 850);
         sprintf(y1Str, "%d", 130);
         sprintf(y2Str, "%d", 830);
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

      installLog << "cal.dat file converted.\n";

      return;
   }

   installLog << "v5.1 cal.dat file found.  No conversion needed\n";

}

void installer::updateCal6 ()
{
   //
   // Create the dat file reader to retrieve the calibration data.
   //
   CDatFileReader datfile(PNAME_CALDAT);
   if ( datfile.Error() )
   {
      installLog << "Calibration file read error : " << datfile.Error() << "\n";
      return;
   }

   // For first time installs, copy the touchscreen template file over
   struct stat tsFileStat;
   if ( stat ((const char*)PNAME_TCHSCRNDAT, &tsFileStat) == ERROR)
   {
      std::string tsTmpl(TEMPLATES_PATH "/" FILE_TCHSCRN_DAT);
      if ( stat(const_cast<char*>(tsTmpl.c_str()), &tsFileStat) == OK )
      {
         if ( cp(tsTmpl.c_str(), PNAME_TCHSCRNDAT) == ERROR )
         {
            installLog << "copy of " << tsTmpl.c_str() << " to " << PNAME_TCHSCRNDAT << " failed\n";
         }
         else
         {
            installLog << "copied " << tsTmpl.c_str() << " to " << PNAME_TCHSCRNDAT << " successfully\n";
         }
      }
   }


   CDatFileReader tscrnFile(PNAME_TCHSCRNDAT);
   if ( tscrnFile.Error() )
   {
      installLog << "Calibration file read error : " << datfile.Error() << "\n";
      return;
   }

   const char*       tsHeader      = "TOUCHSCREEN";
   const std::string tsAF []       = {"a", "b", "c", "d", "e", "f"};
   const std::string tsOriginal [] = {"screen_horizontal_size", "screen_vertical_size", "tsraw_left_edge", "tsraw_right_edge",
                                      "tsraw_top_edge", "tsraw_bottom_edge"};

   // Is it an old 6.0 install (5.8)
   if ( datfile.Find(tsHeader, "a") )
   {
      installLog << "v6.0 " << FILE_CAL_DAT << " old 6.0 cal file found. Conversion needed\n";
      // Move all TOUCHSCREEN related data to touch_screen.dat

      // Transfer values from cal.dat to touch_screen.dat
      for (int i = 0; i<=5; i++)
      {
         const char* val = datfile.Find(tsHeader, tsAF[i].c_str());
         if (val)
         {
            tscrnFile.SetValue(tsHeader, tsAF[i].c_str(), val);
            datfile.RemoveLine(tsHeader, tsAF[i].c_str());
         }
         else
            cout << tsHeader << ":" << tsOriginal[i] << " not found " << std::endl;
      }
      for (int i = 0; i<=5; i++) // Keep both loops separate.
      {
         const char* val = datfile.Find(tsHeader, tsOriginal[i].c_str());
         if (val)
         {
            tscrnFile.SetValue(tsHeader, tsOriginal[i].c_str(), val);
            datfile.RemoveLine(tsHeader, tsOriginal[i].c_str());
         }
         else
            cout << tsHeader << ":" << tsOriginal[i] << " not found " << std::endl;
      }
      datfile.RemoveLine(tsHeader);
   }
   else if ( datfile.Find(tsHeader) )
   {
      installLog << "Pre-v6.0 " << FILE_CAL_DAT << " file found.  Conversion needed\n";
      //////////////////////////////////////////////////////////////////////////////////
      //                 5.1/P-->6.0 changes
      //////////////////////////////////////////////////////////////////////////////////
      {
         tscrnFile.SetValue(tsHeader, "a", "0.842105263");
         tscrnFile.SetValue(tsHeader, "b", "0.0");
         tscrnFile.SetValue(tsHeader, "c", "-75.7894737");
         tscrnFile.SetValue(tsHeader, "d", "0.0");
         tscrnFile.SetValue(tsHeader, "e", "0.685714286");
         tscrnFile.SetValue(tsHeader, "f", "-89.1428571");
      }

      const char* val = 0;
      for (int i = 0; i<=5; i++)
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
      if ( stat((const char*)PNAME_TCHSCRNDAT, &fileStat) == OK )
      {
         installLog << "File " << FILE_TCHSCRN_DAT << " present with and up to date " << FILE_CAL_DAT << ". No conversion needed\n";
      }
      else
      {
         installLog << " ... pre-v5.1 " << FILE_CAL_DAT << " file found. Unable to Convert! ending...\n";
      }
      return;
   }

   tscrnFile.Write(PNAME_TCHSCRNDAT);
   datfile.Write(PNAME_CALDAT);

   installLog << FILE_CAL_DAT << " file converted.\n";
}

bool installer::extractUpdateFiles5 ()
{
   installLog << installLogStream::DEVELOPMENT << "Extracting Trima 5.X files\n";

   //
   // If we booted up using the default vxWorks (and not vxWorks.old) image, then
   //  save it for emergency cases ...
   //
   BOOT_PARAMS* params = new BOOT_PARAMS;

   bootStringToStruct(sysBootLine, params);
   bool defaultBootImage = ( strcmp(params->bootFile, "/ata0/vxWorks") == 0 );
   delete params;

   if ( defaultBootImage )
   {
      //
      // Save off the old vxWorks image in case of failure ...
      installLog << "Saving the old OS image...\n";
      attrib(VXBOOT_PATH "/vxWorks.old", "-R");
      if ( cp(VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.old") == ERROR )
      {
         installLog << "Archive of old OS image failed\n";
         return false;
      }
   }

   //
   // Store the new files in the proper position
   installLog << "Extracting the OS image...\n";
   if ( tarExtract(UPDATE_PATH "/vxboot.taz", UPDATE_PATH) == ERROR )
   {
      installLog << "Extraction of OS image failed\n";
      return false;
   }

   /* make the kernel images writeable so they can be overwritten */
   attrib(VXBOOT_PATH "/bootrom.sys", "-R");
   attrib(VXBOOT_PATH "/vxWorks", "-R");


   // check if we're installing a python-capable 5.X version
   struct stat fileStat;
   if ( stat((const char*)UPDATE_PATH "/vxWorks_python", &fileStat) == OK ||
        stat((const char*)UPDATE_PATH "/vxWorks_orig", &fileStat) == OK)
   {
      if (isVersalogicPython())
      {
         if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom.sys",  VXBOOT_PATH "/bootrom.sys") == ERROR ||
              updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_python", VXBOOT_PATH "/vxWorks") == ERROR )
         {
            installLog << "Install of Python OS image failed\n";
            return false;
         }
      }
      else
      {
         if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom.sys",  VXBOOT_PATH "/bootrom.sys") == ERROR ||
              updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_orig", VXBOOT_PATH "/vxWorks") == ERROR )
         {
            installLog << "Install of non-Python OS image failed\n";
            return false;
         }
      }

      if ( remove(UPDATE_PATH "/bootrom.sys") == ERROR ||
           remove(UPDATE_PATH "/vxWorks_orig") == ERROR ||
           remove(UPDATE_PATH "/vxWorks_python") == ERROR ||
           remove(UPDATE_PATH "/vxboot.taz") == ERROR )
      {
         installLog << "Removal of temporary OS image failed\n";
      }
   }
   else if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom.sys", VXBOOT_PATH "/bootrom.sys") == ERROR ||
             updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks", VXBOOT_PATH "/vxWorks") == ERROR )
   {
      installLog << "Install of OS image failed\n";
      return false;

      if ( remove(UPDATE_PATH "/bootrom.sys") == ERROR ||
           remove(UPDATE_PATH "/vxWorks") == ERROR ||
           remove(UPDATE_PATH "/vxboot.taz") == ERROR )
      {
         installLog << "Removal of temporary OS image failed\n";
      }
   }

/*
   if ( remove(UPDATE_PATH "/bootrom.sys") == ERROR ||
        remove(UPDATE_PATH "/vxWorks") == ERROR ||
        remove(UPDATE_PATH "/vxboot.taz") == ERROR )
   {
      installLog << "Removal of temporary OS image failed\n";
//      return false;
   }
*/

   //
   // Remove existing Trima files
   installLog << "Removing old Trima files...\n";

   // Remove any old fonts and strings
   installLog << "Removing any old fonts and string files\n";
   fileSort(STRING_DIRECTORY,  FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(DROP_IN_FONTS_DIR, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);

#if CPU!=SIMNT
   fileSort(TRIMA_PATH,    FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(SAVEDATA_PATH, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(TOOLS_PATH,    FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
#else
//   char cmdStr[256];
//   sprintf(cmdStr, "del /s /f /q %s\\*", TRIMA_PATH);
//   system(cmdStr);
//   sprintf(cmdStr, "del /s /f /q %s\\*", SAVEDATA_PATH);
//   system(cmdStr);
//   sprintf(cmdStr, "del /s /f /q %s\\*", TOOLS_PATH);
//   system(cmdStr);
#endif

   //
   // Uncompress the update file
   installLog << "Extracting the Trima software files...\n";
   if ( tarExtract(UPDATE_PATH "/trima.taz", TRIMA_PATH) == ERROR )
   {
      installLog << "Extraction of the Trima software failed.\n";
      return false;
   }

   if ( remove(UPDATE_PATH "/trima.taz") == ERROR )
   {
      installLog << "Removal of Trima archive image failed\n";
#if CPU!=SIMNT // this doesn't work on the simulator
      return false;
#endif
   }

   return true;

}

bool installer::extractUpdateFiles6 ()
{
   installLog << installLogStream::DEVELOPMENT << "Extracting Trima 6.X+ files\n";

   struct stat fileStat;

   //
   // If we booted up using the default vxWorks image, then
   //  save it for emergency cases ...
   //
   BOOT_PARAMS* params = new BOOT_PARAMS;

   bootStringToStruct(sysBootLine, params);
   bool defaultBootImage = ( strstr(params->bootFile, "/vxWorks") != NULL );
   delete params;

   if ( defaultBootImage )
   {
      //
      // Save off the old vxWorks image in case of failure ...
      installLog << "Saving the old OS image...\n";
      attrib(VXBOOT_PATH "/vxWorks.old", "-R");

      if ( cp(VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.old") == ERROR )
      {
         installLog << "Archive of old OS image failed\n";
      }

      // Remove the .bak file to save space for vxWorks 6.9
      installLog << "Removing vxWorks.bak\n";
      attrib(VXBOOT_PATH "/vxWorks.bak", "-R");

      struct stat bakfileStat;

      if ( stat((const char*)VXBOOT_PATH "/vxWorks.bak", &bakfileStat) == OK )
      {
         remove(VXBOOT_PATH "/vxWorks.bak");
      }

      // For development: if it exists, backup the kernel_init directory too
      if ( updatetrimaUtils::development_install &&
           stat((const char*)TRIMA_PATH "/kernel_init", &bakfileStat) == OK )
      {
         const char* backupDir = TEMP_PATH "/kernel_init.bak";
         attrib(TEMP_PATH, "-R");
         if ( mkdir((char*)backupDir) == OK )
         {
            installLog << "Backing up kernel_init ...\n";
            cp(TRIMA_PATH "/kernel_init/*.out", backupDir);
         }
      }
   }

   //
   // Store the new files in the proper position
   installLog << "Extracting the OS image from " << UPDATE_PATH << "/vxboot.taz\n";

   if ( tarExtract(UPDATE_PATH "/vxboot.taz", UPDATE_PATH) == ERROR )
   {
      installLog << "Extraction of OS image failed\n";
      return false;
   }

   if ( stat((const char*)VXBOOT_PATH "/bootrom.sys", &fileStat) == ERROR )
   {
      perror(VXBOOT_PATH "/bootrom.sys");
   }

   if ( stat((const char*)VXBOOT_PATH "/vxWorks", &fileStat) == ERROR )
   {
      perror(VXBOOT_PATH "/vxWorks");
   }

   //
   // Copy over the control images depending on the board type.
   //
   // Use Fox-specific images if present
   if ( isVersalogicFox() && stat((const char*)UPDATE_PATH "/vxWorks_fox", &fileStat) == OK )
   {
      installLog << "Copying Control Versalogic Fox bootrom.sys and vxWorks to " << VXBOOT_PATH << "\n";

      if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom_fox.sys", VXBOOT_PATH "/bootrom.sys") == ERROR ||
           updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_fox",     VXBOOT_PATH "/vxWorks") == ERROR  )
      {
         installLog << "Install of OS image failed\n";
         return false;
      }
   }
   else if ( isAmpro() )
   {
      installLog << "Copying Control Ampro bootrom.sys and vxworks to " << VXBOOT_PATH << "\n";

      if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom_ampro.sys", VXBOOT_PATH "/bootrom.sys") == ERROR ||
           updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_ampro", VXBOOT_PATH "/vxWorks") == ERROR  )
      {
         installLog << "Install of OS image failed\n";
         return false;
      }
   }
   else
   {
      installLog << "Copying Control Versalogic bootrom.sys and vxworks to " << VXBOOT_PATH << "\n";

      if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom_versalogic.sys", VXBOOT_PATH "/bootrom.sys") == ERROR ||
           updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_versalogic", VXBOOT_PATH "/vxWorks") == ERROR  )
      {
         installLog << "Install of OS image failed\n";
         return false;
      }
   }

   //
   // Clean up source files provided in the TAR file.  Note ampro files may or may not be
   // included in archive so we remove all and then check to make sure nothing is left over.
   //
   remove(UPDATE_PATH "/bootrom_ampro.sys");
   remove(UPDATE_PATH "/vxWorks_ampro");
   remove(UPDATE_PATH "/bootrom.sys");
   remove(UPDATE_PATH "/vxWorks");
   remove(UPDATE_PATH "/bootrom_versalogic.sys");
   remove(UPDATE_PATH "/vxWorks_versalogic");
   remove(UPDATE_PATH "/bootrom_fox.sys");
   remove(UPDATE_PATH "/vxWorks_fox");
   remove(UPDATE_PATH "/vxboot.taz");

   if ( stat((const char*)UPDATE_PATH "/bootrom_ampro.sys", &fileStat) != ERROR ||
        stat((const char*)UPDATE_PATH "/vxWorks_ampro", &fileStat) != ERROR ||
        stat((const char*)UPDATE_PATH "/bootrom.sys", &fileStat) != ERROR ||
        stat((const char*)UPDATE_PATH "/vxWorks", &fileStat) != ERROR ||
        stat((const char*)UPDATE_PATH "/bootrom_versalogic.sys", &fileStat) != ERROR ||
        stat((const char*)UPDATE_PATH "/vxWorks_versalogic", &fileStat) != ERROR ||
        stat((const char*)UPDATE_PATH "/bootrom_fox.sys", &fileStat) != ERROR ||
        stat((const char*)UPDATE_PATH "/vxWorks_fox", &fileStat) != ERROR ||
        stat((const char*)UPDATE_PATH "/vxboot.taz", &fileStat) != ERROR )
   {
      installLog << "Removal of temporary OS image failed\n";
      return false;
   }

   //
   // Remove existing Trima files
   installLog << "Removing old Trima files...\n";

   // Remove any old fonts and strings
   installLog << "Removing any old fonts and string files\n";
   fileSort(STRING_DIRECTORY,  FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(DROP_IN_FONTS_DIR, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);

#if CPU!=SIMNT
   fileSort(TRIMA_PATH,      FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_file_set_rdwrite);
   fileSort(TRIMA_PATH,      FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(SAVEDATA_PATH,   FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(TOOLS_PATH,      FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(STRING_DIRECTORY, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(GRAPHICS_PATH,   FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
#else

//   update_clean_file(TRIMA_PATH);
/*
   char cmdStr[256];
//   sprintf(cmdStr, "del /s /f /q %s", TRIMA_PATH);
   sprintf(cmdStr, "rmdir /S /Q %s", TRIMA_PATH);
   installLog << "Delete command: " << cmdStr << "\n";
   system(cmdStr);
   sprintf(cmdStr, "del /s /f /q %s\\*", SAVEDATA_PATH);
   system(cmdStr);
   sprintf(cmdStr, "del /s /f /q %s\\*", TOOLS_PATH);
   system(cmdStr);
   sprintf(cmdStr, "del /s /f /q %s\\*", STRING_DIRECTORY);
   system(cmdStr);
   sprintf(cmdStr, "del /s /f /q %s\\*", GRAPHICS_PATH);
   system(cmdStr);
*/
#endif

   //
   // Uncompress the update file
   installLog << "Extracting the Trima software files...\n";
   installLog << "Extracting the Trima software files from " << UPDATE_PATH << "/trima.taz to " << TRIMA_PATH << "\n";

   if ( tarExtract(UPDATE_PATH "/trima.taz", TRIMA_PATH) == ERROR )
   {
      installLog << "Extraction of the Trima software failed.\n";
      return false;
   }

   if ( remove(UPDATE_PATH "/trima.taz") == ERROR )
   {
      installLog << "Removal of Trima archive image failed\n";
      return false;
   }

   //
   // Uncompress the update file
   installLog << "Extracting the string.info files...\n";

   if ( tarExtract(UPDATE_PATH "/strings.taz", STRING_DIRECTORY) == ERROR )
   {
      installLog << "Extraction of the string.info files failed.\n";
      return false;
   }

   if ( remove(UPDATE_PATH "/strings.taz") == ERROR )
   {
      installLog << "Removal of string archive image failed\n";
      return false;
   }

   //
   // Uncompress the update file
   installLog << "Extracting the font files...\n";

   if ( tarExtract(UPDATE_PATH "/fonts.taz", DROP_IN_FONTS_DIR) == ERROR )
   {
      installLog << "Extraction of the font files failed.\n";
      return false;
   }

   if ( remove(UPDATE_PATH "/fonts.taz") == ERROR )
   {
      installLog << "Removal of font archive image failed\n";
      return false;
   }

   //
   // Uncompress the update file
   installLog << "Extracting the data files...\n";

   if ( tarExtract(UPDATE_PATH "/data.taz", DATA_DIRECTORY) == ERROR )
   {
      installLog << "Extraction of the data files failed.\n";
      return false;
   }

   if ( remove(UPDATE_PATH "/data.taz") == ERROR )
   {
      installLog << "Removal of data archive image failed\n";
      return false;
   }

   //
   // Uncompress the update file
   installLog << "Extracting the graphics files...\n";

   if ( tarExtract(UPDATE_PATH "/graphics.taz", GRAPHICS_PATH) == ERROR )
   {
      installLog << "Extraction of the graphics files failed.\n";
      return false;
   }

   if ( remove(UPDATE_PATH "/graphics.taz") == ERROR )
   {
      installLog << "Removal of graphics archive image failed\n";
      return false;
   }

   // Remove CommonKernel directories that will be installed by the extraction below
   // (otherwise, they are N/A for pre-ComonKernel configurations)
   xdelete(COMMON_KERNEL_INIT_DIR); // root of SAFETY_COMMON_KERNEL_INIT_PATH
   xdelete(COMMON_KERNEL_INIT_OLD); // this is obsolete

   // Look for Safety's Common Kernel boot files (vxboot_safety.taz)
   if ( stat((const char*)UPDATE_PATH "/vxboot_safety.taz", &fileStat) == OK )
   {
      installLog << "Extracting the safety vxboot files...\n";

      if (tarExtract(UPDATE_PATH "/vxboot_safety.taz", SAFETY_COMMON_KERNEL_INIT_PATH) == ERROR)
      {
         installLog << "Extraction of the safety vxboot files failed.\n";
      }

      if (remove(UPDATE_PATH "/vxboot_safety.taz") == ERROR)
      {
         installLog << "Removal of Tools archive image failed\n";
      }

      // One other special-case for Common Kernel: to compute the same CRC that
      // Safety computes for its kernel init module, we do what Control does
      // during init and copy Safety's module to /machine/tmp.
      attrib(SAFETY_KERNEL_INIT_TMP, "-R");

      if (cp(SAFETY_KERNEL_INIT_SRC, SAFETY_KERNEL_INIT_TMP) == ERROR)
      {
         installLog << "Temp copy of safety's kernel init file failed: " << SAFETY_KERNEL_INIT_SRC << "\n";
         return false;
      }
   }

   //
   // Copy over the safety images depending on the board type.
   //
   // If there are files in SAFETY_BOOT_PATH install Safety from there (pre-CommonKernel case)
   if ( stat((const char*)SAFETY_BOOT_PATH "/vxWorks_bengal", &fileStat) != ERROR ||
        stat((const char*)SAFETY_BOOT_PATH "/bootrom_ampro.sys", &fileStat) != ERROR ||
        stat((const char*)SAFETY_BOOT_PATH "/vxWorks_versalogic", &fileStat) != ERROR)
   {
      if ( isVersalogicFox() )
      {
         // Fox Control boards are paired with Bengal Safety boards (E-Box 2016).
         // Uses PXE server to transfer files.

         installLog << "Copying Safety Bengal bootrom.sys and vxWorks to " << SAFETY_BOOT_PATH << "\n";

         if ( cp(SAFETY_BOOT_PATH "/bootrom_bengal.pxe", SAFETY_BOOTROM_PXE_IMAGE) == ERROR ||
              cp(SAFETY_BOOT_PATH "/vxWorks_bengal",     SAFETY_VXWORKS_IMAGE)     == ERROR )
         {
            installLog << "Install of OS image failed\n";
            return false;
         }
      }
      else if ( isAmpro() )
      {
         installLog << "Copying Safety Ampro bootrom.sys and vxworks to " << SAFETY_BOOT_PATH << "\n";

         if ( cp(SAFETY_BOOT_PATH "/bootrom_ampro.sys", SAFETY_BOOTROM_IMAGE) == ERROR ||
              cp(SAFETY_BOOT_PATH "/vxWorks_ampro",     SAFETY_VXWORKS_IMAGE) == ERROR )
         {
            installLog << "Install of OS image failed\n";
            return false;
         }
      }
      else
      {
         installLog << "Copying Safety Versalogic bootrom.sys and vxworks to " << SAFETY_BOOT_PATH << "\n";

         if ( cp(SAFETY_BOOT_PATH "/vxWorks_versalogic",      SAFETY_VXWORKS_IMAGE)     == ERROR ||
              cp(SAFETY_BOOT_PATH "/bootrom_versa_bootp.sys", SAFETY_BOOTROM_IMAGE)     == ERROR ||
              cp(SAFETY_BOOT_PATH "/vxWorks_versalogic_pxe",  SAFETY_VXWORKS_PXE_IMAGE) == ERROR ||
              cp(SAFETY_BOOT_PATH "/bootrom_versa_pxe.sys",   SAFETY_BOOTROM_PXE_IMAGE) == ERROR )
         {
            installLog << "Install of OS image failed\n";
            return false;
         }
      }
   }
   else if ( stat((const char*)SAFETY_COMMON_KERNEL_INIT_PATH "/vxWorks", &fileStat) == ERROR )
//   else if ( attrib(SAFETY_COMMON_KERNEL_INIT_PATH "/vxWorks", "-R") == ERROR )
   {
      installLog << "No file found in " << SAFETY_COMMON_KERNEL_INIT_PATH << "\n";
      // If there aren't files in SAFETY_BOOT_PATH or SAFETY_KERNEL_INIT_PATH abort the install
      return false;
   }

   //
   //  These files may or may not exist depending on whether it is a Pentium or Ampro image,
   //  so remove without checking status and then make sure nothing remains.
   //

   remove(SAFETY_BOOT_PATH "/bootrom_ampro.sys");
   remove(SAFETY_BOOT_PATH "/vxWorks_ampro");
   remove(SAFETY_BOOT_PATH "/bootrom_versa_bootp.sys");
   remove(SAFETY_BOOT_PATH "/bootrom_versa_pxe.sys");
   remove(SAFETY_BOOT_PATH "/vxWorks_versalogic");
   remove(SAFETY_BOOT_PATH "/vxWorks_versalogic_pxe");
   remove(SAFETY_BOOT_PATH "/bootrom_bengal.pxe");
   remove(SAFETY_BOOT_PATH "/vxWorks_bengal");

   if ( stat((const char*)SAFETY_BOOT_PATH "/bootrom_ampro.sys", &fileStat) != ERROR ||
        stat((const char*)SAFETY_BOOT_PATH "/vxWorks_ampro", &fileStat) != ERROR ||
        stat((const char*)SAFETY_BOOT_PATH "/bootrom_versa_bootp.sys", &fileStat) != ERROR ||
        stat((const char*)SAFETY_BOOT_PATH "/bootrom_versa_pxe.sys", &fileStat) != ERROR ||
        stat((const char*)SAFETY_BOOT_PATH "/vxWorks_versalogic", &fileStat) != ERROR ||
        stat((const char*)SAFETY_BOOT_PATH "/vxWorks_versalogic_pxe", &fileStat) != ERROR ||
        stat((const char*)SAFETY_BOOT_PATH "/bootrom_bengal.pxe", &fileStat) != ERROR ||
        stat((const char*)SAFETY_BOOT_PATH "/vxWorks_bengal", &fileStat) != ERROR )
   {
      installLog << "removal of temporary OS image failed\n";
      return false;
   }


   // Uncompress the optional tools archive if it exists
   if ( stat((const char*)UPDATE_PATH "/engr_tools.taz", &fileStat) == OK )
   {
      installLog << "Extracting the engr tools files...\n";

      if (tarExtract(UPDATE_PATH "/engr_tools.taz", ROOT "/machine/tools") == ERROR)
      {
         installLog << "Extraction of the Tools files failed.\n";
      }

      if (remove(UPDATE_PATH "/engr_tools.taz") == ERROR )
      {
         installLog << "Removal of Tools archive image failed\n";
      }
   }

   return true;
}

bool installer::checkCRC5 ()
{

   // Set permissions in config directory
#if CPU!=SIMNT
   fileSort(CONFIG_PATH, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_file_set_rdwrite);
#else
   char cmdStrAttr[256];
   sprintf(cmdStrAttr, "attrib -R /S /D %s\\*", CONFIG_PATH);
   system(cmdStrAttr);
#endif

   // Remove FEATURES files since it doesn't exist in 5.X
   struct stat featuresFileStat;
   if ( stat((const char*)FILELISTS_PATH "/features.files", &featuresFileStat) == OK )
   {
      remove(FILELISTS_PATH "/features.files");
   }

   struct stat featurescrcFileStat;
   if ( stat((const char*)CONFIG_CRC_PATH "/features.crc", &featurescrcFileStat) == OK )
   {
      remove(CONFIG_CRC_PATH  "/features.crc");
   }

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
      installLog << "CRC check of installed software failed\n";
      return false;
   }

   return true;
}

bool installer::checkCRC6 ()
{
   //
   // Update configuration CRC values
   mkdir(CONFIG_CRC_PATH);

   softcrc("-filelist " FILELISTS_PATH "/caldat.files              -update " CONFIG_CRC_PATH  "/caldat.crc");
   softcrc("-filelist " FILELISTS_PATH "/config.files              -update " CONFIG_CRC_PATH  "/config.crc");
   softcrc("-filelist " FILELISTS_PATH "/hwdat.files               -update " CONFIG_CRC_PATH  "/hwdat.crc");
   softcrc("-filelist " FILELISTS_PATH "/rbcdat.files              -update " CONFIG_CRC_PATH  "/rbcdat.crc");
   softcrc("-filelist " FILELISTS_PATH "/cassette.files            -update " CONFIG_CRC_PATH  "/cassette.crc");
   softcrc("-filelist " FILELISTS_PATH "/setconfig.files           -update " CONFIG_CRC_PATH  "/setconfig.crc");
   softcrc("-filelist " FILELISTS_PATH "/graphics.files            -update " PNAME_GUI_GRAPHICS_CRC);
   softcrc("-filelist " FILELISTS_PATH "/strings.files             -update " PNAME_STRING_CRC);
   softcrc("-filelist " FILELISTS_PATH "/fonts.files               -update " PNAME_FONT_CRC);
   softcrc("-filelist " FILELISTS_PATH "/data.files                -update " PNAME_DATA_CRC);
   softcrc("-filelist " FILELISTS_PATH "/machine.files             -update " CONFIG_CRC_PATH "/machine.crc");

   // Separate check for TERROR since the file may or may not exist
   const int terrorExists = open(FILELISTS_PATH "/terrordat.files",  O_RDONLY, DEFAULT_FILE_PERM);

   if (terrorExists > 0)
   {
      close(terrorExists);
      softcrc("-filelist " FILELISTS_PATH "/terrordat.files -update " CONFIG_CRC_PATH  "/terrordat.crc");
   }

   // Separate check for FEATURES since the file may or may not exist
   const int featuresExists = open(FILELISTS_PATH "/features.files",  O_RDONLY, DEFAULT_FILE_PERM);

   if (featuresExists > 0)
   {
      close(featuresExists);
      softcrc("-filelist " FILELISTS_PATH "/features.files -update " CONFIG_CRC_PATH  "/features.crc");
   }

   // Separate check for Barcode Categories since the file may or may not exist
   const int barcodeExists = open(FILELISTS_PATH "/barcode_categories.files",  O_RDONLY, DEFAULT_FILE_PERM);
   if (barcodeExists > 0)
   {
      close(barcodeExists);
      softcrc("-filelist " FILELISTS_PATH "/barcode_categories.files -update " CONFIG_CRC_PATH  "/barcode_categories.crc");
   }

   // Separate check for RTS config since the file may or may not exist
   const int rtsConfigExists = open(FILELISTS_PATH "/rts_config.files",  O_RDONLY, DEFAULT_FILE_PERM);
   if (rtsConfigExists > 0)
   {
      close(rtsConfigExists);
      softcrc("-filelist " FILELISTS_PATH "/rts_config.files -update " CONFIG_CRC_PATH  "/rts_config.crc");
   }

   const int appServerExists = open(FILELISTS_PATH "/app_server.files",  O_RDONLY, DEFAULT_FILE_PERM);
   if (appServerExists > 0)
   {
      close(appServerExists);
      softcrc("-filelist " FILELISTS_PATH "/app_server.files -update " CONFIG_CRC_PATH  "/app_server.crc");
   }

   // Set permissions in config directory
   updatetrimaUtils::update_file_set_rdonly(CONFIG_PATH);

   // Verify the installation CRC values
   if (verifyCrc("-filelist " FILELISTS_PATH "/caldat.files              -verify " CONFIG_CRC_PATH "/caldat.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/config.files              -verify " CONFIG_CRC_PATH "/config.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/hwdat.files               -verify " CONFIG_CRC_PATH "/hwdat.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/rbcdat.files	             -verify "CONFIG_CRC_PATH"/rbcdat.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/cassette.files            -verify " CONFIG_CRC_PATH "/cassette.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/setconfig.files           -verify " CONFIG_CRC_PATH "/setconfig.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/graphics.files            -verify " PNAME_GUI_GRAPHICS_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/strings.files             -verify " PNAME_STRING_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/fonts.files               -verify " PNAME_FONT_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/data.files                -verify " PNAME_DATA_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/safety.files              -verify " TRIMA_PATH "/safety.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/trima.files               -verify " TRIMA_PATH "/trima.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/machine.files             -verify " CONFIG_CRC_PATH "/machine.crc"))
   {
      return false;
   }

   // Separate check for TERROR since the file may or may not exist
   if ( (terrorExists > 0) && verifyCrc("-filelist " FILELISTS_PATH "/terrordat.files	-verify "CONFIG_CRC_PATH"/terrordat.crc") )
   {
      return false;
   }

   // Separate check for FEATURES since the file may or may not exist
   if ( (featuresExists > 0) && verifyCrc("-filelist " FILELISTS_PATH "/features.files	-verify "CONFIG_CRC_PATH"/features.crc") )
   {
      return false;
   }

   // Separate check for Barcode Categories since the file may or may not exist
   if ( (barcodeExists > 0) && verifyCrc("-filelist " FILELISTS_PATH "/barcode_categories.files	-verify "CONFIG_CRC_PATH "/barcode_categories.crc") )
   {
      return false;
   }

   // Separate check for RTS Config since the file may or may not exist
   if ( (rtsConfigExists > 0) && verifyCrc("-filelist " FILELISTS_PATH "/rts_config.files	-verify "CONFIG_CRC_PATH"/rts_config.crc") )
   {
      return false;
   }

   // Separate check for app server
   if ( (appServerExists > 0) && verifyCrc("-filelist " FILELISTS_PATH "/app_server.files	-verify "CONFIG_CRC_PATH"/app_server.crc") )
   {
      return false;
   }

   return true;
}

bool installer::checkPasSettings ()
{
   bool returnVal = false;

   CDatFileReader datfile(PNAME_CONFIGDAT);
   if ( datfile.Error() )
   {
      installLog << "Config file read error : " << datfile.Error() << "\n";
      return false;
   }

   for (int prodNum = 0; prodNum < NUM_YIELD_VOLUME; prodNum++)
   {
      // Variable name string
      char varNameStr[64];

      // Find the PAS toggle setting
      sprintf (varNameStr, "key_plt_mss_%d", prodNum + 1);

      const char* pasOnStr = datfile.Find("PRODUCT_TEMPLATES", varNameStr);

      // If no PAS toggle setting exists, or the setting is off, then we're presumably safe
      // from the condition we're looking for.
      if (!pasOnStr || !atoi(pasOnStr))
      {
         continue;
      }

      // The setting exists and is on, so see if we violate conditions.
      // Start by looking up percent plasma
      sprintf (varNameStr, "key_plt_pct_carryover_%d", prodNum + 1);
      const char* percentPlasmaStr = datfile.Find("PRODUCT_TEMPLATES", varNameStr);
      float       percentPlasma    = ((percentPlasmaStr) ? atof(percentPlasmaStr) : 0.0f);

      // Now find collect vol setting
      char pltVolVarNameStr[64];
      sprintf (pltVolVarNameStr, "key_plt_volume_%d", prodNum + 1);
      const char* collectVolStr = datfile.Find("PRODUCT_TEMPLATES", pltVolVarNameStr);
      float       collectVol    = ((collectVolStr) ? atof(collectVolStr) : 0.0f);

      // If one of these settings was 0, something's wierd, so try the next one.
      if (percentPlasma == 0.0f || collectVol == 0.0f)
      {
//         printf("Hmm... for PLT%d, percent plasma is %f, and collect vol is %f. Moving on...\n", prodNum + 1, percentPlasma, collectVol);
         installLog << "Hmm... for PLT" << prodNum + 1;
         installLog << " percent plasma is " << percentPlasma;
         installLog << " and collect vol is " << collectVol;
         installLog << ". Moving on...\n";
         continue;
      }

      // Figure out the max bag volume.
      float collectVolMax = floor(1200.0f * (percentPlasma / 100.0f));

      if (collectVol > collectVolMax)
      {
//         printf("PLT%d has collect vol %f greater than max %f. Adjusting accordingly.\n", prodNum + 1, collectVol, collectVolMax);
         installLog << "PLT" << prodNum + 1;
         installLog << " has collect vol " << collectVol;
         installLog << " greater than max " << collectVolMax;
         installLog << ". Adjusting accordingly.\n";

         datfile.SetFloat("PRODUCT_TEMPLATES", pltVolVarNameStr, collectVolMax);
         returnVal = true;
      }
   }

   if ( returnVal )
   {
      datfile.WriteCfgFile(FILE_CONFIG_DAT);
   }

   return returnVal;
}


bool installer::checkRasSettings ()
{
   bool returnVal     = false;
   bool drbcAlertOn   = false;
   int  drbcThreshold = 275;

   CDatFileReader datfile(PNAME_CONFIGDAT);
   if ( datfile.Error() )
   {
      installLog << "Config file read error : " << datfile.Error() << "\n";
      return false;
   }

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

      int dose   = 0;
      int rasVol = 0;

      // Find the PAS toggle setting
      sprintf (rasVolVarNameStr, "key_rbc_mss_volume_%d", prodNum + 1);
      sprintf (rbcDoseVarNameStr, "key_rbc_dose_%d", prodNum + 1);

      const char* rasVolStr  = datfile.Find("PRODUCT_TEMPLATES", rasVolVarNameStr);
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
         datfile.SetFloat("PRODUCT_TEMPLATES", rasVolVarNameStr, 200);
         returnVal = true;
      }

      if (dose < drbcThreshold && rasVol < 80)
      {
         datfile.SetFloat("PRODUCT_TEMPLATES", rasVolVarNameStr, 100);
         returnVal = true;
      }
   }

   if ( returnVal )
   {
      datfile.WriteCfgFile(FILE_CONFIG_DAT);
   }

   return returnVal;
}


void installer::forceSetConfig ()
{
//   if ( !development_only )
//   {
   if ( cp(TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT) == ERROR )
   {
//      printf("copy of %s failed\n", FILE_SETCONFIG_DAT);
      installLog << "copy of " << FILE_SETCONFIG_DAT << " failed\n";
      return;
   }

   attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
   fflush(stdout);
//   }
}

bool installer::checkRange (const char* section, const char* key, const char* value, std::string& forceVal)
{
   // These are used for logging
//   const char* rangeTypeStr[]    = {"V510", "V520", "V600", "V630", "V640", "V700", "END"};
//   const char* compareTypeStr[]  = {"MIN", "MAX", "NOT", "FORCE"};
//   const char* rangeValTypeStr[] = {"INT", "FLOAT"};

//   installLog << installLogStream::DEBUG << "Checking range of: " << section << " " << key << " " << value << "\n";

   const char* bloodTypeKey         = "key_blood_type_";
   bool        retval               = true;
   bool        foundInRangeData     = false;
   bool        everFoundInRangeData = false;
   rangeStruct rangeData;

   // clear out the forceVal string and populate it later, if needed
//   forceVal.clear();
   forceVal.resize(0);

   rangeData = updatetrimaUtils::getNextRangeData(true);

//   int rngCtr = 0;

   while ( rangeData.rangeType != END )
   {
      if ( rangeData.rangeType == newBuildData.rangeType)
      {
         if (strcmp(rangeData.section, section) == 0 &&
             strcmp(rangeData.dataKey, key) == 0)
         {
            installLog << " Found exact match rangeData:"
                       << " " << rangeTypeStr[(int)(rangeData.rangeType)]
                       << " " << rangeData.section
                       << " " << rangeData.dataKey;

            foundInRangeData     = true;
            everFoundInRangeData = true;
         }
         else if (strlen(key) - 1 == strlen(rangeData.dataKey) &&
                  strncmp(rangeData.dataKey, key, strlen(key) - 1) == 0 &&
                  strcmp(section, "PRODUCT_DEFINITIONS") == 0)
         {
            installLog << " Found partial match rangeData:"
                       << " " << rangeTypeStr[(int)(rangeData.rangeType)]
                       << " " << rangeData.section
                       << " " << rangeData.dataKey;

            foundInRangeData     = true;
            everFoundInRangeData = true;
         }
      }
      if ( foundInRangeData )
      {
         installLog << " Compare data:"
                    << " compareType: " << compareTypeStr[(int)(rangeData.compareType)]
                    << " valueType: " << rangeValTypeStr[(int)(rangeData.valType)]
                    << " value: " << rangeData.value;

         if ( rangeData.compareType == FORCE )
         {
//            installLog << installLogStream::DEBUG << "Forcing update\n";
            retval = false;
         }
         else if ( rangeData.valType == INT )
         {
            int intvalue = atoi(value);

//            installLog << installLogStream::DEBUG << "Comparing INT " << intvalue;

            if ( rangeData.compareType == MIN )
            {
//               installLog << installLogStream::DEBUG << " MIN vs " << rangeData.value;

               if ( intvalue < atoi(rangeData.value) )
               {
//                  installLog << installLogStream::DEBUG << " FAILED\n";
                  retval = false;
               }
//               installLog << installLogStream::DEBUG << " PASSED\n";
            }
            else if ( rangeData.compareType == MAX )
            {
//               installLog << installLogStream::DEBUG << " MAX vs " << rangeData.value;

               if ( intvalue > atoi(rangeData.value) )
               {
//                  installLog << installLogStream::DEBUG << " FAILED\n";
                  retval = false;
               }
//               installLog << installLogStream::DEBUG << " PASSED\n";
            }
            else if ( rangeData.compareType == NOT )
            {
//               installLog << installLogStream::DEBUG << " NOT vs " << rangeData.value;

               if ( intvalue == atoi(rangeData.value) )
               {
//                  installLog << installLogStream::DEBUG << " FAILED\n";
                  retval = false;
               }
//               installLog << installLogStream::DEBUG << " PASSED\n";
            }
         }
         else    // it must be a float
         {
            float floatvalue = atof(value);

            if ( rangeData.compareType == MIN )
            {
               if ( floatvalue < atof(rangeData.value) )
               {
                  retval = false;
               }
            }
            else if ( rangeData.compareType == MAX )
            {
               if ( floatvalue > atof(rangeData.value) )
               {
                  retval = false;
               }
            }
         }

         installLog << " " << (retval ? "PASSED; " : "FAILED") << "\n";

         // if it failed then we can stop checking
         if (retval == false)
         {
            break;
         }

         foundInRangeData = false;
      }
      rangeData = updatetrimaUtils::getNextRangeData();

//      rngCtr++;
   }

   if ( everFoundInRangeData )
   {
      if (retval)
      {
         installLog << " Found in range data, PASSED all checks\n";
      }
      else
      {
         installLog << " Found in range data, FAILED a check";

         if (rangeData.useLimit)
         {
            installLog << " - forcing value\n";
            forceVal = rangeData.value;
         }
         else if ( strcmp(rangeData.dataKey, bloodTypeKey) == 0
                   && rangeData.compareType == MAX )
         {
            // if the failure was a blood type max range, try to convert it, otherwise let it use the default
            std::string newVal;
            if ( bloodTypeConversionCheck(atoi(value), newVal) )
            {
               forceVal = newVal;
               installLog << " - converting blood type\n";
            }
            else
            {
               installLog << " - using default blood type\n";
            }
         }
         else
         {
            installLog << " - using default\n";
         }
      }
//      if (!retval && rangeData.useLimit)
//      {
//         forceVal = rangeData.value;
//      }
   }
   else
   {
      installLog << " Not found in range data, PASSED by default\n";
   }

   return retval;
}

// Check if the blood type value in the new format corresponds to one of the old ones
// if so, return true and the old value that corresponds to it
bool installer::bloodTypeConversionCheck (int bloodType, std::string& newVal)
{
   bool retval    = true;
   int  newIntVal = 0;
   char charVal[4];

   // the offset of the new blood types from the old ones
   // can't use the value from bloodtypes.h because it doesn't exist in all versions of the file
   static const int Offset = 13;

   switch (bloodType)
   {
      case BT_ALL + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_ALL;
         break;
      case BT_OP_ON + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_OP_ON;
         break;
      case BT_ON + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_ON;
         break;
      case BT_OP_ON_AP_AN + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_OP_ON_AP_AN;
         break;
      case BT_OP_ON_AN + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_OP_ON_AN;
         break;
      case BT_ON_AN + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_ON_AN;
         break;
      case BT_OP_ON_BP_BN + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_OP_ON_BP_BN;
         break;
      case BT_OP_ON_AP_AN_BP_BN + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_OP_ON_AP_AN_BP_BN;
         break;
      case BT_OP_ON_AP_AN_BN + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_OP_ON_AP_AN_BN;
         break;
      case BT_AP_AN + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_AP_AN;
         break;
      case BT_AP_AN_BP_BN_ABP_ABN + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_AP_AN_BP_BN_ABP_ABN;
         break;
      case BT_BP_BN + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_BP_BN;
         break;
      case BT_ABP_ABN + Offset :
         newIntVal = CONFIG_INDEX_BLOOD_TYPE_ABP_ABN;
         break;
      default :
         retval = false;
   }

   if ( retval )
   {
      sprintf(charVal, "%d", newIntVal);
      newVal = charVal;
   }

   return retval;
}


bool installer::checkPlasmaRB ()
{
   bool        retval = false;
   const char* configData;
   const char* swData;

   // Get the rinseback setting from sw.dat
   swData = findSetting("allow_plasma_rinseback=", PNAME_SWDAT);

//   if (swData) installLog << "sw.dat allow_plasma_rinseback = " << swData << "\n";

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
            installLog << "Can't open config.dat to modify key_plasma_rinseback\n";
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

bool installer::allowedUpgrade (bool ampro, bool versalogic, bool python, bool ocelot)
{
   bool retval = true;

   // possible change
/*
   if ( isVersalogicPython() )
   {
      if ( !python )
      {
      installLog << "This update path is not allowed on a VersaLogic Python Trima.\n";
      retval = false;
      }
   }
   if ( isVersalogicVSBC6() )
   {
      if ( !versalogic )
      {
      installLog << "This update path is not allowed on a VersaLogic Trima.\n";
      retval = false;
      }
   }
   if ( isAmpro() )
   {
      if ( !ampro )
      {
      installLog << "This update path is not allowed on an Ampro Trima.\n";
      retval = false;
      }
   }
*/
   if ( isVersalogicPython() && !python )
   {
      installLog << "This update path is not allowed on a VersaLogic Python Trima.\n";
      retval = false;
   }
   else
   {
      installLog << "This update path is allowed.\n";
   }

   return retval;
}


bool installer::updateConfigGeneric ()
{
   bool           retval       = true;
   int            readState    = 0;
   FILE*          configFilefp = NULL;
   CDatFileReader newdatfile;

   // the line read from the existing config.dat
   CFileLine cfLine;

   // the section header from the existing config.dat
   char section[256];
   section[255] = 0;

   // the value from the existing config.dat that might be the "new" value in the new config.dat
   char newVal[256];
   newVal[255] = 0;

   // a temp string to store the value from the new config.dat for comparisons
   std::string strVal = "";

   // clean up
   attrib(PNAME_CONFIGDAT ".new", "-R");
   remove(PNAME_CONFIGDAT ".new");

////////////////////
// #define JUST_LOOKING

#ifdef JUST_LOOKING // open the template/config.dat and don't change anything
   // open the template/config.dat
   newdatfile.Initialize(TEMPLATES_PATH "/" FILE_CONFIG_DAT);
   if ( newdatfile.Error() )
   {
      installLog << "Config file read error : " << templatedatfile.Error() << "\n";
      retval = false;
      goto LEAVEupdateConfigGeneric;
   }
#else // not JUST_LOOKING do the normal processing
      // copy the template config.dat to /config and make it writable
   if ( cp(TEMPLATES_PATH "/" FILE_CONFIG_DAT, PNAME_CONFIGDAT ".new") != ERROR )
   {
      attrib(PNAME_CONFIGDAT ".new", "-R");

      // open the new config.dat
      newdatfile.Initialize(PNAME_CONFIGDAT ".new");
      if ( newdatfile.Error() )
      {
         installLog << "Config file read error : " << newdatfile.Error() << "\n";
         retval = false;
         goto LEAVEupdateConfigGeneric;
      }
   }
   else
   {
      installLog << "copy of config.dat template failed\n";
      retval = false;
      goto LEAVEupdateConfigGeneric;
   }
#endif // JUST_LOOKING

   // open the old config.dat for reading as a normal file because CDatFileReader
   // doesn't support reading of every line without lookup
   if ((configFilefp = fopen(PNAME_CONFIGDAT, "r")) == NULL )
   {
      retval = false;
      goto LEAVEupdateConfigGeneric;
   }

   while ( (readState = readDatFileVals(cfLine, configFilefp)) > 0 )
   {
      if ( cfLine.IsSection() )
      {
         strcpy(section, cfLine.cpName());
         continue;
      }

      // if we have a section, key, and value then
      // look up the key in the new config file, use the last section reported
      // then compare the value and change it if it's different
      if ( strlen(section) > 0 && newdatfile.Find(section, cfLine.cpName()) )
      {
         strcpy(newVal, cfLine.cpValue());

//         installLog << installLogStream::DEBUG << "Checking: " << section << " " << cfLine.cpName() << " " << newVal << "\n";

         // get the value from the new config.dat
         strVal = newdatfile.GetString(section, cfLine.cpName());
//         installLog << installLogStream::DEBUG << section << " " << cfLine.cpName() << " " << "Existing val: " << newVal << " Default val: " << strVal << "\n";

         // compare the values and check if the old value passes the range check for the new config.dat
         if ( strVal.compare(newVal) != 0 )
         {
            installLog << "Checking the range of value: " << section << " " << cfLine.cpName() << " " << newVal << " Default val: " << strVal << "\n";

            std::string forceVal;

            if ( checkRange(section, cfLine.cpName(), newVal, forceVal) )
#ifdef JUST_LOOKING
            {
               installLog << "Passed\n";
            }
            else
            {
               installLog << "Failed\n";
            }
#else       // not JUST_LOOKING
            {
               // When upgrading from Trima 6.4 to Trima 7 the Max duration allowed
               // should be set to default value of 150 and not to be transferred
               // from previous Trima 6.4 build
               std::string parameter("key_proc_time");
               int         result = strncmp(parameter.c_str(), cfLine.cpName(), parameter.size());

               if (!is64To7Upgrade || (0 != result))
               {
                  newdatfile.SetValue(section, cfLine.cpName(), newVal);
                  installLog << " Transferring setting for: " << section << " " << cfLine.cpName() << ", setting to: " << newVal << "\n";
               }
               else
               {
                  newdatfile.SetValue(section, cfLine.cpName(), strVal.c_str());
                  installLog << " Setting default value for: " << section << " " << cfLine.cpName() << " to: " << strVal << "\n";
               }
            }
            else
            {
               installLog << " Range check failed for: " << section << " " << cfLine.cpName() << " " << newVal;

               // if the compare failed and a forced value is provided, use it
               if (forceVal.size() > 0)
               {
                  newdatfile.SetValue(section, cfLine.cpName(), forceVal.c_str());
                  installLog << ", using forced value of " << forceVal.c_str() << "\n";
               }
               else
               {
                  installLog << ", using default value of " << strVal.c_str() << "\n";
               }
            }
#endif      // JUST_LOOKING
         }
      }

   }
//    cerr << endl;

   fclose(configFilefp);

////////////////////////////////////////

#ifndef JUST_LOOKING
   // if it ended without error, write the data and copy it over the old config.dat
   if ( readState >= 0 )
   {
      // write the modified config.dat.new
      if ( !newdatfile.WriteCfgFile(FILE_CONFIG_DAT ".new") )
      {
         installLog << "config.dat.new write failed\n";
         retval = false;
         goto LEAVEupdateConfigGeneric;
      }
      else
      {
         installLog << "config.dat.new write worked\n";
      }

      // the dat file reader can read a file from a specific path but will only write it to /config
      // so for the sim we have to move the file to where we want it
#if CPU==SIMNT
      installLog << " Moving config.dat.new for the simulator\n";
      remove(PNAME_CONFIGDAT ".new");
      cp("config/" FILE_CONFIG_DAT ".new", PNAME_CONFIGDAT ".new");
      remove("config/" FILE_CONFIG_DAT ".new");
      remove("config/" FILE_CONFIG_DAT ".new.bk");
#endif

      ///////////////////////////////////
      // copy config.dat.new over config.dat
      attrib(PNAME_CONFIGDAT, "-R");
      if ( cp(PNAME_CONFIGDAT ".new", PNAME_CONFIGDAT) == ERROR )
      {
         installLog << "copy of " << PNAME_CONFIGDAT << ".new failed.\n";
         retval = false;
         goto LEAVEupdateConfigGeneric;
      }
      else
      {
         // clean up
         attrib(PNAME_CONFIGDAT ".new", "-R");
         remove(PNAME_CONFIGDAT ".new");
         attrib(PNAME_CONFIGDAT ".new.bk", "-R");
         remove(PNAME_CONFIGDAT ".new.bk");
      }
   }
   else
   {
      retval = false;
      goto LEAVEupdateConfigGeneric;
   }
#endif // JUST_LOOKING

LEAVEupdateConfigGeneric:

   attrib(PNAME_CONFIGDAT, "+R");

   return retval;

}


int installer::readDatFileVals (CFileLine& cfLine, FILE* fp)
{
// return 1 if found a line, 0 if hit the end of file, -1 if something went wrong

   int  retval = 0;
   char buffer[256];
   buffer[255] = 0;
   bool found = false;

   while ( !found && fgets(buffer, sizeof(buffer), fp) )
   {
      CFileLine newLine(buffer);

      CFileLine::LINETYPE newType = newLine.Type();

//        fprintf(debugfp, "%s\n", newLine.cpName());

      if ( newType == CFileLine::LT_ERROR)
      {
         retval = -1;
         break;
      }
      else if (newType == CFileLine::LT_COMMENT)
      {
         continue;
      }

      cfLine = newLine;

      found = true;
   }

LEAVEROUTINE:

   // found == true, all good, retval = 1
   // found == false && feof, all good, retval = 0
   // found == false && !feof, bad, retval = -1
   if ( !found && retval == 0 )
   {
      if ( !feof(fp) )
      {
         retval = -1;
      }
      else
      {
         retval = 0;
      }
   }
   else
   {
      retval = 1;
   }

   return retval;
}

bool installer::installMachineId ()
{
   struct stat tempFileStat;
   struct stat configFileStat;
   int         tempFileStatStatus   = OK;
   int         configFileStatStatus = OK;

   bool result = true;

   tempFileStatStatus   = stat(const_cast<const char*>(TEMPLATES_PATH "/" FILE_MACHINE_ID), &tempFileStat);
   configFileStatStatus = stat(const_cast<const char*>(PNAME_MACHINE_ID), &configFileStat);

   // just for logging
   if ( tempFileStatStatus == OK )
   {
      installLog << "machine.id exists in templates\n";
   }
   else
   {
      installLog << "machine.id does not exist in templates\n";
   }

   if ( configFileStatStatus == OK )
   {
      installLog << "machine.id exists in config\n";
   }
   else
   {
      installLog << "machine.id does not exist in config\n";
   }

   // if there's a machine.id in temp and there isn't one in config, install it
//   if ( ( stat(const_cast<char*>(TEMPLATES_PATH "/" FILE_MACHINE_ID), &tempFileStat) == OK ) &&
//        ( stat(const_cast<char*>(PNAME_MACHINE_ID), &configFileStat) != OK ))
   if ( ( tempFileStatStatus == OK ) && ( configFileStatStatus != OK ) )
   {
      installLog << "Installing default machine.id\n";

      // append the serial number and copy the file
      if ( appendSerialNumToZipFile(TEMPLATES_PATH "/" FILE_MACHINE_ID) )
      {
         if ( cp(TEMPLATES_PATH "/" FILE_MACHINE_ID, PNAME_MACHINE_ID) == ERROR )
         {
            installLog << "Copy of default machine.id failed\n";
            result = false;
         }
      }
      else
      {
         installLog << "Appending serial number failed so copy of default machine.id failed\n";
         result = false;
      }
   }
   else
   {
      installLog << "Install of machine.id not needed\n";
   }

   return result;
}


//////////////////////////////////////////////////////////////////////////////////////
//  The main line of update
//////////////////////////////////////////////////////////////////////////////////////
int installer::upgrade (versionStruct& fromVer, versionStruct& toVer)
{
   bool retval = 0;

   // get the info for the previous build
//   updatetrimaUtils::getBuildInfo(fromVer, prevBuildRef);
   if (!updatetrimaUtils::getBuildInfo(fromVer, prevBuildRef))
   {
      installLog << "Previous build info not found - aborting install\n";
      return(-1);
   }
   installLog << "Previous build info: " << prevBuildRef << "\n";

   // record it for later use
//   prevBuildData = updatetrimaUtils::getBuildData(prevBuildRef);

   // get the info for the new build
   if (!updatetrimaUtils::getBuildInfo(toVer, newBuildRef))
   {
      installLog << "Build info not found - aborting install\n";
      return(-1);
   }
   installLog << "Build info: " << newBuildRef << "\n";

   // record it for later use
   newBuildData = updatetrimaUtils::getBuildData(newBuildRef);

   installLog << "Build best match found for the version given: " << newBuildData.buildNum << "\n";

   // check if this upgrade is allowed
   if ( !allowedUpgrade(newBuildData.ampro, newBuildData.versalogic, newBuildData.python, newBuildData.ocelot) )
   {
      installLog << "Upgrade to this version not allowed on this hardware\n";
      return(-1);
   }
   installLog << "Upgrade to this version is allowed on this hardware\n";

   // simulated control board
#if CPU==SIMNT
   installLog << "Simulated Control board is: ";
   if (isAmpro())
      installLog << "Ampro\n";
   else if (isVersalogicVSBC6())
      installLog << "Versalogic\n";
   else if (isVersalogicPython())
      installLog << "Python\n";
   else
      installLog << "UNKNOWN\n";
#endif

   // set the files read-write
#if CPU!=SIMNT
   updatetrimaUtils::update_file_set_rdwrite(CONFIG_PATH);
#else
   char cmdStrAttr[256];
   sprintf(cmdStrAttr, "attrib -R /S /D %s\\*", CONFIG_PATH);
   system(cmdStrAttr);
#endif

   //
   // Make /vxboot and /trima partitions writable
#if CPU!=SIMNT
   DOS_VOLUME_DESC_ID pVolDesc;

   const char* vxboot = VXBOOT_PATH;
   pVolDesc = dosFsVolDescGet((void*)vxboot, NULL);
   if ( !pVolDesc )
   {
      perror(VXBOOT_PATH);
      return(-1);
   }

#if (_WRS_VXWORKS_MAJOR < 6)
   cbioModeSet(pVolDesc->pCbio, O_RDWR);  // for VxWorks 5.5
#else
   pVolDesc->readOnly = 0;
#endif

   const char* trima = TRIMA_PATH;
   pVolDesc = dosFsVolDescGet((void*)trima, NULL);
   if ( !pVolDesc )
   {
      perror(TRIMA_PATH);
      return(-1);
   }

#if (_WRS_VXWORKS_MAJOR < 6)
   cbioModeSet(pVolDesc->pCbio, O_RDWR);  // for VxWorks 5.5
#else
   pVolDesc->readOnly = 0;
#endif

   const char* config = CONFIG_PATH;
   pVolDesc = dosFsVolDescGet((void*)config, NULL);
   if ( !pVolDesc )
   {
      perror(CONFIG_PATH);
      return(-1);
   }

#if (_WRS_VXWORKS_MAJOR < 6)
   cbioModeSet(pVolDesc->pCbio, O_RDWR);  // for VxWorks 5.5
#else
   pVolDesc->readOnly = 0;
#endif
#endif // SIMNT

   // If this is an upgrade from v6.4 to v7, set the flag
   if (newBuildData.rangeType >= V700 &&
       (fromVer.majorRev == 12 && fromVer.minorRev == 4))
   {
      is64To7Upgrade = true;
   }

   // if the version is 7.0 or greater, we need the machine/tools directory
   if (newBuildData.rangeType >= V700)
   {
      mkdir(TOOLS_PATH);
   }

   // Remove any old copy of features.bin in templates
   installLog << "Removing any old copy of features.bin in templates\n";

   struct stat featuresFileStat;

   if ( stat((const char*)TEMPLATES_PATH "/" FILE_FEATURES, &featuresFileStat) == OK )
   {
      remove(TEMPLATES_PATH "/" FILE_FEATURES);
   }

   installLog << "Extract the Trima files\n";
   // extract the Trima files
   if ( newBuildData.extractType == 5 )
   {
      retval = extractUpdateFiles5();
   }
   else
   {
      retval = extractUpdateFiles6();
   }

   if ( !retval )
   {
      installLog << "File extraction failed\n";
      return(-1);
   }

   installLog << "Updating machine.id\n";
   if (!installMachineId())
   {
      installLog << "Machine.id install failed\n";
      return(-1);
   }

   attrib(TEMP_PATH, "-R");

   // Update the configuration files
   // do this before all other config.dat changes
   installLog << "Update the configuration files\n";
   if (!updateConfigGeneric())
   {
      installLog << "Configuration update failed\n";
      return (-1);
   }

   // update cal files
   if ( newBuildData.calFileType == 5 )
   {
      installLog << "Update cal ver 5\n";
      updateCal5();
   }
   else
   {
      installLog << "Update cal ver 6\n";
      updateCal6();
   }

//    installLog << "Do a bunch of stuff\n";
// stuff to always do
   installLog << "Updating Globvars\n";
   updateGlobVars();
   installLog << "Updating RBC\n";
   updateRBC();
   installLog << "Updating Hardware\n";
   updateHW();
   installLog << "Updating App Server\n";
   updateAppServer();
   installLog << "Updating Software\n";
   updateSW();
   if (newBuildData.rangeType < V520)
   {
      // Force ARM = ON
      installLog << "Updating Software - Ver 5.19 Specific\n";
      bool result = replaceDatfileLine (PNAME_SWDAT, "airout_mitigation", "1", true);
   }
   installLog << "Updating Terror\n";
   updateTerror();
   installLog << "Updating Sounds\n";
   updateSounds();
   installLog << "Updating Cassette\n";
   updateCassette();
   installLog << "Updating Vista\n";
   updateVista();
   installLog << "Updating Barcode Categories\n";
   updateBarcodeCategories();
   installLog << "Updating RTS Config\n";
   updateRTSConfig();
   // installLog << "Updating machine.id\n";
   // installMachineId();

   // do different stuff for 6.3
   if ( newBuildData.setConfigCopy )
   {
      installLog << "force copy setconfig file\n";
      forceSetConfig();
   }
   else
   {
      installLog << "update setconfig file\n";
      updateSetConfig();
   }

   // validate the cassettes in setconfig.dat vs cassette.dat
   installLog << "validating the setconfig file\n";
   validateSetConfig(toVer);

   // if coming from 5.1.0-3 update trap files if called for
   if ( newBuildData.updateTrapFiles && fromVer.majorRev == 6 )
   {
      installLog << "update trap file\n";
      copyTrapFiles();
   }

   if ( newBuildData.adjPostcount )
   {
      installLog << "update post count\n";
      updatePostCount();
   }

   if ( newBuildData.adjPASRAS )
   {
      installLog << "check PAS & RAS settings\n";
      checkPasSettings();
      checkRasSettings();
   }

   // only do for 5.X to 5.X
   if ( (fromVer.majorRev == 6 || fromVer.majorRev == 7) &&
        (toVer.majorRev == 6 || toVer.majorRev == 7) )
   {
      installLog << "check plasma RB\n";
      checkPlasmaRB();
   }

#if CPU!=SIMNT
   // update & check the CRCs
   if ( newBuildData.calFileType == 5 )
   {
      installLog << "checkCRC5\n";
      retval = checkCRC5();
   }
   else
   {
      installLog << "checkCRC6\n";
      retval = checkCRC6();
   }

   if ( !retval )
   {
      return(-1);
   }
#endif

LEAVEROUTINE:
   ;
//    attrib( TEMP_PATH,"R" );

   installLog << "Trima software update complete\n";

   // Delete the update script so that it doesn't run again on the subsequent boot if the GTS guy
   // is still holding down the buttons.
   installLog << "Removing update script\n";
   remove(UPDATE_PATH "/projectrevision");
   remove(UPDATE_PATH "/updatetrima");
   remove(UPDATE_PATH "/updatetrima.taz");

   fflush(stdout);

#ifdef OUTPUTFILE
   freopen("CON", "w", stdout);
#endif

   trimaSysStartDiskSync();

   return(0);
}

/* FORMAT HASH bb8f796240f6f4bd5183e1fede4f1b61 */
