/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 *
 * $Header: //bctquad3/home/BCT_Development/Install/Trima_v6.0/rcs/updatetrima.cpp 1.1 2008/10/23 20:45:57Z jsylusb Exp jsylusb $
 * $Log: updatetrima.cpp $
 * Revision 1.56  2008/10/01 14:01:27  dslausb
 * String.info files are in a different place.
 * Revision 1.55  2008/10/01 19:49:31Z  dslausb
 * Move string files to avoid CRC issues
 * Revision 1.54  2008/09/25 19:49:10Z  dslausb
 * IT 8867 - Add extra PLT vol check in update script
 * Revision 1.53  2008/09/24 18:52:55Z  dslausb
 * Made sure that config.dat update will now allow excess PLT volumes
 * Revision 1.52  2008/07/21 17:43:39Z  ssunusb
 * IT 8741 - Refactored updateConfig to minimize multiple returns
 * Revision 1.51  2008/07/08 18:52:43Z  dslausb
 * IT 8735 - Add four more plt procedures
 * Revision 1.50  2008/07/01 22:11:54Z  ssunusb
 * IT 8741 - Force post count config setting to be 100K if < 100K
 * Revision 1.49  2008/05/21 21:49:05Z  dslausb
 * IT 8473 - Added residual volumes to run summary screen
 * Revision 1.48  2008/05/08 20:17:54Z  dslausb
 * IT 7973 - Add saline rinseback button
 * Revision 1.47  2008/05/07 16:02:40Z  dslausb
 * IT 8076 - Add more available procedures
 * Revision 1.46  2008/04/23 21:04:29Z  dslausb
 * IT 8590 - Crit vs Hemoglobin
 * Revision 1.45  2008/04/18 18:58:06Z  ssunusb
 * IT8482 - Make trima sounds configurable
 * Revision 1.44  2008/04/11 21:32:25Z  dslausb
 * Added some compiler directives to get this to easily compile under Trima 5.1 for OS upgrades.
 * Revision 1.43  2008/04/07 20:07:34Z  dslausb
 * Add config setting in there.
 * Revision 1.42  2008/01/23 17:36:30Z  dslausb
 * IT 8435 - VxWorks 5.5 Checkin
 * Revision 1.41  2007/09/18 20:24:46Z  jheiusb
 * 7992 -- add new config values to update
 * Revision 1.40  2007/06/11 15:34:16Z  dslausb
 * Fixed a type that's been bugging me
 * Revision 1.39  2007/05/24 19:00:22Z  dslausb
 * IT 8040 - Forgot to check in update change.
 * Revision 1.38  2007/05/14 16:11:31Z  jheiusb
 * IT 7974 -- keep in sync with the config.dat changes
 * Revision 1.37  2007/04/09 21:14:34Z  jheiusb
 * add the drbc thrshold value to config.dat
 * Revision 1.36  2007/02/23 17:19:12Z  jheiusb
 * 5687 -- add air removal tag for upgrade
 * Revision 1.35  2006/12/11 20:22:31Z  jheiusb
 * 7817 fix due to config file change
 * Revision 1.34  2006/12/06 20:20:45Z  jheiusb
 * add 5.r config stuff
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
#include <math.h>

#include "zlib.h"
#include "filenames.h"
#include "filesort.h"

#include "targzextract.c"
#include "crc.c"

#include "datfile.h"
#include "configdef.h"

#define __COMPILE_FOR_VX_54__ 	// Define this variable if you want to compile
								// this with Trima 5.1 or 5.2 to upgrade from
								// 5.1 or 5.2 to 6.0.


