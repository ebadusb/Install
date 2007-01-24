/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 *
 * $Header: H:/BCT_Development/Install/Downgrade/v5.2_to_v5.1/rcs/updatetrima.cpp 1.1 2007/01/23 17:39:35Z jheiusb Exp jheiusb $
 * $Log: updatetrima.cpp $
 * Revision 1.34  2006/12/08 18:16:41Z  jheiusb
 * 7817 -- removed Plt Volume Type, not used anymore
 * Revision 1.33  2006/10/19 16:22:27Z  jheiusb
 * 7440- upgare for 5.1->5.2
 * Revision 1.32  2006/08/09 21:07:45Z  dslausb
 * IT 7682 - Changed update to check for setconfig.dat file version.
 * Revision 1.31  2006/08/09 16:53:55Z  dslausb
 * IT 7682 - Changed to 2-file cassette entry functionality
 * Revision 1.30  2006/05/01 21:31:46Z  rm70006
 * IT 7051.  Remove update script after installation.
 * Revision 1.29  2006/05/01 19:30:32Z  rm70006
 * IT 7051.  Remove clinical and test build files during upgrade.
 * Revision 1.28  2006/04/20 18:27:13Z  dslausb
 * Fixed some CRC issues and included cassette.dat in the update script.
 * Revision 1.27  2004/02/12 17:32:31Z  ms10234
 * 5812 - change include file name.
 * Revision 1.26  2004/02/05 23:28:47Z  ms10234
 * 5811 - touchscreen calibration error
 * Revision 1.25  2004/01/30 23:04:38Z  ms10234
 * 6050 5811 - upgrade from v5 to v5.1
 * Revision 1.24  2004/01/06 16:53:53Z  rm70006
 * IT 5858.  Add terror config file to update.
 * Revision 1.23  2003/12/29 15:51:54Z  rm70006
 * IT 5858.  Add terror config files to install.
 * Revision 1.22  2003/12/05 16:01:18Z  jl11312
 * - enable file write protections
 * Revision 1.21  2003/12/02 19:53:13Z  jl11312
 * - corrected handling of sw.dat updates
 * Revision 1.20  2003/11/20 18:07:17Z  jl11312
 * - handle updates to sw.dat
 * - clear tools directory on upgrade
 * Revision 1.19  2003/11/13 15:11:02Z  jl11312
 * - update hw.dat if config copy is out of date or missing
 * Revision 1.18  2003/06/26 22:43:01Z  jl11312
 * - added crc function to updateTrima
 * Revision 1.17  2003/06/19 18:55:53Z  ms10234
 * 5809 - run the update program at low priority so as not to affect anyone important
 * Revision 1.16  2003/06/16 22:11:03Z  jl11312
 * - corrected copy file function
 * Revision 1.15  2003/05/13 15:48:21Z  jl11312
 * - added handling of CRC files during installation
 * Revision 1.14  2003/05/12 16:06:26Z  ms10234
 * 5809 - Update status logging.
 * Revision 1.13  2003/04/09 20:12:45Z  ms10234
 * Updates for Trima install
 * Revision 1.12  2003/03/07 22:52:15Z  td07711
 * remove periodic safety crc files, use filenames.h
 * Revision 1.11  2003/02/26 15:16:55Z  ms10234
 * - Added code to remove the safety CRCs after update
 * - Added code to remove the archived files from the update directory if a successful install
 * - Added rbc.dat to /config during install
 * Revision 1.10  2002/12/10 19:56:06Z  ms10234
 * Revision 1.9  2002/12/04 22:57:05Z  ms10234
 * use a bigger read buffer
 * Revision 1.8  2002/11/20 20:20:41Z  ms10234
 * Add section to save old vxWorks image
 * Revision 1.7  2002/11/19 19:12:38Z  ms10234
 * changed name of trima zip file
 * Revision 1.6  2002/11/19 18:03:06Z  ms10234
 * Added support for the vxWorks and bootrom images.
 * Revision 1.5  2002/11/19 15:22:34Z  ms10234
 * Added external declarations
 * Revision 1.4  2002/11/13 22:05:59Z  ms10234
 * Added reference for targzextract functions
 * Revision 1.3  2002/11/08 16:36:25Z  jl11312
 * - changed argument to gzopen
 * Revision 1.2  2002/11/08 00:04:12  ms10234
 * Added status output
 * Revision 1.1  2002/11/06 23:27:54Z  ms10234
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <cbioLib.h>
#include <dosFsLib.h>
#include <private/dosFsLibP.h>
#include <bootLib.h>
#include <sysLib.h>
#include <taskLib.h>
#include <unistd.h>
#include <usrLib.h>
#include <stdio.h>
#include <fcntl.h>

