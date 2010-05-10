/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 *
 * $Header: //bctquad3/home/BCT_Development/Install/Downgrade/v6.0_to_V5.1/rcs/updatetrima.cpp 1.3 2009/03/16 15:23:12Z jsylusb Exp $
 * $Log: updatetrima.cpp $
 * Revision 1.3  2009/03/16 15:23:12Z  jsylusb
 * The configurable list (setconfig.dat) is replaced with the template on a downgrade from 6.0 to 5.2, and is deleted on downgrade from 6.0 to 5.1.
 * Revision 1.2  2009/01/07 19:19:45Z  jsylusb
 * Updated the script in order to update the trap files. 
 * Revision 1.1  2008/10/23 20:50:10Z  jsylusb
 * Initial revision
 * Revision 1.1  2008/03/12 16:06:17  jheiusb
 * Initial revision
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

#include "zlib.h"
#include "filenames.h"
#include "filesort.h"

#include "targzextract.c"
#include "crc.c"

#include "datfile.h"
#undef __COMPILE_FOR_VX_54__
#ifdef __COMPILE_FOR_VX_54__
	#include "trima_opsys.h"
#endif

#ifndef TEMPLATES_PATH
	#define TEMPLATES_PATH "/trima/config/templates"
#endif
#ifndef FILE_CASSETTE_DAT
	#define FILE_CASSETTE_DAT "cassette.dat"
#endif // #ifndef FILE_CASSETTE_DAT

#ifndef FILE_SETCONFIG_DAT
	#define FILE_SETCONFIG_DAT "setconfig.dat"
#endif // #ifndef FILE_SETCONFIG_DAT

#ifndef CLINICAL_BUILD
	#define CLINICAL_BUILD CONFIG_PATH "/clinical_build"
#endif // #ifndef CLINICAL_BUILD

#ifndef TEST_BUILD
	#define TEST_BUILD CONFIG_PATH "/test_build"
#endif // #ifndef TEST_BUILD

#define SOFTCRC_PATH       TRIMA_PATH   "/softcrc"
#define FILELISTS_PATH     SOFTCRC_PATH "/filelists"


#ifdef __cplusplus
extern "C" { 
#endif

int cp(const char * from, const char * to);
int xcopy (const char * src, const char *dest);
int copyFileContiguous(const char * from, const char * to);
int unzipFile(const char * from, const char * to);
void updateTrima();

int softcrc(const char * options);

void trimaSysStartDiskSync(void);

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
      fprintf( stderr, "copying %s to %s: file length = %ld bytes\n", from, to, fileStat.st_size);

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
         fprintf( stderr, "Copy failed (%ld bytes written)\n", bytesCopied);
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

      fprintf( stderr, "\tUncompression complete\n" );
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
//
//  Update the dat files.....
//
///////////////////////////////////////////////////////////////////////////////////
//              Config.dat
///////////////////////////////////////////////////////////////////////////////////
//IT 8737 (5.14) - Force post count config setting to 100K 
bool updatePostCount(CDatFileReader& datfile)
{
   float postCount = datfile.GetFloat("PROCEDURE_CONFIG", "key_post_plat");
   if (postCount < 100000) {
	   datfile.RemoveLine("PROCEDURE_CONFIG", "key_post_plat");
	   datfile.AddLine("PROCEDURE_CONFIG", "key_post_plat", "100000");
	   cerr << "Changed donor post count from "<< postCount << " to 100000" << endl;
	   return true;
   }
   return false;
}

// Returns true if updates were made and false if not.
bool updateConfig51to517(CDatFileReader& datfile)
{
	if ( !datfile.Find("PRODUCT_TEMPLATES","key_plt_amap_single_yield_min") )
	{
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_single_yield_min", "1.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_single_yield_max", "4.5" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_double_yield_min", "5.5" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_double_yield_max", "8.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_triple_yield_min", "9.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_triple_yield_max", "12.0" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_single_conc", "1400" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_double_conc", "1400" );
		datfile.AddLine( "PRODUCT_TEMPLATES", "key_plt_amap_triple_conc", "1400" );

		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_a", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_b", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_c", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_d", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_e", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_f", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_g", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_h", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_i", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_j", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_k", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_l", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_m", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_n", "0" );
		datfile.AddLine( "PRODUCT_DEFINITIONS", "key_platelet_amap_o", "0" );

		return true;
	}

	return false;
}

