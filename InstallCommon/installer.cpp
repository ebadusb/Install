/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include "updatetrima.h"
#include "installer.h"

#include "crc.c"
#include <fstream>

// #include "zlib.h"
#include "targzextract.c"
#include "updatecassette_dat.h"

#include "updatetrimaData.h"

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


// Default constructor
installer::installer()
{}

// Copy constructor
installer::installer(const installer& obj)
{}

// Default destructor
installer::~installer()
{}

bool installer::replaceDatfileLine (const char* datFileName, const char* optionName, const char* newVal)
{
   bool  retval     = true;
   bool  madeChange = false;
   char  destDatFileName[256];
   char  oldDatFileName[256];

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
      if (!fpSource) printf("fpSource %s failed to open\n", datFileName);
//        if (!fpSource) logStream << "fpSource " << datFileName << " failed to open" << endl;
      if (!fpDest) printf("fpDest %s failed to open\n", destDatFileName);
//        if (!fpDest) logStream << "fpDest " << destDatFileName << " failed to open" << endl;

      retval = false;
      fclose(fpSource);
      fclose(fpDest);
      goto LEAVEROUTINE;
   }

   while ( fgets(inputBuffer, 256, fpSource) )
   {
      // make a copy to work on
      strncpy(workingBuffer, inputBuffer, strlen(inputBuffer));

      // printf("inputBuffer %s\n", inputBuffer);

      char* start = workingBuffer + strspn(workingBuffer, " \t");    // get rid of leading white space

      // printf("workingBuffer %s\n", workingBuffer);
      // printf("start %s\n", start);

      // did we find the option?
      if ( strncmp(start, optionName, strlen(optionName)) == 0 )
      {
         // now look for the existing value
         start += strlen(optionName);
         start += strcspn(start, " =");         // pass any spaces and ='s
         start[strcspn(start, ",\n\r")] = '\0'; // stop it at the comma in front of the crc & get rid of trailing cr/lf's

         // is the value not equal to the new value?
         if ( strncmp(start, newVal, strlen(newVal)) != 0 )
         {
            unsigned long crcval = 0;
            long          buflen = 0;

            // create the updated line
            buflen = sprintf(baseNewlineBuf, "%s=%s", optionName, newVal);

            // calc the crc for the line
            crcgen32(&crcval, (const unsigned char*)baseNewlineBuf, buflen);

            // add the crc to the line
            buflen = sprintf(newlineBuf, "%s,%lx\n", baseNewlineBuf, crcval);

            // set the new line to be the one written
            outputLine = newlineBuf;

            // set this so we know we've made a change
            madeChange = true;
         }
      }

      // write the line to the dest file
      fprintf(fpDest, "%s", outputLine);

      // reset the output pointer in case we changed it
      outputLine = inputBuffer;
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
//            printf("move existing file failed\n");
         // logStream << "move existing file failed" << endl;
         // stop
         retval = false;
      }
      else if ( mv(destDatFileName, datFileName) == ERROR )
      {
//            printf("move new file to existing file failed\n");
         // logStream << "move new file to existing file failed" << endl;
         // try to put it back
         mv(oldDatFileName, datFileName);
         retval = false;
      }
   }
   else
   {
      rm(destDatFileName);
   }

LEAVEROUTINE: ;     // this extra semicolon cleans up auto-indenting after the label

   // printf("returning %d\n", retval);
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
         char* start = buffer + strspn(buffer, " \t");
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
//        printf("opened %s\n", fileName);
      // logStream << "opened " << fileName << endl;

      result = findSetting(setting, fp);
      fclose(fp);
   }

   return result;
}

bool installer::replaceCassette (const char* refStr, unsigned int tubingSetCode, const char* barcodeStr)
{
   bool retval = true;

   AdminUpdateCassetteDat::read();

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
      retval = false;
   }

   return( retval );
}


bool installer::updatePostCount ()
{
   bool           bUpdate = false;

   CDatFileReader datfile(PNAME_CONFIGDAT);
   if ( datfile.Error() )
   {
      cerr << "Config file read error : " << datfile.Error() << endl;
      return bUpdate;
   }

   // IT 8741/8737 - Force post count config setting to 100K
   float postCount = datfile.GetFloat("PROCEDURE_CONFIG", "key_post_plat");
   if (postCount < 100000)
   {
      datfile.RemoveLine("PROCEDURE_CONFIG", "key_post_plat");
      datfile.AddLine("PROCEDURE_CONFIG", "key_post_plat", "100000");
      cerr << "Changed donor post count from " << postCount << " to 100000" << endl;
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
//        printf("copy of rbc.dat failed\n");
      // logStream << "copy of rbc.dat failed" << endl;
      return;
   }
   attrib(CONFIG_PATH "/" FILE_RBC_DAT, "+R");

}

void installer::updateGlobVars ()
{
   // Create the dat file reader to retrieve the global environment vars data.
   CDatFileReader datfile(GLOBVARS_FILE, false, true);
   if ( datfile.Error() )
   {
      cerr << GLOBVARS_FILE << " file read error : " << datfile.Error() << endl;
      return;
   }

   if ( !datfile.Find("EXTERNALIP") )
   {
      cerr << "pre-v5.1 globvars file found Unable to Convert... ending" << endl;
      return;
   }

   cerr << "v5.2 globvars file found.  No conversion needed" << endl;
}