#include "zlib.h"
#include "filenames.h"
#include "filesort.h"

#include "targzextract.c"
#include "crc.c"

#include "datfile.h"

#include "trima_opsys.h"

#ifdef __cplusplus
extern "C" { 
#endif

int cp(const char * from, const char * to);
int xcopy (const char * src, const char *dest);
int copyFileContiguous(const char * from, const char * to);
int unzipFile(const char * from, const char * to);

void updateTrima();

int softcrc(const char * options);

#ifdef __cplusplus
};
#endif


int copyFileContiguous(const char * from, const char * to)
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
      fprintf( stdout, "copying %s to %s: file length = %ld bytes\n", from, to, fileStat.st_size);

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
         fprintf( stdout, "Copy failed (%ld bytes written)\n", bytesCopied);
      }
      else
      {
         result = bytesCopied;
      }
   }
   else
   {
      if ( fromFD < 0 ) perror(from);
      if ( toFD < 0 ) perror(to);
   }

   if ( fromFD >= 0 ) close(fromFD);
   if ( toFD >= 0 ) close(toFD);

   return result;
}

int unzipFile(const char * from, const char * to)
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

      fprintf( stdout, "\tUncompression complete\n" );
      gzclose(fromFD);
      close(toFD);
       
      return 1;
   }
   else
   {
      if ( fromFD < 0 ) perror(from);
      if ( toFD < 0 )   perror(to);
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////////
////////////////////////  PRIVATE FUNCTS  /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

extern "C" STATUS xdelete(const char * fileName);
static FileCallBackStatus update_clean_file(const char * fullPathName)
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

static FileCallBackStatus update_file_set_rdwrite(const char * fullPathName)
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

static FileCallBackStatus update_file_set_rdonly(const char * fullPathName)
{
    struct stat fileStat;

    if ( stat((char *)fullPathName, &fileStat) == OK &&
          (fileStat.st_mode & S_IFDIR) != 0 )
    {
        fileSort(fullPathName, FILE_SORT_BY_DATE_ASCENDING, update_file_set_rdwrite);
        attrib(fullPathName, "-R");
    }
    else
    {
        attrib(fullPathName, "+R");
    }

    return FILE_CALLBACK_CONTINUE;
}

const char * findSetting(const char * setting, FILE * fp)
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

const char * findSetting(const char * setting, const char * fileName)
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

enum { MaxIDStringLength = 256 };

//
// Scan memory for specified character pattern.  Used to find ID strings
// in BIOS ROM and expansion ROM memory areas.
//
static const char * FindIDString(
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

//
// Scan system BIOS memory area for ID strings
//
static bool IsVendor( const char * vendor )
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
               return true;
         }

         stringInfo = FindIDString(romPtr, BIOSLength, romIDString[stringIdx], searchStartIdx, stringLength);
      }

      stringIdx += 1;
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
//
//  Update the dat files.....
//
///////////////////////////////////////////////////////////////////////////////////
//              Config.dat
///////////////////////////////////////////////////////////////////////////////////
void updateConfig()
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
      
   // check if 5.P by looking for a new parameter.......
   if ( datfile.Find("PROCEDURE_CONFIG", "key_mss_plt_on") )
   {
      cerr << "v5.2 config.dat file found.  No conversion needed" << endl;
      return;
   }
   cerr << "v5.1 config.dat file found.  Conversion needed" << endl;
   //////////////////////////////////////////////////////////////////////////////////
   //                 5.1-->5.P changes
   //////////////////////////////////////////////////////////////////////////////////
   {

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

      //datfile.RemoveLine( "PROCEDURE_CONFIG", "key_greater_than_vol" );
   }

   ////////////////////////////////////////////////////////////////////////////
   //        New values for v5.p ...
   //        None Yet....
   ////////////////////////////////////////////////////////////////////////////
   {
      // datfile.AddLine( "PROCEDURE_CONFIG", "key_rinseback_protocol", "0" );
   }

   datfile.WriteCfgFile(FILE_CONFIG_DAT);

   cerr << "config.dat file converted." << endl;
}
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//              CAL.dat
///////////////////////////////////////////////////////////////////////////////////
void updateCal()
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
      
   if ( !datfile.Find("TOUCHSCREEN","screen_horizontal_size") )
   {
      cerr << " ... pre-v5.1 cal.dat file found.  Unable to Convert!  ending..." << endl;
      return;
   }
   cerr << "v5.2 " << FILE_CAL_DAT << " file found.  No conversion needed" << endl;
}
///////////////////////////////////////////////////////////////////////////////////