// Returns true if updates were made and false if not.
bool updateConfig50to51(CDatFileReader& datfile)
{
	if ( datfile.Find("PROCEDURE_CONFIG","key_rinseback_protocol") )
	{
	   return false;
	}
	cerr << "pre-v5.1 config.dat file found.  Conversion needed" << endl;

	/* IT 6050 ------------------------------------------------------------------*/
	/*                                                                           */
	/*   ...                                                                     */
	/* Mon May 5 14:30:13 MDT 2003 -Scott Butzke- My suggestions for             */
	/* manufacturing implementation are:                                         */ 
	/*   ...                                                                     */
	/*   - key_weight_setting: ... For upgrades, copy V5"less than" to all three */
	/*     "less than" fields, weight setting to all three weight cutoffs,       */
	/*     and "greater than" to the "key_weight_3_greater_than_vol" field.      */                                 
	/*   ...                                                                     */
	/*   - key_plasma_1 to 6: ... for upgrades set plasma1 to V5 small, plasma2  */ 
	/*     to V5 medium, and plasma3-6 to V5 large.                              */
	/*   ...                                                                     */
	/*                                                                           */
	/* IT 6050 ------------------------------------------------------------------*/

	/*
		 Changes for v5.1 ...

		   key_weight_1_setting=<key_weight_setting>
		   key_weight_2_setting=<key_weight_setting>
		   key_weight_3_setting=<key_weight_setting>
		   key_weight_1_less_than_vol=<key_less_than_vol>
		   key_weight_2_less_than_vol=<key_less_than_vol>
		   key_weight_3_less_than_vol=<key_less_than_vol>
		   key_weight_3_greater_than_vol=<key_greater_than_vol>
		   ...
		   key_pls_volume_1=<key_sm_vol>
		   key_pls_volume_2=<key_med_vol>
		   key_pls_volume_3=<key_lg_vol>
		   key_pls_volume_4=<key_lg_vol>
		   key_pls_volume_5=<key_lg_vol>
		   key_pls_volume_6=<key_lg_vol>
		   ...
		   key_pls_amap_minimum=<key_pls_amap_min>
	*/

	{
	   float weightSetting = datfile.GetFloat( "PROCEDURE_CONFIG", "key_weight_setting" );
	   char weightSettingStr[16];
	   sprintf( weightSettingStr, "%0.1f", weightSetting );
	   datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_1_setting", weightSettingStr );
	   datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_2_setting", weightSettingStr );
	   datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_3_setting", weightSettingStr );
	   datfile.RemoveLine( "PROCEDURE_CONFIG", "key_weight_setting" );
	}
	{
	   float lessThanVol = datfile.GetFloat( "PROCEDURE_CONFIG", "key_less_than_vol" );
	   char lessThanVolStr[16];
	   sprintf( lessThanVolStr, "%0.1f", lessThanVol );
	   datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_1_less_than_vol", lessThanVolStr );
	   datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_2_less_than_vol", lessThanVolStr );
	   datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_3_less_than_vol", lessThanVolStr );
	   datfile.RemoveLine( "PROCEDURE_CONFIG", "key_less_than_vol" );
	}
	{
	   float greaterThanVol = datfile.GetFloat( "PROCEDURE_CONFIG", "key_greater_than_vol" );
	   char greaterThanVolStr[16];
	   sprintf( greaterThanVolStr, "%0.1f", greaterThanVol );
	   datfile.AddLine( "PROCEDURE_CONFIG", "key_weight_3_greater_than_vol", greaterThanVolStr );
	   datfile.RemoveLine( "PROCEDURE_CONFIG", "key_greater_than_vol" );
	}
	{
	   float smallVol = datfile.GetFloat( "PRODUCT_TEMPLATES", "key_sm_vol" );
	   float mediumVol = datfile.GetFloat( "PRODUCT_TEMPLATES", "key_med_vol" );
	   float largeVol = datfile.GetFloat( "PRODUCT_TEMPLATES", "key_lg_vol" );
	   char smallVolStr[16];
	   char mediumVolStr[16];
	   char largeVolStr[16];
	   sprintf( smallVolStr, "%0.1f", smallVol );
	   sprintf( mediumVolStr, "%0.1f", mediumVol );
	   sprintf( largeVolStr, "%0.1f", largeVol );
	   datfile.AddLine( "PRODUCT_TEMPLATES", "key_pls_volume_1", smallVolStr );
	   datfile.AddLine( "PRODUCT_TEMPLATES", "key_pls_volume_2", mediumVolStr );
	   datfile.AddLine( "PRODUCT_TEMPLATES", "key_pls_volume_3", largeVolStr );
	   datfile.AddLine( "PRODUCT_TEMPLATES", "key_pls_volume_4", largeVolStr );
	   datfile.AddLine( "PRODUCT_TEMPLATES", "key_pls_volume_5", largeVolStr );
	   datfile.AddLine( "PRODUCT_TEMPLATES", "key_pls_volume_6", largeVolStr );
	   datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_sm_vol" );
	   datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_med_vol" );
	   datfile.RemoveLine( "PRODUCT_TEMPLATES", "key_lg_vol" );
	}
	{
	   float amapMin = datfile.GetFloat( "PREDICTION_CONFIG", "key_amap_min" );
	   char amapMinStr[16];
	   sprintf( amapMinStr, "%0.1f", amapMin );
	   datfile.AddLine( "PREDICTION_CONFIG", "key_pls_amap_minimum", amapMinStr );
	   datfile.RemoveLine( "PREDICTION_CONFIG", "key_pls_amap_min" );
	}

	/*
		 New values for v5.1 ...

		   - key_rinseback_protocol=0
		   - key_audit_tracking=0
		   - key_pls_amap_maximum=1000
	*/
	{
	   datfile.AddLine( "PROCEDURE_CONFIG", "key_rinseback_protocol", "0" );
	   datfile.AddLine( "PROCEDURE_CONFIG", "key_audit_tracking", "0" );
	   datfile.AddLine( "PREDICTION_CONFIG", "key_pls_amap_maximum", "1000" );
	}
	return true;
}

