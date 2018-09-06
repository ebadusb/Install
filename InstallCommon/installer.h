/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */
#ifndef INSTALLER_H
#define INSTALLER_H

#include "updateTrimaDefines.h"
#include "updatetrimaUtils.h"

#include "filenames.h"
#include "datfile.h"

#include "os/discover_hw.h"


class installer
{
public:
   // Default constructor
   installer();

   // Default destructor
   virtual ~installer();

   int upgrade (versionStruct& fromVer, versionStruct& toVer);

private:
   // Copy constructor
   installer(const installer& obj);

protected:

   const char* findSetting (const char* setting, FILE* fp);

   const char* findSetting (const char* setting, const char* fileName);

   int verifyCrc (const char* commandLine);

   bool updatePostCount ();

   bool replaceCassette (const char* refStr, unsigned int tubingSetCode, const char* barcodeStr);

   bool replaceDatfileLine (const char* datFileName, const char* optionName, const char* newVal, bool addCRC = true);

   bool validateSetConfig (versionStruct& toVer);

   // line by line dat file reader
   int readDatFileVals (CFileLine& cfLine, FILE* fp);

/////////////////////////////////////////////////////

//
// rbc.dat update function
//
   virtual void updateRBC ();

//
// hw.dat update function
//
   virtual void updateHW ();

//
// app_server.dat function
//
   virtual void updateAppServer ();

//
// sw.dat update function
//
   virtual void updateSW ();

//
// terror_config.dat update function
//
   virtual void updateTerror ();

//
// sounds.dat update function
//
   virtual void updateSounds ();

//
// cassette.dat update function
//
   virtual void updateCassette ();

//
// barcode_categories.dat update function
//
   virtual void updateBarcodeCategories ();

//
// rts_config.dat update function
//
   virtual void updateRTSConfig ();

//
// setconfig.dat update function
//
   virtual void updateSetConfig ();

//
// force the copy of the new setconfig.dat
//
   virtual void forceSetConfig ();

//
// vista_ip.dat update function
//
   virtual void updateVista ();

//
// common location for code to copy trap files
//
   virtual void copyTrapFiles ();

//
// check for inconsistent plasma rinseback settings
//
   virtual bool checkPlasmaRB ();


//
// update the config.dat from what it was to what it should be
//
   virtual bool updateConfigGeneric ();

   virtual bool checkRange (const char* section, const char* key, const char* value, std::string& forceVal);

   virtual bool bloodTypeConversionCheck (int bloodType, std::string& newVal);

//
// Main config file update function, calls the others
//
//   virtual void updateConfig(TrimaVersion fromVersion) = 0;

//
// does the file extraction
//
   virtual bool extractUpdateFiles5 ();
   virtual bool extractUpdateFiles6 ();

//
// cal.dat update function
//
   virtual void updateCal5 ();
   virtual void updateCal6 ();

//
// CRC update & checking
//
   virtual bool checkCRC5 ();
   virtual bool checkCRC6 ();

//
// globvars update function
//
   virtual void updateGlobVars ();

//
// check if we can do this upgrade
//
   virtual bool allowedUpgrade (bool ampro, bool versalogic, bool python, bool ocelot);

//
// The idea here is that after an upgrade it's possible to end up
// with PLT config settings with a combined PAS + PLT volume greater than the
// total bag capacity of 1200.  That's not good.  This function mitigates that.
//
   virtual bool checkPasSettings ();

//
// Make sure that RAS settings are within range, since we changed the RAS
// vol range late in development and it has some funky rules (see IT 9113)
//
   virtual bool checkRasSettings ();

//
// Installs a default machine.id file if one is present in the taz file but there
// isn't already one on the machine
//
   virtual bool installMachineId ();

//
// Appends a CRC'd serial number line to a given zipped file
// It doesn't check if the file already has a serial number in it
// It unzips, appends, and rezips the given filename (must include path)
// Set sectionHdr to true to write the [MACHINE_ID] line before the serial number
//
   virtual bool appendSerialNumToZipFile (const char* filename, bool sectionHdr = false);


public:
   enum { MaxIDStringLength = 256 };

protected:

   const char*     currVersion;
   const char*     newVersion;
   int             newBuildRef;
   buildDataStruct newBuildData;
   int             prevBuildRef;
   bool            is64To7Upgrade;
//   buildDataStruct prevBuildData;

};
#endif // INSTALLER_H

/* FORMAT HASH f7090a1e97ff53372e3ad5f156890983 */