// About this extra stuff: these definitions are normally located
// in the "filenames.h directory.  However, for 5.1 they were not
// defined.  It's desireable to be able to just drop this update file
// into an old sandbox and compile it for the VxWorks upgrade,
// so now you just have to change the line above and you're good.
// Otherwise, this stuff remains invisible.
// 
#ifdef __COMPILE_FOR_VX_54__

	#include "trima_opsys.h"

    #ifndef TEMPLATES_PATH
		#define TEMPLATES_PATH TRIMA_PATH "/config/templates"
	#endif // #ifndef TEMPLATES_PATH

	#ifndef FILE_CASSETTE_DAT
		#define FILE_CASSETTE_DAT "cassette.dat"
	#endif // #ifndef FILE_CASSETTE_DAT

	#ifndef FILE_SETCONFIG_DAT
		#define FILE_SETCONFIG_DAT "setconfig.dat"
	#endif // #ifndef FILE_SETCONFIG_DAT

	#ifndef FILE_SOUNDS_DAT
		#define FILE_SOUNDS_DAT "sounds.dat"
	#endif // #ifndef FILE_SOUNDS_DAT

	#ifndef PNAME_SOUNDSDAT
		#define PNAME_SOUNDSDAT CONFIG_PATH "/" FILE_SOUNDS_DAT
	#endif //#ifndef PNAME_SOUNDSDAT

	#ifndef CLINICAL_BUILD
		#define CLINICAL_BUILD CONFIG_PATH "/clinical_build"
	#endif // #ifndef CLINICAL_BUILD

	#ifndef TEST_BUILD
		#define TEST_BUILD CONFIG_PATH "/test_build"
	#endif // #ifndef TEST_BUILD

	#ifndef SOFTCRC_PATH
		#define SOFTCRC_PATH TRIMA_PATH "/softcrc"
	#endif // #ifndef SOFTCRC_PATH

	#ifndef FILELISTS_PATH
		#define FILELISTS_PATH SOFTCRC_PATH "/filelists"
	#endif // #ifndef FILELISTS_PATH

	#ifndef STRING_DIRECTORY
		#define STRING_DIRECTORY CONFIG_PATH "/strings"
    #endif
#else // #ifdef __COMPILE_FOR_VX_54__

	// These are the real include files.
	#include "node_data.h"
	#include "trimaOs.h"

#endif // #ifdef __COMPILE_FOR_VX_54__

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
// 
// 
// 
// checkPasSettings
// 
// (See IT 8867) The idea here is that after an upgrade it's possible to end up
// with PLT config settings with a combined PAS + PLT volume greater than the
// total bag capacity of 1200.  That's not good.  This function mitigates that. 
// 
bool checkPasSettings(CDatFileReader& datfile)
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
	  if (!pasOnStr || !atoi(pasOnStr)) continue;

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
		 fprintf(stderr, "Hmm... for PLT%d, percent plasma is %f, and collect vol is %f. Moving on...\n", prodNum+1, percentPlasma, collectVol);
         continue;
	  }

	  // Figure out the max bag volume.
	  float collectVolMax = floor(1200.0f * (percentPlasma / 100.0f));

	  if (collectVol > collectVolMax)
	  {
		 fprintf(stderr, "PLT%d has collect vol %f greater than max %f. Adjusting accordingly.\n", prodNum+1, collectVol, collectVolMax);
		 datfile.SetFloat( "PRODUCT_TEMPLATES", pltVolVarNameStr, collectVolMax );
		 returnVal = true;
	  }
   }

   return returnVal;
}