void installer::updateHW ()
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
//        printf("Updating hw.dat to new version %s from existing version %s...\n", newVersion, currVersion);
      // logStream << "Updating hw.dat to new version " << newVersion << "from existing version" << currVersion << endl;

      attrib(CONFIG_PATH "/" FILE_HW_DAT, "-R");

      if ( isAmpro() )
      {
         if ( cp(TEMPLATES_PATH "/hw_ampro.dat", CONFIG_PATH "/" FILE_HW_DAT) == ERROR )
         {
//                printf("copy of hw_ampro.dat failed\n");
            // logStream << "copy of hw_ampro.dat failed" << endl;
            return;
         }
      }
      else
      {
         if ( cp(TEMPLATES_PATH "/hw_versalogic.dat", CONFIG_PATH "/" FILE_HW_DAT) == ERROR )
         {
//                printf("copy of hw_versalogic.dat failed\n");
            // logStream << "copy of hw_versalogic.dat failed" << endl;
            return;
         }
      }
      attrib(CONFIG_PATH "/" FILE_HW_DAT, "+R");
      fflush(stdout);
   }

}

void installer::updateSW ()
{
   // Look if there is a features.bin and use it instead of sw.dat
   struct stat featuresFileStat;

   if ( stat((char*)TEMPLATES_PATH "/" FILE_FEATURES, &featuresFileStat) == OK )
   {
//        printf("features.bin exists\n");
      // logStream << "features.bin exists" << endl;

      // update features.bin with the machine ID

      // delete any existing temp file
      if ( stat((char*)TEMPLATES_PATH "/features.temp", &featuresFileStat) == OK )
      {
         remove(TEMPLATES_PATH "/features.temp");
      }

      // unzip features.bin to a temp file
      if ( updatetrimaUtils::unzipFile(TEMPLATES_PATH "/" FILE_FEATURES, TEMPLATES_PATH "/features.temp") )
      {
//            printf("unzipped features.temp\n");
         // logStream << "unzipped features.temp" << endl;

         // open the temp file for appending
         FILE* fp = fopen(TEMPLATES_PATH "/features.temp", "a");
         if ( fp )
         {
//                printf("opened features.temp\n");
            // logStream << "opened features.temp" << endl;

            // get the machine name
            FILE* fp2 = fopen(CONFIG_PATH "/globvars", "r");

            if ( fp2 )
            {
//                    printf("opened globvars\n");
               // logStream << "opened globvars" << endl;
            }
            else
            {
//                    printf("couldn't open globvars\n");
               // logStream << "couldn't open globvars" << endl;
            }
            fclose(fp2);

            const char* machineName = NULL;
            machineName = findSetting("MACHINE=", CONFIG_PATH "/globvars");

            if ( machineName )
            {
               printf("machine name = %s\n", machineName);

               unsigned long crcval = 0;
               long          buflen = 0;
               char          serialnumBuf[256];

               // create the base serial number line
               buflen = sprintf(serialnumBuf, "serial_number=%s", machineName);

               // calc the crc for the line
               crcgen32(&crcval, (const unsigned char*)serialnumBuf, buflen);

               // add the crc to the line
               char serialnumBufLine[256];
               buflen = sprintf(serialnumBufLine, "%s,%lx", serialnumBuf, crcval);

               // write the machine ID to the file
               fprintf(fp, "\n[MACHINE_ID]\n%s\n", serialnumBufLine);
               fflush(fp);

               // close the temp file
               fclose(fp);

               // delete the original features.bin from templates
               remove(TEMPLATES_PATH "/" FILE_FEATURES);

               // zip the temp file to features.bin
               if (updatetrimaUtils::zipFile(TEMPLATES_PATH "/features.temp", TEMPLATES_PATH "/" FILE_FEATURES) == 0)
               {
                  // zip failed
                  printf("zip of features.bin failed\n");
                  goto LEAVEROUTINE;
               }

               // delete the temp file
               remove(TEMPLATES_PATH "/features.temp");

               // copy features.bin to the config directory
               attrib(PNAME_FEATURES, "-R");
               if ( cp(TEMPLATES_PATH "/" FILE_FEATURES, PNAME_FEATURES) == ERROR )
               {
                  printf("copy of %s failed\n", FILE_FEATURES);
                  goto LEAVEROUTINE;
               }
               if (attrib(PNAME_FEATURES, "+R") == ERROR)
               {
                  printf("copy of %s to config failed\n", FILE_FEATURES);
                  goto LEAVEROUTINE;
               }

               updatetrimaUtils::unzipFile(TEMPLATES_PATH "/" FILE_FEATURES, TEMPLATES_PATH "/features.temp2");

            }
            else
            {
               // close the temp file
               fclose(fp);

               // machine name not defined
               printf("machine name not defined\n");
               goto LEAVEROUTINE;
            }
         }
         else
         {
            // can't open the temp file
            printf("can't open the features.bin temp file\n");
            goto LEAVEROUTINE;
         }
      }
      else
      {
         // unzip failed
         printf("unzip of features.bin failed\n");
         goto LEAVEROUTINE;
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
         printf("Updating sw.dat to new version %s from existing version %s...\n", newVersion, currVersion);
         attrib(CONFIG_PATH "/" FILE_SW_DAT, "-R");

         if ( cp(TEMPLATES_PATH "/" FILE_SW_DAT, CONFIG_PATH "/" FILE_SW_DAT) == ERROR )
         {
            printf("copy of %s failed\n", FILE_SW_DAT);
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
      printf("Updating terror_config.dat to new version %s from existing version %s...\n", newVersion, currVersion);
      attrib(TERROR_CONFIG_FILE, "-R");

      if ( cp(TEMPLATES_PATH "/terror_config.dat", CONFIG_PATH "/terror_config.dat") == ERROR )
      {
         printf("copy of terror_config.dat failed\n");
         return;
      }

      attrib(CONFIG_PATH "/terror_config.dat", "+R");
      fflush(stdout);
   }
}

void installer::updateSounds ()
{
   printf("Updating sounds.dat...\n");
   attrib(PNAME_SOUNDSDAT, "-R");

   if ( cp(TEMPLATES_PATH "/" FILE_SOUNDS_DAT, PNAME_SOUNDSDAT) == ERROR )
   {
      printf("copy of sounds.dat failed\n");
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

   if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
   {
      printf("Updating %s to new version %s from existing version %s...\n", FILE_CASSETTE_DAT, newVersion, currVersion);
      attrib(CONFIG_PATH "/" FILE_CASSETTE_DAT, "-R");

      if ( cp(TEMPLATES_PATH "/" FILE_CASSETTE_DAT, CONFIG_PATH "/" FILE_CASSETTE_DAT) == ERROR )
      {
         printf("copy of %s failed\n", FILE_CASSETTE_DAT);
         return;
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

   if (currVersion == NULL && newVersion != NULL)
   {
      // if the file isnt there....
      printf("Adding %s ...\n", FILE_SETCONFIG_DAT);

      if ( cp(TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT) == ERROR )
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

      // do maintenance on the setconfig.dat file
      // replace 80537 with the new admin code
      if ( replaceCassette("80537", 2370507, "010502058380537") )
      {
         printf("Completed maintenance update to setconfig.dat\n");
      }
      else
      {
         printf("Unable to make maintenance update to setconfig.dat\n");
      }
   }
}

void installer::updateVista ()
{
   std::string vistaipTmpl(TEMPLATES_PATH "/" FILE_VISTIP_DAT);
   struct stat vistaipTemplFileStat;

   // if there's a vista ip template file we have to make sure the config file is updated
   if ( stat(const_cast<char*>(vistaipTmpl.c_str()), &vistaipTemplFileStat) == OK )
   {
      // For first time installs, copy the template file over to config
      struct stat vistaipFileStat;
      if ( stat ((char*)PNAME_VISTIPDAT, &vistaipFileStat) == ERROR)
      {
         if ( cp(vistaipTmpl.c_str(), PNAME_VISTIPDAT) == ERROR )
         {
            cerr << "copy of " << vistaipTmpl << " to " << PNAME_VISTIPDAT  << " failed" << endl;
         }
         else
         {
            cerr << "copied" << vistaipTmpl << " to " << PNAME_VISTIPDAT  << " successfully" << endl;
         }
      }
      else    // the config already exists so see if we need to update it
      {
         // create the file readers
         CDatFileReader vistaipConfigFile(PNAME_VISTIPDAT);
         CDatFileReader vistaipTemplFile(vistaipTmpl.c_str());

         if ( !vistaipConfigFile.Error() && !vistaipTemplFile.Error() )
         {
            // get the format versions
            float confFormatVer  = vistaipConfigFile.GetFloat("Version", "FormatVersion");
            float templFormatVer = vistaipTemplFile.GetFloat("Version", "FormatVersion");

            // the version of the template is newer, so update the config file
            if ( templFormatVer > confFormatVer )
            {
               vistaipConfigFile.SetValue("VISTA", "VISTA_DIRECT_SEND_IP", vistaipTemplFile.GetString("VISTA", "VISTA_DIRECT_SEND_IP").c_str());
               vistaipConfigFile.SetValue("VISTA", "VISTA_DIRECT_SEND_PORT", vistaipTemplFile.GetString("VISTA", "VISTA_DIRECT_SEND_PORT").c_str());
               vistaipConfigFile.Write(PNAME_VISTIPDAT);

               cerr << PNAME_VISTIPDAT << " file updated." << endl;
            }
         }
         else
         {
            cerr << FILE_VISTIP_DAT << " file reader creation error" << endl;
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
   }

   return crcReturnVal;
}


void installer::copyTrapFiles ()
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

void installer::updateCal5 ()
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
   const std::string tsOriginal [] = {"screen_horizontal_size", "screen_vertical_size", "tsraw_left_edge", "tsraw_right_edge",
                                      "tsraw_top_edge", "tsraw_bottom_edge"};

   const char*       tsHeader = "TOUCHSCREEN";
   struct stat       fileStat;
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
         printf("Removal of %s file failed\n", PNAME_TCHSCRNDAT);
      }
      return;
   }

   else if ( !datfile.Find("TOUCHSCREEN", "screen_horizontal_size") )
   {
      cerr << "pre-v5.1 cal.dat file found.  Conversion needed" << endl;

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
            printf("Removal of absf.2 file failed\n");
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

      cerr << "cal.dat file converted." << endl;

      return;
   }

   cerr << "v5.1 cal.dat file found.  No conversion needed" << endl;

}

void installer::updateCal6 ()
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

   // For first time installs, copy the touchscreen template file over
   struct stat tsFileStat;
   if ( stat ((char*)PNAME_TCHSCRNDAT, &tsFileStat) == ERROR)
   {
      std::string tsTmpl(TEMPLATES_PATH "/" FILE_TCHSCRN_DAT);
      if ( stat(const_cast<char*>(tsTmpl.c_str()), &tsFileStat) == OK )
      {
         if ( cp(tsTmpl.c_str(), PNAME_TCHSCRNDAT) == ERROR )
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

   const char*       tsHeader      = "TOUCHSCREEN";
   const std::string tsAF []       = {"a", "b", "c", "d", "e", "f"};
   const std::string tsOriginal [] = {"screen_horizontal_size", "screen_vertical_size", "tsraw_left_edge", "tsraw_right_edge",
                                      "tsraw_top_edge", "tsraw_bottom_edge"};

   // Is it an old 6.0 install (5.8)
   if ( datfile.Find(tsHeader, "a") )
   {
      cerr << "v6.0 " << FILE_CAL_DAT << " old 6.0 cal file found.  Conversion needed" << endl;
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
            std::cout << tsHeader << ":" << tsOriginal[i] << " not found " << std::endl;
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

bool installer::extractUpdateFiles5 ()
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
   BOOT_PARAMS* params = new BOOT_PARAMS;

   bootStringToStruct(sysBootLine, params);
   bool defaultBootImage = ( strcmp(params->bootFile, "/ata0/vxWorks") == 0 );
   delete params;

   if ( defaultBootImage )
   {
      //
      // Save off the old vxWorks image in case of failure ...
      printf("Saving the old OS image...");
      attrib(VXBOOT_PATH "/vxWorks.old", "-R");
      if ( cp(VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.old") == ERROR )
      {
         printf("Archive of old OS image failed\n");
      }
   }

   //
   // Store the new files in the proper position
   printf("Extracting the OS image...\n");
   if ( tarExtract(UPDATE_PATH "/vxboot.taz", UPDATE_PATH) == ERROR )
   {
      printf("Extraction of OS image failed\n");
      return false;
   }

   /* make the kernel images writeable so they can be overwritten */
   attrib(VXBOOT_PATH "/bootrom.sys", "-R");
   attrib(VXBOOT_PATH "/vxWorks", "-R");

   if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom.sys", VXBOOT_PATH "/bootrom.sys") == ERROR ||
        updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks", VXBOOT_PATH "/vxWorks") == ERROR )
   {
      printf("Install of OS image failed\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/bootrom.sys") == ERROR ||
        remove(UPDATE_PATH "/vxWorks") == ERROR ||
        remove(UPDATE_PATH "/vxboot.taz") == ERROR )
   {
      printf("Removal of temporary OS image failed\n");
      return false;
   }

   //
   // Remove existing Trima files
   printf("Removing old Trima files...\n");
   fileSort(TRIMA_PATH,    FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(SAVEDATA_PATH, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(TOOLS_PATH,    FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);

   //
   // Uncompress the update file
   printf("Extracting the Trima software files...\n");
   if ( tarExtract("/machine/update/trima.taz", "/trima") == ERROR )
   {
      printf("Extraction of the Trima software failed.\n");
      return false;
   }
   if ( remove("/machine/update/trima.taz") == ERROR )
   {
      printf("Removal of Trima archive image failed\n");
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
   printf("Extracting updateTrima ...\n");
   if ( tarExtract( UPDATE_PATH "/updateTrima.taz", UPDATE_PATH ) == ERROR )
   {
       printf("Extraction of update files failed\n");
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
      printf("Saving the old OS image...\n");
      attrib(VXBOOT_PATH "/vxWorks.old", "-R");

      if ( cp(VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.old") == ERROR )
      {
         printf("Archive of old OS image failed\n");
      }

      // Save off backup vxWorks image in case of massive failure ...
      printf("Saving the backup OS image...\n");
      attrib(VXBOOT_PATH "/vxWorks.bak", "-R");

      if ( cp(VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.bak") == ERROR )
      {
         printf("Archive of backup OS image failed\n");
      }
   }

   //
   // Store the new files in the proper position
   printf("Extracting the OS image...\n");

   if ( tarExtract(UPDATE_PATH "/vxboot.taz", UPDATE_PATH) == ERROR )
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

      if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom_ampro.sys", VXBOOT_PATH "/bootrom.sys") == ERROR ||
           updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_ampro", VXBOOT_PATH "/vxWorks") == ERROR  )
      {
         printf("Install of OS image failed\n");
         return false;
      }
   }
   else
   {
      printf("Copying Control Versalogic bootrom.sys and vxworks to %s\n", VXBOOT_PATH);

      if ( updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/bootrom_versalogic.sys", VXBOOT_PATH "/bootrom.sys") == ERROR ||
           updatetrimaUtils::copyFileContiguous(UPDATE_PATH "/vxWorks_versalogic", VXBOOT_PATH "/vxWorks") == ERROR  )
      {
         printf("Install of OS image failed\n");
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
      printf("Removal of temporary OS image failed\n");
      return false;
   }

   //
   // Remove existing Trima files
   printf("Removing old Trima files...\n");
   fileSort(TRIMA_PATH,      FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(SAVEDATA_PATH,   FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(TOOLS_PATH,      FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(STRING_DIRECTORY, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);
   fileSort(GRAPHICS_PATH,   FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_clean_file);

   //
   // Uncompress the update file
   printf("Extracting the Trima software files...\n");

   if ( tarExtract(UPDATE_PATH "/trima.taz", TRIMA_PATH) == ERROR )
   {
      printf("Extraction of the Trima software failed.\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/trima.taz") == ERROR )
   {
      printf("Removal of Trima archive image failed\n");
      return false;
   }

   //
   // Uncompress the update file
   printf("Extracting the string.info files...\n");

   if ( tarExtract(UPDATE_PATH "/strings.taz", STRING_DIRECTORY) == ERROR )
   {
      printf("Extraction of the string.info files failed.\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/strings.taz") == ERROR )
   {
      printf("Removal of string archive image failed\n");
      return false;
   }

   //
   // Uncompress the update file
   printf("Extracting the font files...\n");

   if ( tarExtract(UPDATE_PATH "/fonts.taz", DROP_IN_FONTS_DIR) == ERROR )
   {
      printf("Extraction of the font files failed.\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/fonts.taz") == ERROR )
   {
      printf("Removal of font archive image failed\n");
      return false;
   }

   //
   // Uncompress the update file
   printf("Extracting the data files...\n");

   if ( tarExtract(UPDATE_PATH "/data.taz", DATA_DIRECTORY) == ERROR )
   {
      printf("Extraction of the data files failed.\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/data.taz") == ERROR )
   {
      printf("Removal of data archive image failed\n");
      return false;
   }

   //
   // Uncompress the update file
   printf("Extracting the graphics files...\n");

   if ( tarExtract(UPDATE_PATH "/graphics.taz", GRAPHICS_PATH) == ERROR )
   {
      printf("Extraction of the graphics files failed.\n");
      return false;
   }

   if ( remove(UPDATE_PATH "/graphics.taz") == ERROR )
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

      if ( cp(SAFETY_BOOT_PATH "/bootrom_ampro.sys", SAFETY_BOOTROM_IMAGE)    == ERROR ||
           cp(SAFETY_BOOT_PATH "/vxWorks_ampro", SAFETY_VXWORKS_IMAGE)    == ERROR )
      {
         printf("Install of OS image failed\n");
         return false;
      }
   }
   else
   {
      printf("Copying Safety Versalogic bootrom.sys and vxworks to %s\n", SAFETY_BOOT_PATH);

      if ( cp(SAFETY_BOOT_PATH "/vxWorks_versalogic", SAFETY_VXWORKS_IMAGE)     == ERROR ||
           cp(SAFETY_BOOT_PATH "/bootrom_versa_bootp.sys", SAFETY_BOOTROM_IMAGE)     == ERROR ||
           cp(SAFETY_BOOT_PATH "/vxWorks_versalogic_pxe", SAFETY_VXWORKS_PXE_IMAGE) == ERROR ||
           cp(SAFETY_BOOT_PATH "/bootrom_versa_pxe.sys", SAFETY_BOOTROM_PXE_IMAGE) == ERROR )
      {
         printf("Install of OS image failed\n");
         return false;
      }
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
      printf("removal of temporary OS image failed\n");
      return false;
   }


   // Uncompress the optional tools archive if it exists
   struct stat fileStat;

   if ( stat((char*)UPDATE_PATH "/engr_tools.taz", &fileStat) == OK )
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

bool installer::checkCRC5 ()
{

   // Set permissions in config directory
   fileSort(CONFIG_PATH, FILE_SORT_BY_DATE_ASCENDING, updatetrimaUtils::update_file_set_rdwrite);

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

bool installer::checkCRC6 ()
{
   //
   // Update configuration CRC values
   mkdir(CONFIG_CRC_PATH);

   softcrc("-filelist " FILELISTS_PATH "/caldat.files    -update " CONFIG_CRC_PATH  "/caldat.crc");
   softcrc("-filelist " FILELISTS_PATH "/config.files    -update " CONFIG_CRC_PATH  "/config.crc");
   softcrc("-filelist " FILELISTS_PATH "/hwdat.files     -update " CONFIG_CRC_PATH  "/hwdat.crc");
   softcrc("-filelist " FILELISTS_PATH "/rbcdat.files    -update " CONFIG_CRC_PATH  "/rbcdat.crc");
   softcrc("-filelist " FILELISTS_PATH "/terrordat.files -update " CONFIG_CRC_PATH  "/terrordat.crc");
   softcrc("-filelist " FILELISTS_PATH "/cassette.files  -update " CONFIG_CRC_PATH  "/cassette.crc");
   softcrc("-filelist " FILELISTS_PATH "/setconfig.files -update " CONFIG_CRC_PATH  "/setconfig.crc");
   softcrc("-filelist " FILELISTS_PATH "/graphics.files	-update "   PNAME_GUI_GRAPHICS_CRC);
   softcrc("-filelist " FILELISTS_PATH "/strings.files		-update "PNAME_STRING_CRC);
   softcrc("-filelist " FILELISTS_PATH "/fonts.files		-update "PNAME_FONT_CRC);
   softcrc("-filelist " FILELISTS_PATH "/data.files		-update "PNAME_DATA_CRC);
   softcrc("-filelist " FILELISTS_PATH "/safety.files		-update "TRIMA_PATH"/safety.crc");
   softcrc("-filelist " FILELISTS_PATH "/trima.files     -update " TRIMA_PATH       "/trima.crc");
   softcrc("-filelist " FILELISTS_PATH "/machine.files		-update "CONFIG_CRC_PATH"/machine.crc");

   // Set permissions in config directory
   updatetrimaUtils::update_file_set_rdonly(CONFIG_PATH);

   // Verify the installation CRC values
   if (verifyCrc("-filelist " FILELISTS_PATH "/caldat.files	-verify "CONFIG_CRC_PATH"/caldat.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/config.files	-verify "CONFIG_CRC_PATH"/config.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/hwdat.files		-verify "CONFIG_CRC_PATH"/hwdat.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/rbcdat.files	-verify "CONFIG_CRC_PATH"/rbcdat.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/terrordat.files	-verify "CONFIG_CRC_PATH"/terrordat.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/cassette.files	-verify "CONFIG_CRC_PATH"/cassette.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/setconfig.files	-verify "CONFIG_CRC_PATH"/setconfig.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/graphics.files	-verify "PNAME_GUI_GRAPHICS_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/strings.files   -verify "   PNAME_STRING_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/fonts.files     -verify "   PNAME_FONT_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/data.files      -verify "   PNAME_DATA_CRC) ||
       verifyCrc("-filelist " FILELISTS_PATH "/safety.files	-verify "TRIMA_PATH"/safety.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/trima.files		-verify "TRIMA_PATH"/trima.crc") ||
       verifyCrc("-filelist " FILELISTS_PATH "/machine.files	-verify "CONFIG_CRC_PATH"/machine.crc"))
   {
      return false;
   }

   return true;
}

bool installer::checkPasSettings ()
{
   bool           returnVal = false;

   CDatFileReader datfile(PNAME_CONFIGDAT);
   if ( datfile.Error() )
   {
      cerr << "Config file read error : " << datfile.Error() << endl;
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
      char        pltVolVarNameStr[64];
      sprintf (pltVolVarNameStr, "key_plt_volume_%d", prodNum + 1);
      const char* collectVolStr = datfile.Find("PRODUCT_TEMPLATES", pltVolVarNameStr);
      float       collectVol    = ((collectVolStr) ? atof(collectVolStr) : 0.0f);

      // If one of these settings was 0, something's wierd, so try the next one.
      if (percentPlasma == 0.0f || collectVol == 0.0f)
      {
         printf("Hmm... for PLT%d, percent plasma is %f, and collect vol is %f. Moving on...\n", prodNum + 1, percentPlasma, collectVol);
         continue;
      }

      // Figure out the max bag volume.
      float collectVolMax = floor(1200.0f * (percentPlasma / 100.0f));

      if (collectVol > collectVolMax)
      {
         printf("PLT%d has collect vol %f greater than max %f. Adjusting accordingly.\n", prodNum + 1, collectVol, collectVolMax);
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
   bool           returnVal     = false;
   bool           drbcAlertOn   = false;
   int            drbcThreshold = 275;

   CDatFileReader datfile(PNAME_CONFIGDAT);
   if ( datfile.Error() )
   {
      cerr << "Config file read error : " << datfile.Error() << endl;
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

      int  dose   = 0;
      int  rasVol = 0;

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
      printf("copy of %s failed\n", FILE_SETCONFIG_DAT);
      return;
   }

   attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
   fflush(stdout);
//   }
}


bool installer::rangeCheckConfig (int buildRef)
{
   // logStream << "Checking config.dat ranges" << endl;

   bool           madeChanges = false;
   char           loggingBuff[256];

   CDatFileReader datfile(PNAME_CONFIGDAT);
   if ( datfile.Error() )
   {
      // logStream << "Config file read error : " << datfile.Error() << endl;
      return false;
   }

   int rngCtr = 0;

   while ( rangeData[rngCtr].rangeType != END )
   {
      bool failCompare = false;

      // Special stuff for Product Definitions
      if ( strcmp(rangeData[rngCtr].section, "PRODUCT_DEFINITIONS") == 0, rangeData[rngCtr].dataKey[strlen(rangeData[rngCtr].dataKey) - 1] == '_' )
      {
         int  value  = 0;
         char keyBuf[256];
         int  varPos = strlen(rangeData[rngCtr].dataKey);
         strcpy(keyBuf, rangeData[rngCtr].dataKey);
         keyBuf[varPos + 1] = 0;

         for (keyBuf[varPos] = 'a'; keyBuf[varPos] <= 'o'; ++keyBuf[varPos])
         {
            if ( datfile.Find(rangeData[rngCtr].section, keyBuf) )
            {
               value = datfile.GetInt("PRODUCT_DEFINITIONS", keyBuf);

               if (value < 0 || value > atoi(rangeData[rngCtr].value))
               {
                  datfile.SetValue("PRODUCT_DEFINITIONS", keyBuf, "0");
                  sprintf(loggingBuff, "Config.dat value: %s failed range check, setting to 0\n", rangeData[rngCtr].dataKey);
                  updatetrimaUtils::logger(loggingBuff);
                  madeChanges = true;
               }
            }
         }
      }
      else if ( rangeData[rngCtr].rangeType == buildData[buildRef].rangeType &&
                datfile.Find(rangeData[rngCtr].section, rangeData[rngCtr].dataKey) )
      {
         const char* newVal = NULL;

         if ( rangeData[rngCtr].compareType == FORCE )
         {
            failCompare = true;
            newVal      = rangeData[rngCtr].value;
         }
         else if ( rangeData[rngCtr].valType == INT )
         {
            int value = datfile.GetInt(rangeData[rngCtr].section, rangeData[rngCtr].dataKey);

            if ( rangeData[rngCtr].compareType == MIN )
            {
               if ( value < atoi(rangeData[rngCtr].value) )
               {
                  failCompare = true;
                  newVal      = rangeData[rngCtr].value;
               }
            }
            else if ( rangeData[rngCtr].compareType == MAX )
            {
               if ( value > atoi(rangeData[rngCtr].value) )
               {
                  failCompare = true;
                  newVal      = rangeData[rngCtr].value;
               }
            }
            else if ( rangeData[rngCtr].compareType == NOT )
            {
               if ( value == atoi(rangeData[rngCtr].value) )
               {
                  failCompare = true;
                  newVal      = "0";
               }
            }
         }
         else    // it must be a float
         {
            float value = datfile.GetFloat(rangeData[rngCtr].section, rangeData[rngCtr].dataKey);

            if ( rangeData[rngCtr].compareType == MIN )
            {
               if ( value < atof(rangeData[rngCtr].value) )
               {
                  failCompare = true;
                  newVal      = rangeData[rngCtr].value;
               }
            }
            else if ( rangeData[rngCtr].compareType == MAX )
            {
               if ( value > atof(rangeData[rngCtr].value) )
               {
                  failCompare = true;
                  newVal      = rangeData[rngCtr].value;
               }
            }
         }

         if ( failCompare )
         {
            datfile.SetValue(rangeData[rngCtr].section, rangeData[rngCtr].dataKey, newVal);
            sprintf(loggingBuff, "Config.dat value: %s failed range check, setting to %s\n", rangeData[rngCtr].dataKey, newVal);
            updatetrimaUtils::logger(loggingBuff);
            madeChanges = true;
         }
      }

      failCompare = false;
      rngCtr++;
   }

   if ( madeChanges )
   {
      datfile.WriteCfgFile(PNAME_CONFIGDAT);
   }

   return madeChanges;
}


bool installer::checkPlasmaRB ()
{
   bool        retval = false;
   const char* configData;
   const char* swData;

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
               retval     = true;

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
      cerr << "This update path is not allowed on a VersaLogic Python Trima." << endl;
      retval = false;
   }
   else
   {
      cerr << "This update path is allowed." << endl;
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

   // copy the template config.dat to /config and make it writable
   if ( cp(TEMPLATES_PATH "/" FILE_CONFIG_DAT, PNAME_CONFIGDAT ".new") != ERROR )
   {
      attrib(PNAME_CONFIGDAT ".new", "-R");

      // open the new config.dat
      newdatfile.Initialize(PNAME_CONFIGDAT ".new");
      if ( newdatfile.Error() )
      {
         cerr << "Config file read error : " << newdatfile.Error() << endl;
         retval = false;
         goto LEAVEupdateConfigGeneric;
      }
   }
   else
   {
      printf("copy of config.dat template failed\n");
      retval = false;
      goto LEAVEupdateConfigGeneric;
   }


////////////////////////////////////////
// make the changes

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

         // get the value from the new config.dat
         strVal = newdatfile.GetString(section, cfLine.cpName());

         // compare the values
         if ( strVal.compare(newVal) != 0 )
         {
//              cerr << "value different for " << cfLine.cpName() << endl;
            newdatfile.SetValue(section, cfLine.cpName(), newVal);
         }
      }

   }
//    cerr << endl;

   fclose(configFilefp);

////////////////////////////////////////

   // if it ended without error, write the data and copy it over the old config.dat
   if ( readState >= 0 )
   {
      // write the modified config.dat.new
      if ( !newdatfile.WriteCfgFile(FILE_CONFIG_DAT ".new") )
      {
         cerr << "config.dat.new write failed" << endl;
         retval = false;
         goto LEAVEupdateConfigGeneric;
      }
      else
      {
         cerr << "config.dat.new write worked" << endl;
      }

      ///////////////////////////////////
      // copy config.dat.new over config.dat
      attrib(PNAME_CONFIGDAT, "-R");
      if ( cp(PNAME_CONFIGDAT ".new", PNAME_CONFIGDAT) == ERROR )
      {
         printf("copy of %s.new failed\n", PNAME_CONFIGDAT);
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
      CFileLine           newLine(buffer);

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

      found  = true;
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



//////////////////////////////////////////////////////////////////////////////////////
//  The main line of update
//////////////////////////////////////////////////////////////////////////////////////
int installer::upgrade (versionStruct& fromVer, versionStruct& toVer)
{
   bool retval          = 0;

   int  cntr            = 0;
   int  buildRef        = 0;
   int  largestMinorRev = 0;
   int  largestBuild    = 0;

   char loggingBuff[256];

   sprintf(loggingBuff, "Looking up build info for buildNum = %d.%d.%d\n", toVer.majorRev, toVer.minorRev, toVer.buildNum);
   updatetrimaUtils::logger(loggingBuff);

   // get the upgrade data for the version we're going to
   // this gets the build info for the version that matches the major/minor revs and
   // has the largest build number that isn't larger than the new build
//    while ( sizeof(buildData) > 0 && strcmp(buildData[cntr].buildNum, "END") != 0 )
   while ( strcmp(buildData[cntr].buildNum, "END") != 0 )
   {
      int toMajorRev = 0;
      int toMinorRev = 0;
      int toBuild    = 0;

      updatetrimaUtils::parseRevision(buildData[cntr].buildNum, toMajorRev, toMinorRev, toBuild);

//        if ( toMajorRev == toVer.majorRev && toMinorRev == toVer.minorRev)
      if ( toMajorRev == toVer.majorRev )
      {
         if ( toMinorRev >= largestMinorRev && toVer.minorRev >= toMinorRev )
         {
            largestMinorRev = toMinorRev;
            buildRef        = cntr;
            sprintf(loggingBuff, "Build info best match: %d.%d.%d\n", toMajorRev, toMinorRev, toBuild);
            updatetrimaUtils::logger(loggingBuff, true);

            if (toBuild > largestBuild && toVer.buildNum >= toBuild)
            {
               sprintf(loggingBuff, "Build info best match: %d.%d.%d\n", toMajorRev, toMinorRev, toBuild);
               updatetrimaUtils::logger(loggingBuff, true);
               buildRef     = cntr;
               largestBuild = toBuild;
            }
         }

      }

      cntr++;
   }

   if ( !buildRef )
   {
      updatetrimaUtils::logger("Build info not found - aborting install\n");
      return(-1);
   }

   sprintf(loggingBuff, "Build best match found for the version given: %s\n", buildData[buildRef].buildNum);
   updatetrimaUtils::logger(loggingBuff);

   // check if this upgrade is allowed
   if ( !allowedUpgrade(buildData[buildRef].ampro, buildData[buildRef].versalogic, buildData[buildRef].python, buildData[buildRef].ocelot) )
   {
      updatetrimaUtils::logger("Upgrade to this version not allowed on this hardware\n");
      return(-1);
   }
   updatetrimaUtils::logger("Upgrade to this version is allowed on this hardware\n");

   updatetrimaUtils::update_file_set_rdwrite(CONFIG_PATH);

   //
   // Make /vxboot and /trima partitions writable
   DOS_VOLUME_DESC_ID pVolDesc;

   char*              vxboot = VXBOOT_PATH;
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

   // Remove any old copy of features.bin in templates
   updatetrimaUtils::logger("Removing any old copy of features.bin in templates\n");
   struct stat featuresFileStat;

   if ( stat((char*)TEMPLATES_PATH "/" FILE_FEATURES, &featuresFileStat) == OK )
   {
      remove(TEMPLATES_PATH "/" FILE_FEATURES);
   }

   updatetrimaUtils::logger("Extract the Trima files\n");
   // extract the Trima files
   if ( buildData[buildRef].extractType == 5 )
   {
      retval = extractUpdateFiles5();
      // logStream << "Extract method 5" << endl;
   }
   else
   {
      retval = extractUpdateFiles6();
      // logStream << "Extract method 6" << endl;
   }

   if ( !retval )
   {
      updatetrimaUtils::logger("File extraction failed\n");
      return(-1);
   }

   attrib(TEMP_PATH, "-R");

   // Update the configuration files
   // do this before all other config.dat changes
   updatetrimaUtils::logger("Update the configuration files\n");
   if ( !updateConfigGeneric() )
   {
      updatetrimaUtils::logger("Configuration update failed\n");
      return(-1);
   }

   // make sure any of the config.dat values are not out of range
   updatetrimaUtils::logger("Range checking config.dat for this version\n");
   if ( rangeCheckConfig(buildRef) )
   {
      updatetrimaUtils::logger("Made config.dat range adjustments\n");
   }

   // update cal files
   if ( buildData[buildRef].calFileType == 5 )
   {
      updatetrimaUtils::logger("Update cal ver 5\n");
      updateCal5();
   }
   else
   {
      updatetrimaUtils::logger("Update cal ver 6\n");
      updateCal6();
   }

//    updatetrimaUtils::logger("Do a bunch of stuff\n");
// stuff to always do
   updatetrimaUtils::logger("Updating Globvars\n");
   updateGlobVars();
   updatetrimaUtils::logger("Updating RBC\n");
   updateRBC();
   updatetrimaUtils::logger("Updating Hardware\n");
   updateHW();
   updatetrimaUtils::logger("Updating Software\n");
   updateSW();
   updatetrimaUtils::logger("Updating Terror\n");
   updateTerror();
   updatetrimaUtils::logger("Updating Sounds\n");
   updateSounds();
   updatetrimaUtils::logger("Updating Cassette\n");
   updateCassette();
   updatetrimaUtils::logger("Updating Vista\n");
   updateVista();

   // do different stuff for 6.3
   if ( buildData[buildRef].setConfigCopy )
   {
      updatetrimaUtils::logger("force copy setconfig file\n");
      forceSetConfig();
   }
   else
   {
      updatetrimaUtils::logger("update setconfig file\n");
      updateSetConfig();
   }

   // if coming from 5.1.0-3 update trap files if called for
   if ( buildData[buildRef].updateTrapFiles && fromVer.majorRev == 6 )
   {
      updatetrimaUtils::logger("update trap files\n");
      copyTrapFiles();
   }

   if ( buildData[buildRef].adjPostcount )
   {
      updatetrimaUtils::logger("update post count\n");
      updatePostCount();
   }

   if ( buildData[buildRef].adjPASRAS )
   {
      updatetrimaUtils::logger("check PAS & RAS settings\n");
      checkPasSettings();
      checkRasSettings();
   }

   // only do for 5.X to 5.X
   if ( (fromVer.majorRev == 6 || fromVer.majorRev == 7) &&
        (toVer.majorRev == 6 || toVer.majorRev == 7) )
   {
      updatetrimaUtils::logger("check plasma RB\n");
      checkPlasmaRB();
   }

   // update & check the CRCs
   if ( buildData[buildRef].calFileType == 5 )
   {
      updatetrimaUtils::logger("checkCRC5\n");
      retval = checkCRC5();
   }
   else
   {
      updatetrimaUtils::logger("checkCRC6\n");
      retval = checkCRC6();
   }

   if ( !retval )
   {
      return(-1);
   }

LEAVEROUTINE:

//    attrib( TEMP_PATH,"R" );

   updatetrimaUtils::logger("Trima software update complete\n");
//    printf("Trima software update complete.\n");

   // Delete the update script so that it doesn't run again on the subsequent boot if the GTS guy
   // is still holding down the buttons.
   updatetrimaUtils::logger("Removing update script\n");
//    printf("Removing update script.\n");
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

/* FORMAT HASH 563801c87dd8a816568b0c25865c0b23 */