bool updateConfig60to51(CDatFileReader& datfile)
{
	bool update = false;
   if(datfile.Find("LANGUAGE_UNIT_CONFIG","key_crit_or_glob") != NULL)
	{
		update = true;
		cerr << "v6.0 config.dat file found.  Conversion needed" << endl;
	}

   //////////////////////////////////////////////////////////////////////////////////
   //                 6.0-->5.1 changes
   //////////////////////////////////////////////////////////////////////////////////
	if(update)
   {
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
	  datfile.RemoveLine( "PROCEDURE_CONFIG", "key_show_pre_aas_flags" );
      datfile.RemoveLine( "PROCEDURE_CONFIG", "key_rbc_def_bag_vol" );
      datfile.RemoveLine( "PROCEDURE_CONFIG", "key_plt_def_bag_vol" );

	  char key[15] = "key_platelet_"; key[14] = 0;
	  int value = 0;

	  value = datfile.GetInt( "LANGUAGE_UNIT_CONFIG", "key_lang" );
	  if (value < 0 || value > 12) {
		  datfile.SetValue( "LANGUAGE_UNIT_CONFIG", "key_lang", "0" );
		  fprintf( stderr, "Setting language to english.\n" );
	  }

	  value = datfile.GetInt( "PROCEDURE_CONFIG", "key_tbv_vol_setting" );
	  if (value < 1 || value > 7) {
		  datfile.SetValue( "PROCEDURE_CONFIG", "key_tbv_vol_setting", "1" );
		  fprintf( stderr, "Setting key_tbv_vol_setting to '1'.\n" );
	  }

	  for (key[13] = 'a'; key[13] <= 'o'; ++key[13]) {
		  value = datfile.GetInt( "PRODUCT_DEFINITIONS", key );
		  if (value < 0 || value > 6) {
			  datfile.SetValue( "PRODUCT_DEFINITIONS", key, "0" );
			  fprintf( stderr, key );
			  fprintf( stderr, " set to product '0'.\n" );
		  }
	  }

	  if (datfile.Find("PROCEDURE_CONFIG", "key_rinseback_protocol") == NULL) {
		  datfile.AddLine("PROCEDURE_CONFIG", "key_rinseback_protocol", "0");
	  }
   }
	return update;
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
      
   bool update = 0;

   update |= updatePostCount(datfile);
   update |= updateConfig50to51(datfile);
   update |= updateConfig51to517(datfile);
	update |= updateConfig60to51(datfile);
   if (update)
   {
	   datfile.WriteCfgFile(FILE_CONFIG_DAT);
	   cerr << "config.dat file converted." << endl;
   }
	else
		cerr << "Up to date v5.1 config.dat file found.  No conversion needed" << endl;
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
	const std::string tsOriginal [] = {"screen_horizontal_size", "screen_vertical_size", "tsraw_left_edge", "tsraw_right_edge", 
		"tsraw_top_edge", "tsraw_bottom_edge" };

	const char* tsHeader = "TOUCHSCREEN";
	struct stat fileStat;
	if ( stat((char *)PNAME_TCHSCRNDAT, &fileStat) == OK )
	{ 	// 6.0 spiral 4 file 
		CDatFileReader tscrnFile(PNAME_TCHSCRNDAT);
		datfile.AddSection("[TOUCHSCREEN]");
		for (int i=0; i<=5; i++) //Keep both loops separate. 
		{
			datfile.AddLine(tsHeader, tsOriginal[i].c_str(), tscrnFile.Find(tsHeader, tsOriginal[i].c_str()));
		}
		datfile.Write(PNAME_CALDAT);
		tscrnFile.Write(PNAME_TCHSCRNDAT);

		if ( remove( PNAME_TCHSCRNDAT ) == ERROR )
      {
         fprintf( stderr, "Removal of %s file failed\n", PNAME_TCHSCRNDAT );
      }
      return;
   }

   else if ( !datfile.Find("TOUCHSCREEN","screen_horizontal_size") )
   {
      cerr << "pre-v5.1 cal.dat file found.  Conversion needed" << endl;

      FILE * fp;
		fp = fopen("/config/absf.2", "r");

      char x1Str[8];
      char x2Str[8];
      char y1Str[8];
      char y2Str[8];

		if ( fp )
		{
         char buffer[256];
         char *savePtr=0;
         char *p=0;
         int temp;
         fgets( buffer, 256, fp );
         fclose(fp);

         p = strtok_r(buffer," :x",&savePtr); // 0 x position
         p = strtok_r( NULL ," :x",&savePtr); // 0 y position
         p = strtok_r( NULL ," :x",&savePtr); // max x position
         p = strtok_r( NULL ," :x",&savePtr); // max y position
         p = strtok_r( NULL ," :x",&savePtr); // x1 position
         strcpy(x1Str,p);
         p = strtok_r( NULL ," :x",&savePtr); // x2 position
         strcpy(x2Str,p);
         p = strtok_r( NULL ," :x",&savePtr); // y1 position
         strcpy(y1Str,p);
         p = strtok_r( NULL ," :x",&savePtr); // y2 position
         strcpy(y2Str,p);

         attrib("/config/absf.2","-R");
         if ( remove( "/config/absf.2" ) == ERROR )
         {
            fprintf( stderr, "Removal of absf.2 file failed\n" );
         }

      }
      else
      {
         //
         // Default values if no absf.2 file ...
         //
         sprintf( x1Str,"%d",90 );
         sprintf( x2Str,"%d",850 );
         sprintf( y1Str,"%d",130 );
         sprintf( y2Str,"%d",830 );
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
///////////////////////////////////////////////////////////////////////////////////


const char *currVersion = "";
const char *newVersion  = "";

///////////////////////////////////////////////////////////////////////////////////
//              RBC.dat
///////////////////////////////////////////////////////////////////////////////////
void updateRBC()
{
   // Put the rbc.dat file in the correct location.
   attrib(CONFIG_PATH "/" FILE_RBC_DAT, "-R");
   if ( cp( TEMPLATES_PATH "/" FILE_RBC_DAT, CONFIG_PATH "/" FILE_RBC_DAT ) == ERROR )
   {
      fprintf( stderr, "copy of rbc.dat failed\n" );
      return;
   }
   attrib(CONFIG_PATH "/" FILE_RBC_DAT, "+R");

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
      fprintf(stderr, "Updating hw.dat to different version %s from existing version %s...\n", newVersion, currVersion);

      attrib(CONFIG_PATH "/" FILE_HW_DAT, "-R");
      if ( IsVendor( "Ampro" ) )
      {
         if ( cp( TEMPLATES_PATH "/hw_ampro.dat", CONFIG_PATH "/" FILE_HW_DAT ) == ERROR )
         {
               fprintf( stderr, "copy of hw_ampro.dat failed\n" );
               return;
         }
      }
      else
      {
         if ( cp( TEMPLATES_PATH "/hw_versalogic.dat", CONFIG_PATH "/" FILE_HW_DAT) == ERROR )
         {
            fprintf( stderr, "copy of hw_versalogic.dat failed\n" );
            return;
         }
      }
      attrib(CONFIG_PATH "/" FILE_HW_DAT, "+R");
      fflush(stderr);
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
      fprintf(stderr, "Updating sw.dat to different version %s from existing version %s...\n", newVersion, currVersion);
      attrib(CONFIG_PATH "/" FILE_SW_DAT, "-R");

      if ( cp( TEMPLATES_PATH "/" FILE_SW_DAT, CONFIG_PATH "/" FILE_SW_DAT ) == ERROR )
      {
         fprintf( stderr, "copy of %s failed\n", FILE_SW_DAT );
         return;
      }

      attrib(CONFIG_PATH "/" FILE_SW_DAT, "+R");
      fflush(stderr);
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
      fprintf(stderr, "Updating terror_config.dat to different version %s from existing version %s...\n", newVersion, currVersion);
      attrib(TERROR_CONFIG_FILE, "-R");

      if ( cp( TEMPLATES_PATH "/terror_config.dat", CONFIG_PATH "/terror_config.dat" ) == ERROR )
      {
         fprintf( stderr, "copy of terror_config.dat failed\n" );
         return;
      }

      attrib(CONFIG_PATH "/terror_config.dat", "+R");
      fflush(stderr);
   }

}

///////////////////////////////////////////////////////////////////////////////////////
//              SOUNDS.dat
//////////////////////////////////////////////////////////////////////////////////////
void updateSounds()
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
      fprintf(stderr, "Updating %s to different version %s from existing version %s...\n", FILE_CASSETTE_DAT, newVersion, currVersion);
      attrib(CONFIG_PATH "/" FILE_CASSETTE_DAT, "-R");

      if ( cp( TEMPLATES_PATH "/" FILE_CASSETTE_DAT, CONFIG_PATH "/" FILE_CASSETTE_DAT ) == ERROR )
      {
         fprintf( stderr, "copy of %s failed\n", FILE_CASSETTE_DAT );
         return;
      }

      attrib(CONFIG_PATH "/" FILE_CASSETTE_DAT, "+R");
      fflush(stderr);
   }
}
//////////////////////////////////////////////////////////////////////////////////////
      

