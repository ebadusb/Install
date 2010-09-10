/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

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

	#ifndef SAFETY_VXWORKS_PXE_IMAGE
		#define SAFETY_VXWORKS_PXE_IMAGE    SAFETY_BOOT_PATH "/vxWorks_pxe"
	#endif // #ifndef SAFETY_VXWORKS_PXE_IMAGE

	#ifndef SAFETY_BOOTROM_PXE_IMAGE
		#define SAFETY_BOOTROM_PXE_IMAGE	SAFETY_BOOT_PATH "/bootrom_pxe.sys"
	#endif // #ifndef SAFETY_BOOTROM_PXE_IMAGE

	#ifdef SAFETY_BOOTROM_IMAGE
		#undef SAFETY_BOOTROM_IMAGE
	#endif
    #define SAFETY_BOOTROM_IMAGE        SAFETY_BOOT_PATH "/bootrom.sys"
		
	#ifdef SAFETY_VXWORKS_IMAGE
		#undef SAFETY_VXWORKS_IMAGE
	#endif
    #define SAFETY_VXWORKS_IMAGE		SAFETY_BOOT_PATH "/vxWorks"

    #ifndef TRIMA_PROJECT_REVISION_FILE
        #undef TRIMA_PROJECT_REVISION_FILE
    #endif
    #define TRIMA_PROJECT_REVISION_FILE TRIMA_PATH "/projectrevision"

#else // #ifdef __COMPILE_FOR_VX_54__

	// These are the real include files.
	#include "node_data.h"
	#include "trimaOs.h"

#endif // #ifdef __COMPILE_FOR_VX_54__


class updatetrimaBase
{
public:
   // Default constructor
   updatetrimaBase();

   // Default destructor
   virtual ~updatetrimaBase();

   // Run function - does everything
   int run();

private:
   // Copy constructor
   updatetrimaBase( const updatetrimaBase &obj );

protected:

////////////////////////////////////////////////////////////////
// Common funtions that shouldn't be overridden
////////////////////////////////////////////////////////////////

   int copyFileContiguous(const char * from, const char * to);

   int unzipFile(const char * from, const char * to);

   static FileCallBackStatus update_clean_file(const char * fullPathName);

   static FileCallBackStatus update_file_set_rdwrite(const char * fullPathName);

   static FileCallBackStatus update_file_set_rdonly(const char * fullPathName);

   const char * findSetting(const char * setting, FILE * fp);

   const char * findSetting(const char * setting, const char * fileName);

   int verifyCrc(const char* commandLine);

   bool updatePostCount(CDatFileReader& datfile);

// Read the Projectrevision file and parse the info
   bool readProjectrevision();

////////////////////////////////////////////////////////////////
// Virtual funtions that can be overridden for each version
////////////////////////////////////////////////////////////////

// Check if this is an allowed upgrade path
// This is virtual so it can be overridden in engr_tools
// to allow updates that aren't allowed in production
   virtual bool allowedUpgradePath();

// This is in the base class because it is needed to go 
// from 5.2 to 5.1 and to go from 5.2 to 6.0 
// because you don't need to downgrade from 5.2 to 5.1.0
// before going to 6.0 - so we have do it internally
//
   virtual bool updateConfig52to51(CDatFileReader& datfile);

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


////////////////////////////////////////////////////////////////
// Abstract funtions that need to be overridden for each version
////////////////////////////////////////////////////////////////
//
// Main config file update function, calls the others
//
   virtual void updateConfig() = 0;

//
// does the file extraction
//
   virtual bool extractUpdateFiles() = 0;

//
// Update the config file for the version being installed
//
   virtual bool updateConfigVersion(CDatFileReader& datfile) = 0;

//
// cal.dat update function
//
   virtual void updateCal() = 0;

//
// trap_default.dat and trap_override.dat update function
//
   virtual void updateTrap() = 0;

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

   // Enum of versions - MUST ADD NEW VERSIONS TO THE END
   enum TrimaVersion {V51, V517, V52, V60, V61, V518};

protected:

    const char *currVersion;
    const char *newVersion;

    TrimaVersion curTrimaVersion;
    TrimaVersion fromTrimaVersion;
    TrimaVersion toTrimaVersion;

};

                          