const char *currVersion = "";
const char *newVersion  = "";

///////////////////////////////////////////////////////////////////////////////////
//              RBC.dat
///////////////////////////////////////////////////////////////////////////////////
void updateRBC()
{
   // Put the rbc.dat file in the correct location.
   attrib(CONFIG_PATH FILE_RBC_DAT, "-R");
   if ( cp( TEMPLATES_PATH "/" FILE_RBC_DAT, CONFIG_PATH "/" FILE_RBC_DAT ) == ERROR )
   {
      fprintf( stdout, "copy of rbc.dat failed\n" );
      return;
   }
   attrib(CONFIG_PATH FILE_RBC_DAT, "+R");

}
//////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//              HW.dat
///////////////////////////////////////////////////////////////////////////////////
void updateHW()
{
   // Replace hw.dat if the version number has changed
   currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_HW_DAT);
   newVersion; 

   if ( IsVendor( "Ampro" ) )
      newVersion = findSetting("file_version=", TEMPLATES_PATH "/hw_ampro.dat");
   else
      newVersion = findSetting("file_version=", TEMPLATES_PATH "/hw_versalogic.dat");

   if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
   {
      fprintf(stdout, "Updating hw.dat to new version %s from existing version %s...\n", newVersion, currVersion);

      attrib(CONFIG_PATH "/" FILE_HW_DAT, "-R");
      if ( IsVendor( "Ampro" ) )
      {
         if ( cp( TEMPLATES_PATH "/hw_ampro.dat", CONFIG_PATH "/" FILE_HW_DAT ) == ERROR )
         {
               fprintf( stdout, "copy of hw_ampro.dat failed\n" );
               return;
         }
      }
      else
      {
         if ( cp( TEMPLATES_PATH "/hw_versalogic.dat", CONFIG_PATH "/" FILE_HW_DAT) == ERROR )
         {
            fprintf( stdout, "copy of hw_versalogic.dat failed\n" );
            return;
         }
      }
      attrib(CONFIG_PATH "/" FILE_HW_DAT, "+R");
      fflush(stdout);
   }

}
////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//              SW.dat
///////////////////////////////////////////////////////////////////////////////////
void updateSW()
{
   //
   // Replace sw.dat if the version number has changed
   currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_SW_DAT);
   newVersion = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SW_DAT);
   if ( newVersion &&
         ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
   {
      fprintf(stdout, "Updating sw.dat to new version %s from existing version %s...\n", newVersion, currVersion);
      attrib(CONFIG_PATH "/" FILE_SW_DAT, "-R");

      if ( cp( TEMPLATES_PATH "/" FILE_SW_DAT, CONFIG_PATH "/" FILE_SW_DAT ) == ERROR )
      {
         fprintf( stdout, "copy of %s failed\n", FILE_SW_DAT );
         return;
      }

      attrib(CONFIG_PATH "/" FILE_SW_DAT, "+R");
      fflush(stdout);
   }

}
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
//              TERROR_CONFIG.dat
//////////////////////////////////////////////////////////////////////////////////////
void updateTerror()
{

   //
   // Replace terror_config.dat if the version number has changed
   currVersion = findSetting("file_version=", TERROR_CONFIG_FILE);
   newVersion = findSetting("file_version=", TEMPLATES_PATH "/terror_config.dat");
    
   if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
   {
      fprintf(stdout, "Updating terror_config.dat to new version %s from existing version %s...\n", newVersion, currVersion);
      attrib(TERROR_CONFIG_FILE, "-R");

      if ( cp( TEMPLATES_PATH "/terror_config.dat", CONFIG_PATH "/terror_config.dat" ) == ERROR )
      {
         fprintf( stdout, "copy of terror_config.dat failed\n" );
         return;
      }

      attrib(CONFIG_PATH "/terror_config.dat", "+R");
      fflush(stdout);
   }

}
//////////////////////////////////////////////////////////////////////////////////////
      
   
//////////////////////////////////////////////////////////////////////////////////////
//              CASSETTE.dat
//////////////////////////////////////////////////////////////////////////////////////
void updateCassette()
{
   //
   // Replace cassette.dat if the version number has changed
   currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_CASSETTE_DAT);
   newVersion = findSetting("file_version=", TEMPLATES_PATH "/" FILE_CASSETTE_DAT);
    
   if ( newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
   {
      fprintf(stdout, "Updating %s to new version %s from existing version %s...\n", FILE_CASSETTE_DAT, newVersion, currVersion);
      attrib(CONFIG_PATH "/" FILE_CASSETTE_DAT, "-R");

      if ( cp( TEMPLATES_PATH "/" FILE_CASSETTE_DAT, CONFIG_PATH "/" FILE_CASSETTE_DAT ) == ERROR )
      {
         fprintf( stdout, "copy of %s failed\n", FILE_CASSETTE_DAT );
         return;
      }

      attrib(CONFIG_PATH "/" FILE_CASSETTE_DAT, "+R");
      fflush(stdout);
   }
}
//////////////////////////////////////////////////////////////////////////////////////
      

