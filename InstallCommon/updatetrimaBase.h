/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */
#ifndef UPDATETRIMABASE_H
#define UPDATETRIMABASE_H

#include "zlib.h"
#include "filenames.h"
#include "filesort.h"

#include "datfile.h"
#include "configdef.h"

#include "os/discover_hw.h"

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
	#endif // #ifndef STRING_DIRECTORY

	#ifndef DROP_IN_FONTS_DIR
		#define DROP_IN_FONTS_DIR CONFIG_PATH "/fonts"
	#endif // #ifndef DROP_IN_FONTS_DIR

	#ifndef DATA_DIRECTORY
		#define DATA_DIRECTORY CONFIG_PATH "/data"
	#endif // #ifndef DATA_DIRECTORY

	#ifndef GRAPHICS_PATH
		#define GRAPHICS_PATH      CONFIG_PATH "/graphics"
	#endif // #ifndef GRAPHICS_PATH

	#ifndef FILE_GUI_GRAPHICS
		#define FILE_GUI_GRAPHICS     "graphics_package.out"
	#endif // #ifndef FILE_GUI_GRAPHICS

	#ifndef FILE_SERVICE_GRAPHICS
		#define FILE_SERVICE_GRAPHICS "service_" FILE_GUI_GRAPHICS
	#endif // #ifndef FILE_SERVICE_GRAPHICS

	#ifndef PNAME_GUI_GRAPHICS
		#define PNAME_GUI_GRAPHICS      GRAPHICS_PATH "/"  FILE_GUI_GRAPHICS
	#endif // #ifndef PNAME_GUI_GRAPHICS

	#ifndef PNAME_SERVICE_GRAPHICS
		#define PNAME_SERVICE_GRAPHICS  GRAPHICS_PATH "/"  FILE_SERVICE_GRAPHICS
	#endif // #ifndef PNAME_SERVICE_GRAPHICS

	#ifndef PNAME_GUI_GRAPHICS_CRC
		#define PNAME_GUI_GRAPHICS_CRC	CONFIG_CRC_PATH "/graphics.crc"
	#endif // #ifndef PNAME_GUI_GRAPHICS_CRC
	
	#ifndef PNAME_FONT_CRC_FILE
		#define PNAME_FONT_CRC_FILE		CONFIG_CRC_PATH "/fonts.crc"
	#endif // #ifndef PNAME_FONT_CRC_FILE

	#ifndef PNAME_DATA_CRC_FILE
		#define PNAME_DATA_CRC_FILE		CONFIG_CRC_PATH "/data.crc"
	#endif // #ifndef PNAME_DATA_CRC_FILE

	#ifndef PNAME_STRING_CRC_FILE
		#define PNAME_STRING_CRC_FILE	CONFIG_CRC_PATH "/strings.crc"
	#endif // #ifndef PNAME_STRING_CRC_FILE

   #ifdef SAFETY_VXWORKS_PXE_IMAGE
      #undef SAFETY_VXWORKS_PXE_IMAGE
   #endif
	#define SAFETY_VXWORKS_PXE_IMAGE    SAFETY_BOOT_PATH "/vxWorks_pxe"

	#ifdef SAFETY_BOOTROM_PXE_IMAGE
      #undef SAFETY_BOOTROM_PXE_IMAGE
   #endif
	#define SAFETY_BOOTROM_PXE_IMAGE	SAFETY_BOOT_PATH "/bootrom_pxe.sys"

	#ifdef SAFETY_BOOTROM_IMAGE
		#undef SAFETY_BOOTROM_IMAGE
	#endif
    #define SAFETY_BOOTROM_IMAGE        SAFETY_BOOT_PATH "/bootrom.sys"
		
	#ifdef SAFETY_VXWORKS_IMAGE
		#undef SAFETY_VXWORKS_IMAGE
	#endif
    #define SAFETY_VXWORKS_IMAGE		SAFETY_BOOT_PATH "/vxWorks"

    #ifdef TRIMA_PROJECT_REVISION_FILE
        #undef TRIMA_PROJECT_REVISION_FILE
    #endif
    #define TRIMA_PROJECT_REVISION_FILE TRIMA_PATH "/projectrevision"

   #ifndef FILE_TCHSCRN_DAT
      #define FILE_TCHSCRN_DAT	 "touch_screen.dat"
   #endif

   #ifndef PNAME_TCHSCRNDAT
      #define PNAME_TCHSCRNDAT	 CONFIG_PATH "/" FILE_TCHSCRN_DAT
   #endif