bool update51to5r(CDatFileReader& datfile)
{
	// make this one of the newly added config values... or it wont update!
	if ( datfile.Find("PROCEDURE_CONFIG", "key_show_residual_loss") )
	   return false;

	cerr << "v5.1 config.dat file found.  Conversion needed" << endl;
	//////////////////////////////////////////////////////////////////////////////////
	//                 5.1-->5.P+R changes
	//////////////////////////////////////////////////////////////////////////////////
	{
	   // Plasma rinseback
	   /// (Note: in 5.1 and 5.2 there was an option called "key_rinseback_setting",
	   //  which could be set to 0 (normal) or 1 (plasma).  Now, we have two special
	   //  rinseback settings: one for plasma and one for saline.  They're not mutually
	   //  exclusive and each has its own button, so "key_rinseback_setting" became
	   //  "key_plasma_rinseback", and "key_saline_rinseback" was added.  Both are
	   //  0 for off and 1 for on.
	   const char* oldRinsebackSetting = datfile.Find("PROCEDURE_CONFIG", "key_rinseback_setting");
	   if (oldRinsebackSetting && (atoi(oldRinsebackSetting) == 1)) // If plasma rinseback existed and was turned on, turn it on.
		   datfile.AddLine( "PROCEDURE_CONFIG", "key_plasma_rinseback",  "1" );
	   else datfile.AddLine( "PROCEDURE_CONFIG", "key_plasma_rinseback",  "0" );

	   // Saline Rinseback
	  datfile.AddLine( "PROCEDURE_CONFIG", "key_saline_rinseback",     "0" );

	  // Show Residual Loss
	  datfile.AddLine( "PROCEDURE_CONFIG", "key_show_residual_loss",   "0" );

	  // Split PAS into separate bag form PLT product
	  datfile.AddLine( "PROCEDURE_CONFIG", "key_plt_mss_split_notif",  "0" );

	  // Show blood diversion graphics
      datfile.AddLine( "PROCEDURE_CONFIG", "key_blood_diversion",      "0" );

	  // Master RAS and PAS toggles
      datfile.AddLine( "PROCEDURE_CONFIG", "key_mss_plt_on",           "0" );
      datfile.AddLine( "PROCEDURE_CONFIG", "key_mss_rbc_on",           "0" );

	  // Operators potentially discard flagged products
	  datfile.AddLine( "PROCEDURE_CONFIG", "key_show_pre_aas_flags",   "1" );

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
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_volume_1", "25" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_ptf_1", "0" );
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_2", "0" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_volume_2", "25" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_ptf_2", "0" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_3", "0" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_mss_volume_3", "25" ); 
      datfile.AddLine( "PRODUCT_TEMPLATES", "key_rbc_ptf_3", "0" ); 

	  // TRALI exclusions
      datfile.AddLine( "PREDICTION_CONFIG", "key_male_only_plt",  "0" );
      datfile.AddLine( "PREDICTION_CONFIG", "key_male_only_plasma",  "0" );

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
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_volume_7", "500.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_7", "0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_7", "50" );
		
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_yield_8", "9.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_volume_8", "500.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_8", "0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_8", "50" );
		
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_yield_9", "10.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_volume_9", "500.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_9", "0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_9", "50" );
		
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_yield_10", "11.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_volume_10", "500.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_mss_10", "0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_pct_carryover_10", "50" );
   }
	return true;
}