//////////////////////////////////////////////////////////////////////////////////////
//              SETCONFIG.dat
//////////////////////////////////////////////////////////////////////////////////////
void updateSetConfig()
{
   //
   // these are the customer selected sets.... dont overwrite if it exists! 

   currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_SETCONFIG_DAT);
   //  newVersion = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SETCONFIG_DAT);

   // if the file isnt there....
   if (currVersion == NULL)
   {
       fprintf(stdout, "Adding %s ...\n", FILE_SETCONFIG_DAT);
       // attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "-R");

       if ( cp( TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT ) == ERROR )
       {
          fprintf( stdout, "copy of %s failed\n", FILE_SETCONFIG_DAT );
          return;
       }

       attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
       fflush(stdout);

   }   else {

       fprintf(stdout, "%s already exixst ...\n", FILE_SETCONFIG_DAT);

   }

    
}
///////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
//              Global  vars
//////////////////////////////////////////////////////////////////////////////////////
void updateGlobVars() 
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

   if ( !datfile.Find( "EXTERNALIP" ) )
   {
      cerr << "pre-v5.1 globvars file found Unable to Convert... ending" << endl;
      return;
   }

   cerr << "v5.2 globvars file found.  No conversion needed" << endl;
}
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
//  The main line of update
//////////////////////////////////////////////////////////////////////////////////////
void updateTrima()
{
   //
   // Make sure we don't interrupt anybody else who is running ...
   //
   taskPrioritySet( taskIdSelf(), 250 );

    //
    // Make /vxboot and /trima partitions writable
    DOS_VOLUME_DESC_ID  pVolDesc;
    pVolDesc = dosFsVolDescGet(VXBOOT_PATH, NULL);
    if ( !pVolDesc )
    {
        perror(VXBOOT_PATH);
        return;
    }

    cbioModeSet(pVolDesc->pCbio, O_RDWR);

    pVolDesc = dosFsVolDescGet(TRIMA_PATH, NULL);
    if ( !pVolDesc )
    {
        perror(TRIMA_PATH);
        return;
    }

    cbioModeSet(pVolDesc->pCbio, O_RDWR);   
   
   //
   // Extract the update files
   fprintf( stdout, "Extracting updateTrima ...\n" );
   if ( tarExtract( UPDATE_PATH "/updateTrima.taz", UPDATE_PATH ) == ERROR )
   {
      fprintf( stdout, "Extraction of update files failed\n" );
      return;
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
      fprintf( stdout, "Saving the old OS image..." );
      attrib(VXBOOT_PATH "/vxWorks.old", "-R");
      if ( cp( VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.old" ) == ERROR )
      {
         fprintf( stdout, "Archive of old OS image failed\n" );
      }

   }
    
   //
   // Store the new files in the proper position
   fprintf( stdout, "Extracting the OS image...\n" );
   if ( tarExtract( UPDATE_PATH "/vxboot.taz", UPDATE_PATH ) == ERROR )
   {
      fprintf( stdout, "Extraction of OS image failed\n" );
      return;
   }

   attrib(VXBOOT_PATH "/bootrom.sys", "-R");
   attrib(VXBOOT_PATH "/vxWorks", "-R");
   if ( copyFileContiguous( UPDATE_PATH "/bootrom.sys", VXBOOT_PATH "/bootrom.sys" ) == ERROR ||
        copyFileContiguous( UPDATE_PATH "/vxWorks"    , VXBOOT_PATH "/vxWorks"     ) == ERROR )
   {
      fprintf( stdout, "Install of OS image failed\n" );
      return;
   }
   if ( remove( UPDATE_PATH "/bootrom.sys" ) == ERROR ||
        remove( UPDATE_PATH "/vxWorks"     ) == ERROR ||
        remove( UPDATE_PATH "/vxboot.taz"  ) == ERROR )
   {
      fprintf( stdout, "Removal of temporary OS image failed\n" );
      return;
   }
    
   //
   // Remove existing Trima files
   fprintf( stdout, "Removing old Trima files...\n" );
   fileSort(TRIMA_PATH,    FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
   fileSort(SAVEDATA_PATH, FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
   fileSort(TOOLS_PATH,    FILE_SORT_BY_DATE_ASCENDING, update_clean_file);

   // IT 7051.  Remove clinical file if exists.
   remove(CLINICAL_BUILD);
   remove(TEST_BUILD);
   
   //
   // Uncompress the update file
   fprintf( stdout, "Extracting the Trima software files...\n" );
   if ( tarExtract( UPDATE_PATH "/trima.taz", TRIMA_PATH ) == ERROR )
   {
      fprintf( stdout, "Extraction of the Trima software failed.\n" );
      return;
   }
   if ( remove( UPDATE_PATH "/trima.taz" ) == ERROR )
   {
      fprintf( stdout, "Removal of Trima archive image failed\n" );
      return;
   }

   
   // Update the configuration files ...
   ///////////////////////////////////////////////////////////////////////////////////
   //              Config.dat
   ///////////////////////////////////////////////////////////////////////////////////
   updateConfig();
   ///////////////////////////////////////////////////////////////////////////////////


   ///////////////////////////////////////////////////////////////////////////////////
   //              CAL.dat
   ///////////////////////////////////////////////////////////////////////////////////
   updateCal();
   ///////////////////////////////////////////////////////////////////////////////////
      
   
   ///////////////////////////////////////////////////////////////////////////////////
   //              Global
   ///////////////////////////////////////////////////////////////////////////////////
   updateGlobVars();
   ///////////////////////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////////////////////
   //              RBC.dat
   ///////////////////////////////////////////////////////////////////////////////////
   updateRBC();
   //////////////////////////////////////////////////////////////////////////////////

   
   ///////////////////////////////////////////////////////////////////////////////////
   //              HW.dat
   ///////////////////////////////////////////////////////////////////////////////////
   updateHW();
   ////////////////////////////////////////////////////////////////////////////////////


   ///////////////////////////////////////////////////////////////////////////////////
   //              SW.dat
   ///////////////////////////////////////////////////////////////////////////////////
   updateSW();
   //////////////////////////////////////////////////////////////////////////////////////


   //////////////////////////////////////////////////////////////////////////////////////
   //              TERROR_CONFIG.dat
   //////////////////////////////////////////////////////////////////////////////////////
   updateTerror();
   //////////////////////////////////////////////////////////////////////////////////////
      
      
   //////////////////////////////////////////////////////////////////////////////////////
   //              CASSETTE.dat
   //////////////////////////////////////////////////////////////////////////////////////
   updateCassette();
   //////////////////////////////////////////////////////////////////////////////////////
      

   //////////////////////////////////////////////////////////////////////////////////////
   //              SETCONFIG.dat
   //////////////////////////////////////////////////////////////////////////////////////
   updateSetConfig();
   ///////////////////////////////////////////////////////////////////////////////////////


   // Set permissions in config directory
   fileSort(CONFIG_PATH, FILE_SORT_BY_DATE_ASCENDING, update_file_set_rdonly);

   //
   // Update configuration CRC values
   mkdir(CONFIG_CRC_PATH);
   softcrc("-filelist " FILELISTS_PATH "/caldat.files    -update " CONFIG_CRC_PATH "/caldat.crc");
   softcrc("-filelist " FILELISTS_PATH "/config.files    -update " CONFIG_CRC_PATH "/config.crc");
   softcrc("-filelist " FILELISTS_PATH "/hwdat.files     -update " CONFIG_CRC_PATH "/hwdat.crc");
   softcrc("-filelist " FILELISTS_PATH "/machine.files   -update " CONFIG_CRC_PATH "/machine.crc");
   softcrc("-filelist " FILELISTS_PATH "/rbcdat.files    -update " CONFIG_CRC_PATH "/rbcdat.crc");
   softcrc("-filelist " FILELISTS_PATH "/terrordat.files -update " CONFIG_CRC_PATH "/terrordat.crc");
   softcrc("-filelist " FILELISTS_PATH "/cassette.files  -update " CONFIG_CRC_PATH "/cassette.crc");
   softcrc("-filelist " FILELISTS_PATH "/setconfig.files  -update " CONFIG_CRC_PATH "/setconfig.crc");

   
   // Verify the installation CRC values
   if ( softcrc("-filelist " FILELISTS_PATH "/trima.files -verify  "    TRIMA_PATH      "/trima.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/safety.files -verify "    TRIMA_PATH      "/safety.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/caldat.files -verify "    CONFIG_CRC_PATH "/caldat.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/config.files -verify "    CONFIG_CRC_PATH "/config.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/hwdat.files -verify "     CONFIG_CRC_PATH "/hwdat.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/machine.files -verify "   CONFIG_CRC_PATH "/machine.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/rbcdat.files -verify "    CONFIG_CRC_PATH "/rbcdat.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/terrordat.files -verify " CONFIG_CRC_PATH "/terrordat.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/cassette.files -verify "  CONFIG_CRC_PATH "/cassette.crc") != 0 ||
		softcrc("-filelist " FILELISTS_PATH "/setconfig.files -verify "  CONFIG_CRC_PATH "/setconfig.crc") != 0)
   {
      fprintf(stdout, "CRC check of installed software failed\n");
      return;
   }

   fprintf( stdout, "Trima software update complete.\n" );

   // Delete the update script so that it doesn't run again on the subsequent boot if the GTS guy
   // is still holding down the buttons.
   fprintf(stdout, "removing update script.\n");
   remove( UPDATE_PATH "/updatetrima" );
   remove( UPDATE_PATH "/updatetrima.taz" );

   fflush( stdout );

   trimaSysStartDiskSync();
}
                          