//////////////////////////////////////////////////////////////////////////////////////
//              SETCONFIG.dat
//////////////////////////////////////////////////////////////////////////////////////
void updateSetConfig()
{
   // these are the customer selected sets.... dont overwrite if it exists! 
   currVersion = findSetting("file_version=", CONFIG_PATH "/" FILE_SETCONFIG_DAT);
   newVersion = findSetting("file_version=", TEMPLATES_PATH "/" FILE_SETCONFIG_DAT);

   if (currVersion == NULL) {
	   // if the file isnt there....
       fprintf(stderr, "Adding %s ...\n", FILE_SETCONFIG_DAT);
       // attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "-R");

       if ( cp( TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT ) == ERROR )
       {
          fprintf( stderr, "copy of %s failed\n", FILE_SETCONFIG_DAT );
          return;
       }

       attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
       fflush(stderr);
   }   
   else if (currVersion != NULL && newVersion != NULL && strcmp(newVersion, currVersion) < 0 ) {
	   // Override the file
       fprintf(stderr, "Overriding %s ...\n", FILE_SETCONFIG_DAT);
	   attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "-R");

       if ( cp( TEMPLATES_PATH "/" FILE_SETCONFIG_DAT, CONFIG_PATH "/" FILE_SETCONFIG_DAT ) == ERROR )
       {
          fprintf( stderr, "copy of %s failed\n", FILE_SETCONFIG_DAT );
          return;
       }

       attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "+R");
       fflush(stderr);
   }
   else if (currVersion != NULL && newVersion == NULL) {
	   // Remove the file
       fprintf(stderr, "Removing %s ...\n", FILE_SETCONFIG_DAT);
	   attrib(CONFIG_PATH "/" FILE_SETCONFIG_DAT, "-R");
	   remove(CONFIG_PATH "/" FILE_SETCONFIG_DAT);
   }
   else {
       fprintf(stderr, "%s already exists ...\n", FILE_SETCONFIG_DAT);
   }
}
///////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
//              Global  vars
//////////////////////////////////////////////////////////////////////////////////////
bool updateGlobVars50to51(CDatFileReader& datfile)
{
	
   if ( !datfile.Find( "EXTERNALIP" ) )
   {
      cerr << "pre-v5.1 globvars file found.  Conversion needed" << endl;

      // v5.1--------------
      //  MACHINE=1T000XX
      //  SENPID=000000000000
      //  EXTERNALIP=172.21.127.255
      //  EXTERNALBIP=172.21.255.255

      // v5.0--------------
      //  MACHINE=XX
      //  CENPID=000000000000
      //  SENPID=000000000000
      //  ENPID486I=000000000000
      //  AN2IP=172.21.127.255
      //  AN2BIP=172.21.255.255

      string an2ip   = datfile.GetString("AN2IP");
      string an2bip  = datfile.GetString("AN2BIP");
      datfile.RemoveLine("CENPID");
      datfile.RemoveLine("ENPID486I");
      datfile.RemoveLine("AN2IP");
      datfile.RemoveLine("AN2BIP");
      datfile.AddLine("EXTERNALIP",an2ip.c_str());
      datfile.AddLine("EXTERNALBIP",an2bip.c_str());

      datfile.Write(GLOBVARS_FILE);

      cerr << "globvars file converted." << endl;

      return true;
   }

   cerr << "v5.1 globvars file found.  No conversion needed" << endl;
	return false;
}

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

	bool update = updateGlobVars50to51(datfile);
	if(!update)
		cerr << "v5.2 globvars file found.  No conversion needed" << endl;
}


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
	pVolDesc = dosFsVolDescGet(const_cast<char*>("/vxboot"), NULL);
    if ( !pVolDesc )
    {
        perror(VXBOOT_PATH);
        return;
    }

    cbioModeSet(pVolDesc->pCbio, O_RDWR);

    pVolDesc = dosFsVolDescGet((void *)TRIMA_PATH, NULL);
    if ( !pVolDesc )
    {
        perror(TRIMA_PATH);
        return;
    }

    cbioModeSet(pVolDesc->pCbio, O_RDWR);   
   
   //
   // Extract v5 files, if this is an upgrade from v5 ...
   //
   struct stat archiveDir;
   struct stat archive;
   stat( "/machine/v5_archive", &archiveDir );
   stat( "/machine/v5_archive/config/machine.tar", &archive );

   if ( S_ISDIR( archiveDir.st_mode ) &&
        S_ISREG( archive.st_mode ) )
   {
      fprintf( stderr, "Found /machine/v5_archive directory\n" );
      fprintf( stderr, " ... Extracting v5.0 machine files\n" );
       
   //
   // Extract the update files
      fprintf( stderr, "Extracting machine.tar ...\n" );
      if ( tarExtract( "/machine/v5_archive/config/machine.tar", "/machine/tmp" ) == ERROR )
      {
         fprintf( stderr, "Extraction of v5.0 machine files failed\n" );
         return;
      }

      if ( cp( "/machine/v5_archive/config/machine.tar", "/machine/v5_archive/config/old_machine.tar" ) == ERROR ||
           remove( "/machine/v5_archive/config/machine.tar" ) == ERROR )
           
      {
         fprintf( stderr, "Move of machine.tar to old_machine.tar failed\n" );
         return;
      }

      //
      // Move the machine files to the /config partition ....
      fprintf( stderr, "Moving machine files ...\n" );
      if ( xcopy( "/machine/tmp/d/machine","/config" ) == ERROR )
      {
         fprintf( stderr, "Moving the v5.0 machine files failed\n" );
         return;
      }
   }

	//
   // Extract the update files
   fprintf( stderr, "Extracting updateTrima ...\n" );
   if ( tarExtract( "/machine/update/updateTrima.taz", "/machine/update" ) == ERROR )
   {
      fprintf( stderr, "Extraction of update files failed\n" );
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
      fprintf( stderr, "Saving the old OS image..." );
      attrib(VXBOOT_PATH "/vxWorks.old", "-R");
      if ( cp( VXBOOT_PATH "/vxWorks", VXBOOT_PATH "/vxWorks.old" ) == ERROR )
      {
         fprintf( stderr, "Archive of old OS image failed\n" );
      }

   }
    
   //
   // Store the new files in the proper position
   fprintf( stderr, "Extracting the OS image...\n" );
   if ( tarExtract( UPDATE_PATH "/vxboot.taz", UPDATE_PATH ) == ERROR )
   {
      fprintf( stderr, "Extraction of OS image failed\n" );
      return;
   }

   attrib(VXBOOT_PATH "/bootrom.sys", "-R");
   attrib(VXBOOT_PATH "/vxWorks", "-R");
   if ( copyFileContiguous( UPDATE_PATH "/bootrom.sys", VXBOOT_PATH "/bootrom.sys" ) == ERROR ||
        copyFileContiguous( UPDATE_PATH "/vxWorks"    , VXBOOT_PATH "/vxWorks"     ) == ERROR )
   {
      fprintf( stderr, "Install of OS image failed\n" );
      return;
   }
   if ( remove( UPDATE_PATH "/bootrom.sys" ) == ERROR ||
        remove( UPDATE_PATH "/vxWorks"     ) == ERROR ||
        remove( UPDATE_PATH "/vxboot.taz"  ) == ERROR )
   {
      fprintf( stderr, "Removal of temporary OS image failed\n" );
      return;
   }
    
   //
   // Remove existing Trima files
   fprintf( stderr, "Removing old Trima files...\n" );
   fileSort(TRIMA_PATH,    FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
   fileSort(SAVEDATA_PATH, FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
   fileSort(TOOLS_PATH,    FILE_SORT_BY_DATE_ASCENDING, update_clean_file);

   // IT 7051.  Remove clinical file if exists.
   remove(CLINICAL_BUILD);
   remove(TEST_BUILD);
   
   //
   // Uncompress the update file
   fprintf( stderr, "Extracting the Trima software files...\n" );
   if ( tarExtract( "/machine/update/trima.taz", "/trima" ) == ERROR )
   {
      fprintf( stderr, "Extraction of the Trima software failed.\n" );
      return;
   }
   if ( remove( "/machine/update/trima.taz" ) == ERROR )
   {
      fprintf( stderr, "Removal of Trima archive image failed\n" );
      return;
   }

   //
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
   
   // Verify the installation CRC values
   if ( softcrc("-filelist " FILELISTS_PATH "/trima.files -verify  "    TRIMA_PATH      "/trima.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/safety.files -verify "    TRIMA_PATH      "/safety.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/caldat.files -verify "    CONFIG_CRC_PATH "/caldat.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/config.files -verify "    CONFIG_CRC_PATH "/config.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/hwdat.files -verify "     CONFIG_CRC_PATH "/hwdat.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/machine.files -verify "   CONFIG_CRC_PATH "/machine.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/rbcdat.files -verify "    CONFIG_CRC_PATH "/rbcdat.crc") != 0 ||
        softcrc("-filelist " FILELISTS_PATH "/terrordat.files -verify " CONFIG_CRC_PATH "/terrordat.crc") != 0)
   {
      fprintf(stderr, "CRC check of installed software failed\n");
      return;
   }

   fprintf( stderr, "Trima software update complete.\n" );

   // Delete the update script so that it doesn't run again on the subsequent boot if the GTS guy
   // is still holding down the buttons.
   fprintf(stderr, "removing update script.\n");
   remove( UPDATE_PATH "/updatetrima" );
   remove( UPDATE_PATH "/updatetrima.taz" );

   fflush( stderr );

   trimaSysStartDiskSync();
}
                          