#else // #ifdef __COMPILE_FOR_VX_54__

	// These are the real include files.
	#include "node_data.h"
	#include "trimaOs.h"

#endif // #ifdef __COMPILE_FOR_VX_54__

// Make sure these are defined for everybody
#ifndef FILE_VISTIP_DAT
   #define FILE_VISTIP_DAT	 "vista_ip.dat"
#endif

#ifndef PNAME_VISTIPDAT
   #define PNAME_VISTIPDAT	 CONFIG_PATH "/" FILE_VISTIP_DAT
#endif

#ifndef FILE_FEATURES
   #define FILE_FEATURES "features.bin"
#endif

#ifndef PNAME_FEATURES
   #define PNAME_FEATURES CONFIG_PATH "/" FILE_FEATURES
#endif

// special files

#ifndef CLINICAL_BUILD
   #define CLINICAL_BUILD CONFIG_PATH "/clinical_build"
#endif

#ifndef TEST_BUILD
   #define TEST_BUILD CONFIG_PATH "/test_build"
#endif

#ifndef TELNET_ON
   #define TELNET_ON CONFIG_PATH "/telnet"
#endif

#ifndef PNAME_FTP_ALLOWED
   #define PNAME_FTP_ALLOWED CONFIG_PATH "/ftp_with_cassette_down"
#endif

#ifndef PUMP_STROKE_LOGGING_ON
   #define PUMP_STROKE_LOGGING_ON CONFIG_PATH "/pump_stroke_logging"
#endif

// To fix a change in 6.0 that renames these file defines
#ifndef PNAME_STRING_CRC
    #define PNAME_STRING_CRC PNAME_STRING_CRC_FILE
#endif

#ifndef PNAME_FONT_CRC
    #define PNAME_FONT_CRC PNAME_FONT_CRC_FILE
#endif

#ifndef PNAME_DATA_CRC
    #define PNAME_DATA_CRC PNAME_DATA_CRC_FILE
#endif

// Enum of versions
// I swear, when I started there were only 5!
enum TrimaVersion {V510, V512, V513, V514, V515, V516, V517, V518, V520, V521, V522, V600, V602, V603, V610, V611, V612, V620, V630, NUMBER_OF_VERSIONS};

/*
    FILE *updatelogfile;
    bool updatelogfileopen;

   void updateLogInit()
   {
      updatelogfile = fopen( "/machine/tmp/updatelog.txt", "w" );
      if (updatelogfile)
      {
         updatelogfileopen = true;
      }
      updatelogfileopen = false;
   }
*/


class updatetrimaBase
{
public:
   // Default constructor
   updatetrimaBase();

   // Default destructor
   virtual ~updatetrimaBase();

   // Upgrade from 510 to this version
   int upgrade(TrimaVersion fromVersion);

private:
   // Copy constructor
   updatetrimaBase( const updatetrimaBase &obj );

protected:

////////////////////////////////////////////////////////////////
// Common funtions that shouldn't be overridden
////////////////////////////////////////////////////////////////

   int copyFileContiguous(const char * from, const char * to);

   int unzipFile(const char * from, const char * to);

   int zipFile(const char * from, const char * to);

   bool findTazRevision(const char *searchFileName, char *tazRevString);