bool updatePostCount(CDatFileReader& datfile)
{
	bool bUpdate = false;
	// IT 8741 - Force post count config setting to 100K
	float postCount = datfile.GetFloat("PROCEDURE_CONFIG", "key_post_plat");
	if (postCount < 100000) {
		datfile.RemoveLine("PROCEDURE_CONFIG", "key_post_plat");
		datfile.AddLine("PROCEDURE_CONFIG", "key_post_plat", "100000");
		cerr << "Changed donor post count from "<< postCount << " to 100000" << endl;
		bUpdate = true;
	}
	return bUpdate;
}
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

   bool updatePostCountReturn = updatePostCount(datfile);
   bool update51to5RReturn = update51to5r(datfile);
   bool checkPasSettingsReturn = checkPasSettings(datfile);

   // Note that function calls are done above to avoid the
   // short circuit OR.  We want all three to be evaluated.
   if(updatePostCountReturn	||
	  update51to5RReturn	||
	  checkPasSettingsReturn)
   {
	   datfile.WriteCfgFile(FILE_CONFIG_DAT);
	   cerr << "config.dat file converted." << endl;
   }
   else
	   cerr << "Up to date 5.R config file found. No conversion needed." << endl;
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

   // check if 5.R by looking for a new parameter.......
   if ( datfile.Find("TOUCHSCREEN", "a") )
   {
      cerr << "v5.R " << FILE_CAL_DAT << " file found.  No conversion needed" << endl;
      return;
   }

   cerr << "Pre-v5.R " << FILE_CAL_DAT << " file found.  Conversion needed" << endl;
   //////////////////////////////////////////////////////////////////////////////////
   //                 5.1/P-->5.R changes
   //////////////////////////////////////////////////////////////////////////////////
   {
      datfile.AddLine( "TOUCHSCREEN", "a", "0.842105263" );
      datfile.AddLine( "TOUCHSCREEN", "b", "0.0" );
      datfile.AddLine( "TOUCHSCREEN", "c", "-75.7894737" );
      datfile.AddLine( "TOUCHSCREEN", "d", "0.0" );
      datfile.AddLine( "TOUCHSCREEN", "e", "0.685714286" );
	  datfile.AddLine( "TOUCHSCREEN", "f", "-89.1428571" );
   }

   datfile.WriteCfgFile(FILE_CAL_DAT);

   cerr << FILE_CAL_DAT << " file converted." << endl;
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
///////////////////////////////////////////////////////////////////////////////////////
//              SOUNDS.dat
//////////////////////////////////////////////////////////////////////////////////////
void updateSounds()
{

   //
   // Replace terror_config.dat if the version number has changed
   currVersion = findSetting("file_version=", PNAME_SOUNDSDAT);
   newVersion = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SOUNDS_DAT);
    
   if ( (currVersion == NULL) || 
		(newVersion && ( !currVersion || strcmp(newVersion, currVersion) != 0 ))
	  )
   {
      fprintf(stdout, "Updating sounds.dat to new version %s from existing version %s...\n", newVersion, currVersion);
      attrib(PNAME_SOUNDSDAT, "-R");

      if ( cp( TEMPLATES_PATH "/" FILE_SOUNDS_DAT, PNAME_SOUNDSDAT ) == ERROR )
      {
         fprintf( stdout, "copy of sounds.dat failed\n" );
         return;
      }

      attrib(PNAME_SOUNDSDAT, "+R");
      fflush(stdout);
   } 
}
/////////////////////////////////////////////////////////////////////////////////////
      
   
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
   newVersion = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SETCONFIG_DAT);

   // if the file isnt there....
   if (currVersion == NULL)
   {
       fprintf(stdout, "Adding %s ...\n", FILE_SETCONFIG_DAT);
       attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "-R");

       if ( cp( TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT ) == ERROR )
       {
          fprintf( stdout, "copy of %s failed\n", FILE_SETCONFIG_DAT );
          return;
       }

       attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
       fflush(stdout);

   }   else {

       fprintf(stdout, "%s already exists ...\n", FILE_SETCONFIG_DAT);

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
    char* vxboot = VXBOOT_PATH;
    pVolDesc = dosFsVolDescGet((void*)vxboot, NULL);
    if ( !pVolDesc )
    {
        perror(VXBOOT_PATH);
        return;
    }

    cbioModeSet(pVolDesc->pCbio, O_RDWR);

    char* trima = TRIMA_PATH;  
    pVolDesc = dosFsVolDescGet((void*)trima, NULL);
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

   if ( attrib(VXBOOT_PATH "/bootrom.sys", "-R") == ERROR ) {
      perror(VXBOOT_PATH "/bootrom.sys");
   }

   if ( attrib(VXBOOT_PATH "/vxWorks", "-R") == ERROR ) {
      perror(VXBOOT_PATH "/vxWorks");
   }

   if ( IsVendor( "Ampro" ) ) {
      fprintf( stderr, "Copying Ampro bootrom.sys and vxworks to %s\n", VXBOOT_PATH );

      if ( copyFileContiguous( UPDATE_PATH "/bootrom_ampro.sys", VXBOOT_PATH "/bootrom.sys" ) == ERROR ||
           copyFileContiguous( UPDATE_PATH "/vxWorks_ampro"    , VXBOOT_PATH "/vxWorks"     ) == ERROR  )
      {
      fprintf( stderr, "Install of OS image failed\n" );
      return;
      }
   }
   else {
      fprintf( stderr, "Copying Versalogic bootrom.sys and vxworks to %s\n", VXBOOT_PATH );

      if ( copyFileContiguous( UPDATE_PATH "/bootrom_versalogic.sys", VXBOOT_PATH "/bootrom.sys" ) == ERROR ||
           copyFileContiguous( UPDATE_PATH "/vxWorks_versalogic"    , VXBOOT_PATH "/vxWorks"     ) == ERROR  ) 
      {
      fprintf( stderr, "Install of OS image failed\n" );
      return;
   }
   }

   if ( remove( UPDATE_PATH "/bootrom.sys" ) == ERROR ||
        remove( UPDATE_PATH "/vxWorks"     ) == ERROR ||
        remove( UPDATE_PATH "/bootrom_ampro.sys" ) == ERROR ||
        remove( UPDATE_PATH "/vxWorks_ampro"     ) == ERROR ||
        remove( UPDATE_PATH "/bootrom_versalogic.sys" ) == ERROR || 
        remove( UPDATE_PATH "/vxWorks_versalogic"     ) == ERROR ||
        remove( UPDATE_PATH "/vxboot.taz"  ) == ERROR )
   {
      fprintf( stderr, "Removal of temporary OS image failed\n" );
      return;
   }
    
   //
   // Remove existing Trima files
   fprintf( stdout, "Removing old Trima files...\n" );
   fileSort(TRIMA_PATH,    FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
   fileSort(SAVEDATA_PATH, FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
   fileSort(TOOLS_PATH,    FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
   fileSort(STRING_DIRECTORY,FILE_SORT_BY_DATE_ASCENDING, update_clean_file);

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

   //
   // Uncompress the update file
   fprintf( stdout, "Extracting the string.info files...\n" );
   if ( tarExtract( UPDATE_PATH "/strings.taz", STRING_DIRECTORY ) == ERROR )
   {
      fprintf( stdout, "Extraction of the string.info files failed.\n" );
      return;
   }
   if ( remove( UPDATE_PATH "/strings.taz" ) == ERROR )
   {
      fprintf( stdout, "Removal of string archive image failed\n" );
      return;
   }
   
   if ( IsVendor( "Ampro" ) ) {

      fprintf( stderr, "Copying Ampro bootrom.sys and vxworks to %s\n", SAFETY_BOOT_PATH );
      if ( cp( SAFETY_BOOT_PATH "/bootrom_ampro.sys", SAFETY_BOOT_PATH "/bootrom.sys" ) == ERROR ||
           cp( SAFETY_BOOT_PATH "/vxWorks_ampro"    , SAFETY_BOOT_PATH "/vxWorks"     ) == ERROR  )
      {
      fprintf( stderr, "Install of OS image failed\n" );
      return;
      }
   }
   else {

      fprintf( stderr, "Copying Versalogic bootrom.sys and vxworks to %s\n", SAFETY_BOOT_PATH );
      if ( cp( SAFETY_BOOT_PATH "/bootrom_versalogic.sys", SAFETY_BOOT_PATH "/bootrom.sys" ) == ERROR ||
           cp( SAFETY_BOOT_PATH "/vxWorks_versalogic"    , SAFETY_BOOT_PATH "/vxWorks"     ) == ERROR  ) 
      {
      fprintf( stderr, "Install of OS image failed\n" );
      return;
      }
   }

   if ( remove( SAFETY_BOOT_PATH "/bootrom_ampro.sys" ) == ERROR ||
        remove( SAFETY_BOOT_PATH "/vxWorks_ampro"     ) == ERROR ||
        remove( SAFETY_BOOT_PATH "/bootrom_versalogic.sys" ) == ERROR || 
        remove( SAFETY_BOOT_PATH "/vxWorks_versalogic"     ) == ERROR )
   {
      fprintf( stderr, "Removal of temporary OS image failed\n" );
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
   //              sounds.dat
   //////////////////////////////////////////////////////////////////////////////////////
   updateSounds();
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
   softcrc("-filelist " FILELISTS_PATH "/trima.files     -update "       TRIMA_PATH "/trima.crc");
   softcrc("-filelist " FILELISTS_PATH "/safety.files    -update "       TRIMA_PATH "/safety.crc");


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
                          