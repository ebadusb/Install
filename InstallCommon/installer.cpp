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
     prevBuildRef(0)
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
      if (!fpSource) installLog << "fpDest " << destDatFileName << " failed to open\n";

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
bool installer::validateSetConfig ()
{
   bool retval      = true;
   bool madeChanges = false;
   bool deleteItem  = false;
   char loggingBuff[256];

   // look to see if we're installing 5.1 and quit if we are because 5.1 doesn't use the cassette files
   struct stat fileStat;

   if ( stat((char*)TEMPLATES_PATH "/" FILE_CASSETTE_DAT, &fileStat) != OK )
   {
      installLog << "Installing 5.1, ignoring cassette file validation\n";
      goto LEAVEROUTINE;
   }

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

         if (foundCassette == MasterUpdateCassetteDat::end())
         {
            // didn't find the cassette from set_config.dat in cassette.dat so delete it
            installLog << "Cassette ref #: " << (*iter)->RefNum() << " not found in cassette.dat - removed\n";
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
         else
         {
//            sprintf(loggingBuff, "Found cassette ref #: %d admin code: %d barcode: %s\n", (int)((*iter)->RefNum()), (int)((*iter)->AdminCode()), (*iter)->BarcodeNum() );
//            updatetrimaUtils::logger(loggingBuff);
         }

         if (deleteItem)
         {
            AdminUpdateCassetteDat::erase(iter);
            madeChanges = true;

            // write the file and start over
//            updatetrimaUtils::logger("Removed line, restarting scan from the begining\n" );

            AdminUpdateCassetteDat::updateCassetteFile();
            iter       = AdminUpdateCassetteDat::begin();
            deleteItem = false;
            continue;
         }

         iter++;
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
      // delete any existing temp file
      if ( stat((char*)(tempFile.c_str()), &fileStat) == OK )
      {
         remove(tempFile.c_str());
      }

      // unzip the file to a temp file
      if ( updatetrimaUtils::unzipFile(filename, tempFile.c_str()) )
      {
         // open the temp file for appending
         FILE* tempfp = fopen(tempFile.c_str(), "a");
         if ( tempfp )
         {
            // get the machine name from globvars
            const char* machineName = NULL;
            machineName = findSetting("MACHINE=", CONFIG_PATH "/globvars");

            if ( machineName )
            {
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
                  fprintf(tempfp, "\n[MACHINE_ID]");
               }

               // write the machine ID to the file
               fprintf(tempfp, "\n%s\n", serialnumBufLine);

               fflush(tempfp);

               // close the temp file
               fclose(tempfp);

               if ( cp(filename, bakFile.c_str()) == ERROR )
               {
                  installLog << "Creation of backup file failed\n";
               }

               // delete the original file
               remove(filename);

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
               }

               // clean up file
               remove(tempFile.c_str());
               remove(bakFile.c_str());
            }
            else     // couldn't get a serial number
            {
               installLog << "couldn't get a serial number from globvars\n";

               // close & delete the temp file
               fclose(tempfp);
               remove(tempFile.c_str());
            }
         }
         else
         {
            installLog << "couldn't open " << tempFile.c_str() << " for append\n";

            // delete the temp file
            remove(tempFile.c_str());
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

   if ( stat((char*)TEMPLATES_PATH "/" FILE_FEATURES, &featuresFileStat) == OK )
   {

      if ( appendSerialNumToZipFile(TEMPLATES_PATH "/" FILE_FEATURES, true) )
      {
         // copy features.bin to the config directory
         attrib(PNAME_FEATURES, "-R");
         if ( cp(TEMPLATES_PATH "/" FILE_FEATURES, PNAME_FEATURES) == ERROR )
         {
            installLog << "copy of " << FILE_FEATURES << " failed\n";
         }

         attrib(PNAME_FEATURES, "+R");
      }

      // remove the old sw.dat files
      remove(PNAME_SWDAT);
      remove(TEMPLATES_PATH "/" FILE_SW_DAT);

   }
   else
   {
      // Not installing features.bin so delete any that exist in config directory
      if ( stat((char*)PNAME_FEATURES, &featuresFileStat) == OK )
      {
         remove(PNAME_FEATURES);
      }

      if ( stat((char*)TEMPLATES_PATH "/" FILE_FEATURES, &featuresFileStat) == OK )
      {
         remove(TEMPLATES_PATH "/" FILE_FEATURES);
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

void installer::updateCassette ()
{
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
   // these are the customer selected sets.... dont overwrite if it exists!
   currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_SETCONFIG_DAT);
   newVersion  = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SETCONFIG_DAT);

   // if the new version is from before we used setconfig.dat, remove it from /config
   if ( newVersion == NULL)
   {
      updatetrimaUtils::logger("Installing 5.1, ignoring setconfig.dat\n");
   }
   else if ( currVersion == NULL )
   {
      // if the file isnt there....
      updatetrimaUtils::logger("Adding ", FILE_SETCONFIG_DAT, "\n");

      if ( cp(TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT) == ERROR )
      {
         updatetrimaUtils::logger("copy of ", FILE_SETCONFIG_DAT, " failed\n");
         return;
      }

      attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
      fflush(stdout);
   }
   else
   {
      updatetrimaUtils::logger(FILE_SETCONFIG_DAT, " already exists, not copying a new one.\n");

      // do maintenance on the setconfig.dat file
      // replace 80537 with the new admin code
      if ( replaceCassette("80537", 2370507, "010502058380537") )
      {
         updatetrimaUtils::logger("Completed maintenance update to setconfig.dat\n");
      }
      else
      {
         updatetrimaUtils::logger("Unable to make maintenance update to setconfig.dat\n");
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
      if ( stat ((char*)PNAME_VISTAIPDAT, &vistaipFileStat) == ERROR)
      {
         if ( cp(vistaipTmpl.c_str(), PNAME_VISTAIPDAT) == ERROR )
         {
            updatetrimaUtils::logger("copy of ", vistaipTmpl.c_str(), " to ");
            updatetrimaUtils::logger(PNAME_VISTAIPDAT, " failed\n");
         }
         else
         {
            updatetrimaUtils::logger("copied ", vistaipTmpl.c_str(), " to ");
            updatetrimaUtils::logger(PNAME_VISTAIPDAT, " successfully\n");
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

               updatetrimaUtils::logger(PNAME_VISTAIPDAT, " file updated.\n");
            }
         }
         else
         {
            updatetrimaUtils::logger(FILE_VISTA_IP_DAT, " file reader creation error\n");
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
      updatetrimaUtils::logger("CRC ERROR ", crcReturnVal);
      updatetrimaUtils::logger(" on command line <", commandLine, ">\n");
   }

   return crcReturnVal;
}


void installer::copyTrapFiles ()
{
   attrib(TRAP_DEFAULTS_FILE, "-R");

   if (cp(TEMPLATES_PATH "/trap_default.dat", TRAP_DEFAULTS_FILE) == ERROR)
   {
      updatetrimaUtils::logger("copy of trap_default.dat\n");
      return;
   }
   attrib(TRAP_DEFAULTS_FILE, "+R");

   attrib(TRAP_OVERRIDE_FILE, "-R");

   if (cp(TEMPLATES_PATH "/trap_override.dat", TRAP_OVERRIDE_FILE) == ERROR)
   {
      updatetrimaUtils::logger("copy of trap_override.dat\n");
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
      updatetrimaUtils::logger("Calibration file read error : ", datfile.Error());
      updatetrimaUtils::logger("\n");
      return;
   }
   const std::string tsOriginal [] = {"screen_horizontal_size", "screen_vertical_size", "tsraw_left_edge", "tsraw_right_edge",
                                      "tsraw_top_edge", "tsraw_bottom_edge"};

   const char* tsHeader = "TOUCHSCREEN";
   struct stat fileStat;
   if ( stat((char*)PNAME_TCHSCRNDAT, &fileStat) == OK )
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
//         printf("Removal of %s file failed\n", PNAME_TCHSCRNDAT);
         updatetrimaUtils::logger("Removal of ", PNAME_TCHSCRNDAT, " file failed\n");
      }
      return;
   }

   else if ( !datfile.Find("TOUCHSCREEN", "screen_horizontal_size") )
   {
      updatetrimaUtils::logger("pre-v5.1 cal.dat file found.  Conversion needed\n");

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
            updatetrimaUtils::logger("Removal of absf.2 file failed\n");
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

      updatetrimaUtils::logger("cal.dat file converted.\n");

      return;
   }

   updatetrimaUtils::logger("v5.1 cal.dat file found.  No conversion needed\n");

}

void installer::updateCal6 ()
{
   //
   // Create the dat file reader to retrieve the calibration data.
   //
   CDatFileReader datfile(PNAME_CALDAT);
   if ( datfile.Error() )
   {
      updatetrimaUtils::logger("Calibration file read error : ", datfile.Error());
      updatetrimaUtils::logger("\n");
      return;
   }

   // For first time installs, copy the touchscreen template file over
   struct stat tsFileStat;
   if ( stat ((char*)PNAME_TCHSCRNDAT, &tsFileStat) == ERROR)
   {
      std::string tsTmpl(TEMPLATES_PATH "/" FILE_TCHSCRN_DAT);
      if ( stat(const_cast<char*>(tsTmpl.c_str()), &tsFileStat) == OK )
      {
         if ( cp(tsTmpl.c_str(), PNAME_TCHSCRNDAT) == ERROR )
         {
            updatetrimaUtils::logger("copy of ", tsTmpl.c_str());
            updatetrimaUtils::logger(" to ", PNAME_TCHSCRNDAT, " failed\n");
         }
         else
         {
            updatetrimaUtils::logger("copied ", tsTmpl.c_str());
            updatetrimaUtils::logger(" to ", PNAME_TCHSCRNDAT, " successfully\n");
         }
      }
   }


   CDatFileReader tscrnFile(PNAME_TCHSCRNDAT);
   if ( tscrnFile.Error() )
   {
      updatetrimaUtils::logger("Calibration file read error : ", datfile.Error());
      updatetrimaUtils::logger("\n");
      return;
   }

   const char*       tsHeader      = "TOUCHSCREEN";
   const std::string tsAF []       = {"a", "b", "c", "d", "e", "f"};
   const std::string tsOriginal [] = {"screen_horizontal_size", "screen_vertical_size", "tsraw_left_edge", "tsraw_right_edge",
                                      "tsraw_top_edge", "tsraw_bottom_edge"};

   // Is it an old 6.0 install (5.8)
   if ( datfile.Find(tsHeader, "a") )
   {
      updatetrimaUtils::logger("v6.0 ", FILE_CAL_DAT, " old 6.0 cal file found. Conversion needed\n");
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
      updatetrimaUtils::logger("Pre-v6.0 ", FILE_CAL_DAT, " file found.  Conversion needed\n");
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
      if ( stat((char*)PNAME_TCHSCRNDAT, &fileStat) == OK )
      {
         updatetrimaUtils::logger("File ", FILE_TCHSCRN_DAT);
         updatetrimaUtils::logger(" present with and up to date ", FILE_CAL_DAT, ". No conversion needed\n");
      }
      else
      {
         updatetrimaUtils::logger(" ... pre-v5.1 ", FILE_CAL_DAT, " file found. Unable to Convert! ending...\n");
      }
      return;
   }

   tscrnFile.Write(PNAME_TCHSCRNDAT);
   datfile.Write(PNAME_CALDAT);

   updatetrimaUtils::logger(FILE_CAL_DAT, " file converted.\n");
}

bool installer::extractUpdateFiles5 ()
{
   // This is done by the extractTopLevelFiles in updatetrima.cpp
   /*
   //
   // Extract the update files
   updatetrimaUtils::logger("Extracting updateTrima ...\n" );
   if ( tarExtract( "/machine/update/updateTrima.taz", "/machine/update" ) == ERROR )
   {
       updatetrimaUtils::logger("Extraction of update files failed\n" );
       return false;
   }
*/
   //
   // If we booted up using the default vxWorks image, then
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
      updatetrimaUtils::logger("Saving the old OS image...");
      attrib(VXBOOT_PATH "/vxWorks.old", "-R");
      if ( cp(VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.old") == ERROR )
      {
         updatetrimaUtils::logger("Archive of old OS image failed\n");
         return false;
      }
   }

   //
   // Store the new files in the proper position
   updatetrimaUtils::logger("Extracting the OS image...\n");
   if ( tarExtract(UPDATE_PATH "/vxboot.taz", UPDATE_PATH) == ERROR )
   {
      updatetrimaUtils::logger("Extraction of OS image failed\n");
      return false;
   }

   /* make the kernel images writeable so they can be overwritten */
   attrib(VXBOOT_PATH "/bootrom.sys", "-R");
   attrib(VXBOOT_PATH "/vxWorks", "-R");


   // check if we're installing a python-capable 5.X version
   struct stat fileStat;
   if ( stat((char*)UPDATE_PATH "/vxWorks_python", &fileStat) == OK ||
        stat((char*)UPDATE_PATH "/vxWorks_orig", &fileStat) == OK)
   {
      if (isVersalogicPython())
      {
         if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom.sys",  VXBOOT_PATH "/bootrom.sys") == ERROR ||
              updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_python", VXBOOT_PATH "/vxWorks") == ERROR )
         {
            updatetrimaUtils::logger("Install of Python OS image failed\n");
            return false;
         }
      }
      else
      {
         if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom.sys",  VXBOOT_PATH "/bootrom.sys") == ERROR ||
              updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_orig", VXBOOT_PATH "/vxWorks") == ERROR )
         {
            updatetrimaUtils::logger("Install of non-Python OS image failed\n");
            return false;
         }
      }

      if ( remove(UPDATE_PATH "/bootrom.sys") == ERROR ||
           remove(UPDATE_PATH "/vxWorks_orig") == ERROR ||
           remove(UPDATE_PATH "/vxWorks_python") == ERROR ||
           remove(UPDATE_PATH "/vxboot.taz") == ERROR )
      {
         updatetrimaUtils::logger("Removal of temporary OS image failed\n");
      }
   }
   else if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom.sys", VXBOOT_PATH "/bootrom.sys") == ERROR ||
             updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks", VXBOOT_PATH "/vxWorks") == ERROR )
   {
      updatetrimaUtils::logger("Install of OS image failed\n");
      return false;

      if ( remove(UPDATE_PATH "/bootrom.sys") == ERROR ||
           remove(UPDATE_PATH "/vxWorks") == ERROR ||
           remove(UPDATE_PATH "/vxboot.taz") == ERROR )
      {
         updatetrimaUtils::logger("Removal of temporary OS image failed\n");
      }
   }

/*
   if ( remove(UPDATE_PATH "/bootrom.sys") == ERROR ||
        remove(UPDATE_PATH "/vxWorks") == ERROR ||
        remove(UPDATE_PATH "/vxboot.taz") == ERROR )
   {
      updatetrimaUtils::logger("Removal of temporary OS image failed\n");
//      return false;
   }
*/

   //
   // Remove existing Trima files
   updatetrimaUtils::logger("Removing old Trima files...\n");
   fileSort(TRIMA_PATH,    FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(SAVEDATA_PATH, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(TOOLS_PATH,    FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);

   //
   // Uncompress the update file
   updatetrimaUtils::logger("Extracting the Trima software files...\n");
   if ( tarExtract("/machine/update/trima.taz", "/trima") == ERROR )
   {
      updatetrimaUtils::logger("Extraction of the Trima software failed.\n");
      return false;
   }
   if ( remove("/machine/update/trima.taz") == ERROR )
   {
      updatetrimaUtils::logger("Removal of Trima archive image failed\n");
      return false;
   }

   return true;

}

bool installer::extractUpdateFiles6 ()
{
   // This is done by the extractTopLevelFiles in updatetrima.cpp
   /*
   //
   // Extract the update files
   updatetrimaUtils::logger("Extracting updateTrima ...\n");
   if ( tarExtract( UPDATE_PATH "/updateTrima.taz", UPDATE_PATH ) == ERROR )
   {
       updatetrimaUtils::logger("Extraction of update files failed\n");
       return false;
   }
*/

   //
   // If we booted up using the default vxWorks image, then
   //  save it for emergency cases ...
   //
   BOOT_PARAMS* params = new BOOT_PARAMS;

   bootStringToStruct(sysBootLine, params);
   bool defaultBootImage = ( strcmp(params->bootFile, "/ata0/vxWorks") == 0 );
   bool isCommonKernel   = ( strcmp(params->bootFile, "/ata0a/vxWorks") == 0 );
   delete params;

   if ( defaultBootImage || isCommonKernel )
   {
      //
      // Save off the old vxWorks image in case of failure ...
      updatetrimaUtils::logger("Saving the old OS image...\n");
      attrib(VXBOOT_PATH "/vxWorks.old", "-R");

      if ( cp(VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.old") == ERROR )
      {
         updatetrimaUtils::logger("Archive of old OS image failed\n");
      }

      // Remove the .bak file to save space for vxWorks 6.9
      updatetrimaUtils::logger("Removing vxWorks.bak\n");
      attrib(VXBOOT_PATH "/vxWorks.bak", "-R");

      struct stat bakfileStat;

      if ( stat((char*)VXBOOT_PATH "/vxWorks.bak", &bakfileStat) == OK )
      {
         remove(VXBOOT_PATH "/vxWorks.bak");
      }
   }

   //
   // Store the new files in the proper position
   updatetrimaUtils::logger("Extracting the OS image...\n");

   if ( tarExtract(UPDATE_PATH "/vxboot.taz", UPDATE_PATH) == ERROR )
   {
      updatetrimaUtils::logger("Extraction of OS image failed\n");
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
      updatetrimaUtils::logger("Copying Control Ampro bootrom.sys and vxworks to ", VXBOOT_PATH);

      if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom_ampro.sys", VXBOOT_PATH "/bootrom.sys") == ERROR ||
           updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_ampro", VXBOOT_PATH "/vxWorks") == ERROR  )
      {
         updatetrimaUtils::logger("Install of OS image failed\n");
         return false;
      }
   }
   else
   {
      updatetrimaUtils::logger("Copying Control Versalogic bootrom.sys and vxworks to ", VXBOOT_PATH);

      if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom_versalogic.sys", VXBOOT_PATH "/bootrom.sys") == ERROR ||
           updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_versalogic", VXBOOT_PATH "/vxWorks") == ERROR  )
      {
         updatetrimaUtils::logger("Install of OS image failed\n");
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
   remove(UPDATE_PATH "/vxboot.taz");

   if ( attrib(UPDATE_PATH "/bootrom_ampro.sys", "-R") != ERROR ||
        attrib(UPDATE_PATH "/vxWorks_ampro", "-R") != ERROR ||
        attrib(UPDATE_PATH "/bootrom.sys", "-R") != ERROR ||
        attrib(UPDATE_PATH "/vxWorks", "-R") != ERROR ||
        attrib(UPDATE_PATH "/bootrom_versalogic.sys", "-R") != ERROR ||
        attrib(UPDATE_PATH "/vxWorks_versalogic", "-R") != ERROR ||
        attrib(UPDATE_PATH "/vxboot.taz", "-R") != ERROR )
   {
      updatetrimaUtils::logger("Removal of temporary OS image failed\n");
      return false;
   }

   //
   // Remove existing Trima files
   updatetrimaUtils::logger("Removing old Trima files...\n");
   fileSort(TRIMA_PATH,      FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(SAVEDATA_PATH,   FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(TOOLS_PATH,      FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(STRING_DIRECTORY, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(GRAPHICS_PATH,   FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);

   //
   // Uncompress the update file
   updatetrimaUtils::logger("Extracting the Trima software files...\n");

   if ( tarExtract(UPDATE_PATH "/trima.taz", TRIMA_PATH) == ERROR )
   {
      updatetrimaUtils::logger("Extraction of the Trima software failed.\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/trima.taz") == ERROR )
   {
      updatetrimaUtils::logger("Removal of Trima archive image failed\n");
      return false;
   }

   //
   // Uncompress the update file
   updatetrimaUtils::logger("Extracting the string.info files...\n");

   if ( tarExtract(UPDATE_PATH "/strings.taz", STRING_DIRECTORY) == ERROR )
   {
      updatetrimaUtils::logger("Extraction of the string.info files failed.\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/strings.taz") == ERROR )
   {
      updatetrimaUtils::logger("Removal of string archive image failed\n");
      return false;
   }

   //
   // Uncompress the update file
   updatetrimaUtils::logger("Extracting the font files...\n");

   if ( tarExtract(UPDATE_PATH "/fonts.taz", DROP_IN_FONTS_DIR) == ERROR )
   {
      updatetrimaUtils::logger("Extraction of the font files failed.\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/fonts.taz") == ERROR )
   {
      updatetrimaUtils::logger("Removal of font archive image failed\n");
      return false;
   }

   //
   // Uncompress the update file
   updatetrimaUtils::logger("Extracting the data files...\n");

   if ( tarExtract(UPDATE_PATH "/data.taz", DATA_DIRECTORY) == ERROR )
   {
      updatetrimaUtils::logger("Extraction of the data files failed.\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/data.taz") == ERROR )
   {
      updatetrimaUtils::logger("Removal of data archive image failed\n");
      return false;
   }

   //
   // Uncompress the update file
   updatetrimaUtils::logger("Extracting the graphics files...\n");

   if ( tarExtract(UPDATE_PATH "/graphics.taz", GRAPHICS_PATH) == ERROR )
   {
      updatetrimaUtils::logger("Extraction of the graphics files failed.\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/graphics.taz") == ERROR )
   {
      updatetrimaUtils::logger("Removal of graphics archive image failed\n");
      return false;
   }

   struct stat fileStat;

   // Look for Safety's Common Kernel boot files (vxboot_safety.taz)
   if ( stat((char*)UPDATE_PATH "/vxboot_safety.taz", &fileStat) == OK )
   {
      // Remove the files that will be installed by the extraction
      xdelete(SAFETY_COMMON_KERNEL_INIT_PATH);

      updatetrimaUtils::logger("Extracting the safety vxboot files...\n");

      if (tarExtract(UPDATE_PATH "/vxboot_safety.taz", SAFETY_COMMON_KERNEL_INIT_PATH) == ERROR)
      {
         updatetrimaUtils::logger("Extraction of the safety vxboot files failed.\n");
      }

      if (remove(UPDATE_PATH "/vxboot_safety.taz") == ERROR)
      {
         updatetrimaUtils::logger("Removal of Tools archive image failed\n");
      }

      // One other special-case for Common Kernel: to compute the same CRC that
      // Safety computes for its kernel init module, we do what Control does
      // during init and copy Safety's module to /machine/tmp.
      if (cp(SAFETY_KERNEL_INIT_SRC, SAFETY_KERNEL_INIT_TMP) == ERROR)
      {
         updatetrimaUtils::logger("Temp copy of safety's kernel init file failed: ", SAFETY_KERNEL_INIT_SRC, "\n");
      }
   }

   //
   // Copy over the safety images depending on the board type.
   //
   // If there are files in SAFETY_BOOT_PATH install Safety from there
   if ( attrib(SAFETY_BOOT_PATH "/bootrom_ampro.sys", "-R") != ERROR ||
        attrib(SAFETY_BOOT_PATH "/vxWorks_versalogic", "-R") != ERROR)
   {
      if ( isAmpro() )
      {
         updatetrimaUtils::logger("Copying Safety Ampro bootrom.sys and vxworks to ", SAFETY_BOOT_PATH);

         if ( cp(SAFETY_BOOT_PATH "/bootrom_ampro.sys", SAFETY_BOOTROM_IMAGE)    == ERROR ||
              cp(SAFETY_BOOT_PATH "/vxWorks_ampro", SAFETY_VXWORKS_IMAGE)    == ERROR )
         {
            updatetrimaUtils::logger("Install of OS image failed\n");
            return false;
         }
      }
      else
      {
         updatetrimaUtils::logger("Copying Safety Versalogic bootrom.sys and vxworks to ", SAFETY_BOOT_PATH, "\n");

         if ( cp(SAFETY_BOOT_PATH "/vxWorks_versalogic", SAFETY_VXWORKS_IMAGE)     == ERROR ||
              cp(SAFETY_BOOT_PATH "/bootrom_versa_bootp.sys", SAFETY_BOOTROM_IMAGE)     == ERROR ||
              cp(SAFETY_BOOT_PATH "/vxWorks_versalogic_pxe", SAFETY_VXWORKS_PXE_IMAGE) == ERROR ||
              cp(SAFETY_BOOT_PATH "/bootrom_versa_pxe.sys", SAFETY_BOOTROM_PXE_IMAGE) == ERROR )
         {
            updatetrimaUtils::logger("Install of OS image failed\n");
            return false;
         }
      }
   }
   else if ( attrib(SAFETY_COMMON_KERNEL_INIT_PATH "/vxWorks", "-R") == ERROR )
   {
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

   if ( attrib(SAFETY_BOOT_PATH "/bootrom_ampro.sys", "-R") != ERROR ||
        attrib(SAFETY_BOOT_PATH "/vxWorks_ampro", "-R") != ERROR ||
        attrib(SAFETY_BOOT_PATH "/bootrom_versa_bootp.sys", "-R") != ERROR ||
        attrib(SAFETY_BOOT_PATH "/bootrom_versa_pxe.sys", "-R") != ERROR ||
        attrib(SAFETY_BOOT_PATH "/vxWorks_versalogic", "-R") != ERROR ||
        attrib(SAFETY_BOOT_PATH "/vxWorks_versalogic_pxe", "-R") != ERROR )
   {
      updatetrimaUtils::logger("removal of temporary OS image failed\n");
      return false;
   }


   // Uncompress the optional tools archive if it exists
   if ( stat((char*)UPDATE_PATH "/engr_tools.taz", &fileStat) == OK )
   {
      updatetrimaUtils::logger("Extracting the engr tools files...\n");

      if (tarExtract(UPDATE_PATH "/engr_tools.taz", ROOT "/machine/tools") == ERROR)
      {
         updatetrimaUtils::logger("Extraction of the Tools files failed.\n");
      }

      if (remove(UPDATE_PATH "/engr_tools.taz") == ERROR )
      {
         updatetrimaUtils::logger("Removal of Tools archive image failed\n");
      }
   }

   return true;
}

bool installer::checkCRC5 ()
{

   // Set permissions in config directory
   fileSort(CONFIG_PATH, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_file_set_rdwrite);


   // Remove FEATURES files since it doesn't exist in 5.X
   struct stat featuresFileStat;
   if ( stat((char*)FILELISTS_PATH "/features.files", &featuresFileStat) == OK )
   {
      remove(FILELISTS_PATH "/features.files");
   }

   struct stat featurescrcFileStat;
   if ( stat((char*)CONFIG_CRC_PATH "/features.crc", &featurescrcFileStat) == OK )
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
      updatetrimaUtils::logger("CRC check of installed software failed\n");
      return false;
   }

   return true;
}

bool installer::checkCRC6 ()
{
   //
   // Update configuration CRC values
   mkdir(CONFIG_CRC_PATH);

   softcrc("-filelist " FILELISTS_PATH "/caldat.files    -update " CONFIG_CRC_PATH  "/caldat.crc");
   softcrc("-filelist " FILELISTS_PATH "/config.files    -update " CONFIG_CRC_PATH  "/config.crc");
   softcrc("-filelist " FILELISTS_PATH "/hwdat.files     -update " CONFIG_CRC_PATH  "/hwdat.crc");
   softcrc("-filelist " FILELISTS_PATH "/rbcdat.files    -update " CONFIG_CRC_PATH  "/rbcdat.crc");
   softcrc("-filelist " FILELISTS_PATH "/cassette.files  -update " CONFIG_CRC_PATH  "/cassette.crc");
   softcrc("-filelist " FILELISTS_PATH "/setconfig.files -update " CONFIG_CRC_PATH  "/setconfig.crc");
   softcrc("-filelist " FILELISTS_PATH "/graphics.files  -update " PNAME_GUI_GRAPHICS_CRC);
   softcrc("-filelist " FILELISTS_PATH "/strings.files   -update " PNAME_STRING_CRC);
   softcrc("-filelist " FILELISTS_PATH "/fonts.files     -update " PNAME_FONT_CRC);
   softcrc("-filelist " FILELISTS_PATH "/data.files      -update " PNAME_DATA_CRC);
   softcrc("-filelist " FILELISTS_PATH "/safety.files    -update " TRIMA_PATH "/safety.crc");
   softcrc("-filelist " FILELISTS_PATH "/trima.files     -update " TRIMA_PATH "/trima.crc");
   softcrc("-filelist " FILELISTS_PATH "/machine.files   -update " CONFIG_CRC_PATH "/machine.crc");

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


   // Set permissions in config directory
   updatetrimaUtils::update_file_set_rdonly(CONFIG_PATH);

   // Verify the installation CRC values
   if (verifyCrc("-filelist " FILELISTS_PATH "/caldat.files    -verify " CONFIG_CRC_PATH "/caldat.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/config.files    -verify " CONFIG_CRC_PATH "/config.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/hwdat.files     -verify " CONFIG_CRC_PATH "/hwdat.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/rbcdat.files	   -verify "CONFIG_CRC_PATH"/rbcdat.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/cassette.files  -verify " CONFIG_CRC_PATH "/cassette.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/setconfig.files -verify " CONFIG_CRC_PATH "/setconfig.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/graphics.files  -verify " PNAME_GUI_GRAPHICS_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/strings.files   -verify " PNAME_STRING_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/fonts.files     -verify " PNAME_FONT_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/data.files      -verify " PNAME_DATA_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/safety.files    -verify " TRIMA_PATH "/safety.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/trima.files     -verify " TRIMA_PATH "/trima.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/machine.files   -verify " CONFIG_CRC_PATH "/machine.crc"))
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

   return true;
}

bool installer::checkPasSettings ()
{
   bool returnVal = false;

   CDatFileReader datfile(PNAME_CONFIGDAT);
   if ( datfile.Error() )
   {
      updatetrimaUtils::logger("Config file read error : ", datfile.Error());
      updatetrimaUtils::logger("\n");
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
         updatetrimaUtils::logger("Hmm... for PLT", prodNum + 1);
         updatetrimaUtils::logger(" percent plasma is ");
         updatetrimaUtils::logger(percentPlasma);
         updatetrimaUtils::logger(" and collect vol is ");
         updatetrimaUtils::logger(collectVol);
         updatetrimaUtils::logger(". Moving on...\n");
         continue;
      }

      // Figure out the max bag volume.
      float collectVolMax = floor(1200.0f * (percentPlasma / 100.0f));

      if (collectVol > collectVolMax)
      {
//         printf("PLT%d has collect vol %f greater than max %f. Adjusting accordingly.\n", prodNum + 1, collectVol, collectVolMax);
         updatetrimaUtils::logger("PLT", prodNum + 1);
         updatetrimaUtils::logger(" has collect vol ");
         updatetrimaUtils::logger(collectVol);
         updatetrimaUtils::logger(" greater than max ");
         updatetrimaUtils::logger(collectVolMax);
         updatetrimaUtils::logger(". Adjusting accordingly.\n");

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
      updatetrimaUtils::logger("Config file read error : ", datfile.Error());
      updatetrimaUtils::logger("\n");
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
      updatetrimaUtils::logger("copy of ", FILE_SETCONFIG_DAT, " failed\n");
      return;
   }

   attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
   fflush(stdout);
//   }
}

bool installer::checkRange (const char* section, const char* key, const char* value, std::string& forceVal)
{
   const char* rangeTypeStr[]    = {"V510", "V520", "V600", "V630", "V640", "V700", "END"};
   const char* compareTypeStr[]  = {"MIN", "MAX", "NOT", "FORCE"};
   const char* rangeValTypeStr[] = {"INT", "FLOAT"};

//   installLog << "Checking range of: " << section << " " << key << " " << value << "\n";

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
//            installLog << "Forcing update\n";
            retval = false;
         }
         else if ( rangeData.valType == INT )
         {
            int intvalue = atoi(value);

//            installLog << "Comparing INT " << intvalue;

            if ( rangeData.compareType == MIN )
            {
//               installLog << " MIN vs " << rangeData.value;

               if ( intvalue < atoi(rangeData.value) )
               {
//                  installLog << " FAILED\n";
                  retval = false;
               }
//               installLog << " PASSED\n";
            }
            else if ( rangeData.compareType == MAX )
            {
//               installLog << " MAX vs " << rangeData.value;

               if ( intvalue > atoi(rangeData.value) )
               {
//                  installLog << " FAILED\n";
                  retval = false;
               }
//               installLog << " PASSED\n";
            }
            else if ( rangeData.compareType == NOT )
            {
//               installLog << " NOT vs " << rangeData.value;

               if ( intvalue == atoi(rangeData.value) )
               {
//                  installLog << " FAILED\n";
                  retval = false;
               }
//               installLog << " PASSED\n";
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
         installLog << " Found in range data, FAILD a check\n";

         if (rangeData.useLimit)
         {
            forceVal = rangeData.value;
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


bool installer::checkPlasmaRB ()
{
   bool        retval = false;
   const char* configData;
   const char* swData;

   // Get the rinseback setting from sw.dat
   swData = findSetting("allow_plasma_rinseback=", PNAME_SWDAT);

//   if (swData) updatetrimaUtils::logger("sw.dat allow_plasma_rinseback = ", swData, "\n");

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
            updatetrimaUtils::logger("Can't open config.dat to modify key_plasma_rinseback\n");
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

   if ( isVersalogicPython() && !python )
   {
      updatetrimaUtils::logger("This update path is not allowed on a VersaLogic Python Trima.\n");
      retval = false;
   }
   else
   {
      updatetrimaUtils::logger("This update path is allowed.\n");
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

//         installLog << "Checking: " << section << " " << cfLine.cpName() << " " << newVal << "\n";

         // get the value from the new config.dat
         strVal = newdatfile.GetString(section, cfLine.cpName());
//         installLog << section << " " << cfLine.cpName() << " " << "Existing val: " << newVal << " Default val: " << strVal << "\n";

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
               newdatfile.SetValue(section, cfLine.cpName(), newVal);
               installLog << " Transferring setting for: " << section << " " << cfLine.cpName() << ", setting to: " << newVal << "\n";
            }
            else
            {
               installLog << " Range check failed for: " << section << " " << cfLine.cpName() << " " << newVal;

               // if the compare failed and a forced value is provided, use it
               if (forceVal.size() > 0)
               {
                  newdatfile.SetValue(section, cfLine.cpName(), forceVal.c_str());
                  installLog << ", Using forced value of " << forceVal.c_str() << "\n";
               }
               else
               {
                  installLog << ", Using default value of " << strVal.c_str() << "\n";
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

void installer::installMachineId ()
{
   struct stat tempFileStat;
   struct stat configFileStat;
   int         tempFileStatStatus   = OK;
   int         configFileStatStatus = OK;

   tempFileStatStatus   = stat(const_cast<char*>(TEMPLATES_PATH "/" FILE_MACHINE_ID), &tempFileStat);
   configFileStatStatus = stat(const_cast<char*>(PNAME_MACHINE_ID), &configFileStat);

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
         }
      }
      else
      {
         installLog << "Appending serial number failed so copy of default machine.id failed\n";
      }
   }
   else
   {
      installLog << "Install of machine.id not needed\n";
   }
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

   updatetrimaUtils::update_file_set_rdwrite(CONFIG_PATH);

   //
   // Make /vxboot and /trima partitions writable

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

   // Remove any old copy of features.bin in templates
   installLog << "Removing any old copy of features.bin in templates\n";
   struct stat featuresFileStat;

   if ( stat((char*)TEMPLATES_PATH "/" FILE_FEATURES, &featuresFileStat) == OK )
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

   attrib(TEMP_PATH, "-R");

   // Update the configuration files
   // do this before all other config.dat changes
   // Don't do this if it's the same version
   if (newBuildRef != prevBuildRef)
   {
      installLog << "Update the configuration files\n";
      if ( !updateConfigGeneric() )
      {
         installLog << "Configuration update failed\n";
         return(-1);
      }
   }
   else
   {
      installLog << "No config.dat key difference between the previous software version and the new one, using the existing config.dat unchanged.\n";
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

//    updatetrimaUtils::logger("Do a bunch of stuff\n");
// stuff to always do
   installLog << "Updating Globvars\n";
   updateGlobVars();
   installLog << "Updating RBC\n";
   updateRBC();
   installLog << "Updating Hardware\n";
   updateHW();
   installLog << "Updating Software\n";
   updateSW();
   installLog << "Updating Terror\n";
   updateTerror();
   installLog << "Updating Sounds\n";
   updateSounds();
   installLog << "Updating Cassette\n";
   updateCassette();
   installLog << "Updating Vista\n";
   updateVista();
   installLog << "Updating machine.id\n";
   installMachineId();

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
   validateSetConfig();

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

/* FORMAT HASH a4d896ab5acc87f2640fa66bf3cbb9ce */