   bool readProjectRevisionFile(char *revString);

//   bool parseRevision(const char *revString, TrimaVersion &parsedVersion);

   static FileCallBackStatus update_clean_file(const char * fullPathName);

   static FileCallBackStatus update_file_set_rdwrite(const char * fullPathName);

   static FileCallBackStatus update_file_set_rdonly(const char * fullPathName);

   const char * findSetting(const char * setting, FILE * fp);

   const char * findSetting(const char * setting, const char * fileName);

   int verifyCrc(const char* commandLine);

   bool updatePostCount(CDatFileReader& datfile);

   bool replaceCassette(const char *refStr, unsigned int tubingSetCode, const char *barcodeStr);

   /////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////
// Virtual funtions that can be overridden for each version
////////////////////////////////////////////////////////////////

//
// rbc.dat update function
//
   virtual void updateRBC();

//
// hw.dat update function
//
   virtual void updateHW();

//
// sw.dat update function
//
   virtual void updateSW();

//
// terror_config.dat update function
//
   virtual void updateTerror();

//
// sounds.dat update function
//
   virtual void updateSounds();

//
// cassette.dat update function
//
   virtual void updateCassette();

//
// setconfig.dat update function
//
   virtual void updateSetConfig();

//
// vista_ip.dat update function
//
   virtual void updateVista();

//
// common location for code to copy trap files
//
   virtual void copyTrapFiles();

//
// check for inconsistent plasma rinseback settings
//
   virtual bool checkPlasmaRB();

//
// update the config.dat from 510 to 517/8
//
   virtual bool updateConfig510517(CDatFileReader& datfile);

//
// update the config.dat from 510 to 520
//
   virtual bool updateConfig510520(CDatFileReader& datfile);

//
// update the config.dat from 517/8 to 510
//
   virtual bool updateConfig517510(CDatFileReader& datfile);

//
// update the config.dat from 52X to 510
//
   virtual bool updateConfig52X510(CDatFileReader& datfile);

//
// update the config.dat from 5X to 600
//
   virtual bool updateConfig5X600(CDatFileReader& datfile);

//
// update the config.dat from 600 to 510
//
   virtual bool updateConfig600510(CDatFileReader& datfile);

//
// update the config.dat from 600 to 602
//
   virtual bool updateConfig600602(CDatFileReader& datfile);

//
// update the config.dat from 602 to 600
//
   virtual bool updateConfig602600(CDatFileReader& datfile);

//
// update the config.dat from 600 to 610
//
   virtual bool updateConfig600610(CDatFileReader& datfile);

//
// update the config.dat from 610 to 600
//
   virtual bool updateConfig610600(CDatFileReader& datfile);

//
// update the config.dat from 602 to 630
//
   virtual bool updateConfig602630(CDatFileReader& datfile);

//
// update the config.dat from 630 to 602
//
   virtual bool updateConfig630602(CDatFileReader& datfile);


////////////////////////////////////////////////////////////////
// Abstract funtions that need to be overridden for each version
////////////////////////////////////////////////////////////////
//
// Main config file update function, calls the others
//
   virtual void updateConfig(TrimaVersion fromVersion) = 0;

//
// does the file extraction
//
   virtual bool extractUpdateFiles() = 0;

//
// Update the config file for the version being installed
//
   virtual bool updateConfigVersion(CDatFileReader& datfile, TrimaVersion fromVersion) = 0;

//
// cal.dat update function
//
   virtual void updateCal() = 0;

//
// trap_default.dat and trap_override.dat update function
//
   virtual void updateTrap(TrimaVersion fromVersion) = 0;

//
// globvars update function
//
   virtual void updateGlobVars() = 0;

//
// CRC update & checking
//
   virtual bool checkCRC() = 0;

public:
   enum { MaxIDStringLength = 256 };

protected:

   const char *currVersion;
   const char *newVersion;

};
#endif // UPDATETRIMABASE_H

