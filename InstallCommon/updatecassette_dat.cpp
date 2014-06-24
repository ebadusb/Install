#include <fstream>
#include <map>
#include <usrLib.h>
#include <cstdio>
#include <iostream>


#include "updatecassette_dat.h"
#include "updatetrimaUtils.h"

#include "filenames.h"        // Get Trima filename locations.
#include "crcgen.h"

extern installLogStream installLog;

#define MAX_NUM_LINES 400

// defines to make 5.1 compile
#ifndef PNAME_CASSETTEDAT
   #define PNAME_CASSETTEDAT    CONFIG_PATH "/cassette.dat"
#endif

#ifndef PNAME_CASSETTEDAT_BK
   #define PNAME_CASSETTEDAT_BK PNAME_CASSETTEDAT ".bk"
#endif

#ifndef PNAME_CASSETTEDAT_CRC
   #define PNAME_CASSETTEDAT_CRC    CONFIG_CRC_PATH "/cassette.crc"
#endif

#ifndef PNAME_CASSETTEDAT_CRC_BK
   #define PNAME_CASSETTEDAT_CRC_BK PNAME_CASSETTEDAT_CRC ".bk"
#endif

#ifndef PNAME_SETCONFIGDAT
   #define PNAME_SETCONFIGDAT   CONFIG_PATH "/setconfig.dat"
#endif

#ifndef PNAME_SETCONFIGDAT_BK
   #define PNAME_SETCONFIGDAT_BK    PNAME_SETCONFIGDAT ".bk"
#endif

#ifndef PNAME_SETCONFIG_CRC
   #define PNAME_SETCONFIG_CRC  CONFIG_CRC_PATH "/setconfig.crc"
#endif

#ifndef PNAME_SETCONFIG_CRC_BK
   #define PNAME_SETCONFIG_CRC_BK   PNAME_SETCONFIG_CRC ".bk"
#endif


// THE MASTER LIST OF CASSETTES
static UPDATE_CASSETTE_MAP_TYPE _cassetteMap;

// THE ADMIN CASSETTE LIST ( IT'S A VECTOR TO PRESERVE ORDER ).
static UPDATE_CASSETTE_VECTOR_TYPE _adminCassettes;


static bool _initialized = false;


//
// Base class definitions
//
UpdateCassetteDat::UpdateCassetteDat()
{}


UpdateCassetteDat::~UpdateCassetteDat()
{}


//
// verifies that the main / backup file match
//
bool UpdateCassetteDat::checkFile (const char* datFilePath, const char* crcFilePath, const char* datFileBkPath, const char* crcFileBkPath)
{
   bool bMainOk  = false;
   bool bBackOk  = false;
   bool bSuccess = false;

   // read in the main file and check CRC
   CFileHelper fhelper;
   bMainOk = fhelper.Read (datFilePath);

   if (bMainOk)
   {
      bMainOk = fhelper.CompareCRC (crcFilePath);
   }

   // read in the backup file and check CRC
   CFileHelper fhelperBk;
   bBackOk = fhelperBk.Read (datFileBkPath);

   if (bBackOk)
   {
      bBackOk = fhelperBk.CompareCRC (crcFileBkPath);
   }

   // There are four cases to deal with
   const int nCase = (bMainOk ? 1 : 0) + (bBackOk ? 2 : 0);

   switch (nCase)
   {
      case 1 : // Main file Ok, backup is bad.
      {
         // we need to update the backup file with the main one.
         bSuccess = fixFile(fhelper, datFileBkPath, crcFileBkPath);
         break;
      }

      case 2 : // Backup file is good, main file is bad.
      {
         // we need to update the main file with the backup one.
         bSuccess = fixFile(fhelperBk, datFilePath, crcFilePath);
         break;
      }

      case 3 : // Both files are good, make sure they are the same.
      {
         if (fhelper.CRC() != fhelperBk.CRC())
         {
            // assume the main one is the correct one.
            bSuccess = fixFile(fhelper, datFileBkPath, crcFileBkPath);
         }
         else
         {
            // both match and are fine
            bSuccess = true;
            break;
         }
         break;
      }

      default : // neither file is good, $%#*$#@!*
      {
         bSuccess = false;
         break;
      }
   }

   return bSuccess;
}



//
// attempts to fix a backup file with a good file including crc
//
bool UpdateCassetteDat::fixFile (CFileHelper& goodFile, const char* fname, const char* crcfname)
{
   bool bSuccess = true;
   char crcTxt[15];

   // obtain text copy of the crc
   CFileHelper fCRC;
   goodFile.CRC(crcTxt);

   // put it into the empty crc file
   fCRC.Append (crcTxt);
   fCRC.Append ("\n");

   // write file to backup location and crc to backup file
   bSuccess = goodFile.FastWrite (fname) && fCRC.FastWrite (crcfname, false);

   return bSuccess;
}




//
// Master Cassette.dat methods
//
bool MasterUpdateCassetteDat::_fileOK          = false;
char MasterUpdateCassetteDat::_fileVersion[80] = {};

MasterUpdateCassetteDat::MasterUpdateCassetteDat()
{}



MasterUpdateCassetteDat::~MasterUpdateCassetteDat()
{}



//
// begin
//
UPDATE_CASSETTE_MAP_ITERATOR MasterUpdateCassetteDat::begin ()
{
   return _cassetteMap.begin();
}



//
// end
//
UPDATE_CASSETTE_MAP_ITERATOR MasterUpdateCassetteDat::end ()
{
   return _cassetteMap.end();
}



//
// find
//
UPDATE_CASSETTE_MAP_ITERATOR MasterUpdateCassetteDat::find (const char* cassetteIdString)
{
   return _cassetteMap.find(cassetteIdString);
}



//
// read
//
void MasterUpdateCassetteDat::read ()
{
   char         loggingBuff[256];

   int          numberOfEntries = 0, lineNumber = 0;
   unsigned int catNum          = 0, adminCode = 0;

   char*        token           = 0;
   char*        barcodeString   = 0;
   char*        catNumString    = 0;
   char         buffer[256];
   char         line[256];

   updatetrimaUtils::logger("Reading cassette.dat\n");

   // This causes problems for Install so remove it
   // Match the CRC calculated from cassette.dat with the CRC value stored in cassette.crc.
/*   if (!checkFile(PNAME_CASSETTEDAT, PNAME_CASSETTEDAT_CRC, PNAME_CASSETTEDAT_BK, PNAME_CASSETTEDAT_CRC_BK))
   {
      updatetrimaUtils::logger("MasterUpdateCassetteDat::read Error matching cassette file CRC\n");
      _fileOK = false;
      return;
   }
*/
   ifstream inFile;
   inFile.open(PNAME_CASSETTEDAT, ios::in | ios::binary);

   if (!inFile.good())
   {
      updatetrimaUtils::logger("MasterUpdateCassetteDat::read File: ", PNAME_CASSETTEDAT, " doesn't exist\n");
      inFile.close();
      _fileOK = false;
      return;
   }

   while (!inFile.eof())
   {
      lineNumber++;

      if (lineNumber > MAX_NUM_LINES) // Make sure the read loop terminates.
      {
         inFile.close();
         updatetrimaUtils::logger("MasterUpdateCassetteDat::read cassette.dat read loop wouldn't terminate\n");
         _fileOK = false;
         return;
      }

      if (!inFile.good())
      {
         // Log error
         updatetrimaUtils::logger("MasterUpdateCassetteDat::read Unexpected I/O error reading ", PNAME_CASSETTEDAT, "\n");
         inFile.close();
         _fileOK = false;
         return;
      }

      char* nextToken = 0;

      inFile.getline(buffer, 256);

      // Ignore comments, file version line, and blank lines.
      if (!inFile.good())
         continue;
      else if ((buffer[0] == '#') || (buffer[0] == 13)  || (buffer[0] == 10)  || (buffer[0] == 0)) // # or CR/LF
         continue;
      else if (strstr(buffer, "file_version") != NULL)
      {
         printf("File %s %s.\n", PNAME_CASSETTEDAT, buffer);
         continue;
      }

      strcpy(line, buffer);  // Copy line to print out in case of error.

      // Catalog number
      token = strtok_r(buffer, "\t ", &nextToken);

      if (token == 0)
      {
         printf("Bad Token: %s line read was[%s] at line number %d.\n", token,  buffer, lineNumber);
         _fileOK = false;
         return;
      }

      catNumString = token;
      catNum       = atoi(token);

      // Admin code
      token = strtok_r(NULL, "\t ", &nextToken);

      if (token == 0)
      {
         printf("Bad Token: %s in line %s at line number %d.\n", token,  line, lineNumber);
         _fileOK = false;
         return;
      }

      adminCode = (unsigned int)strtoul(token, NULL, 10);

      // Barcode string
      token = strtok_r(NULL, "\t\r\n ", &nextToken);

      if (token == 0)
      {
         printf("Bad Token: %s in line %s at line number %d.\n", token,  line, lineNumber);
         _fileOK = false;
         return;
      }

      barcodeString = token;

//      sprintf(loggingBuff, "Read Cassette %s %d %s\n", catNumString, adminCode, barcodeString );
//      updatetrimaUtils::logger(loggingBuff);

      _cassetteMap[catNumString] = new UpdateCatalogNumber(catNumString, adminCode, barcodeString);

      numberOfEntries++;
   }

   inFile.close();
   sprintf(loggingBuff, "Read %d rows from cassette.dat\n", numberOfEntries);
   updatetrimaUtils::logger(loggingBuff);

   _fileOK = true;
}





//
// clear
//
void MasterUpdateCassetteDat::clear ()
{
   // Delete all items in the cassette map structure
   for (UPDATE_CASSETTE_MAP_ITERATOR it = _cassetteMap.begin(); it != _cassetteMap.end(); ++it)
   {
      delete it->second;
   }

   _cassetteMap.clear();

   _fileOK         = false;

   _fileVersion[0] = 0;
}



//
// Admin Cassette.dat methods
//
bool AdminUpdateCassetteDat::_fileOK         = false;
char AdminUpdateCassetteDat::_fileVersion[80];
bool AdminUpdateCassetteDat::_fileHasChanged = false;


AdminUpdateCassetteDat::AdminUpdateCassetteDat()
{}



AdminUpdateCassetteDat::~AdminUpdateCassetteDat()
{}


//
// begin
//
UPDATE_CASSETTE_VECTOR_ITERATOR AdminUpdateCassetteDat::begin ()
{
   return _adminCassettes.begin();
}



//
// end
//
UPDATE_CASSETTE_VECTOR_ITERATOR AdminUpdateCassetteDat::end ()
{
   return _adminCassettes.end();
}



//
// erase
//
void AdminUpdateCassetteDat::erase (UPDATE_CASSETTE_VECTOR_ITERATOR it)
{
   AdminUpdateCassetteDat::_fileHasChanged = true;
   _adminCassettes.erase(it);
}



//
// clear
//
void AdminUpdateCassetteDat::clear ()
{
   AdminUpdateCassetteDat::_fileHasChanged = true;

   // Delete all items in the admin cassettes structure
   for (UPDATE_CASSETTE_VECTOR_ITERATOR it = _adminCassettes.begin(); it != _adminCassettes.end(); ++it)
   {
      delete *it;
   }

   _adminCassettes.clear();
}



//
//  addCassette
//
void AdminUpdateCassetteDat::addCassette (const char* refNumber, unsigned int tubingSetCode, const char* barcode)
{
   AdminUpdateCassetteDat::_fileHasChanged = true;
   _adminCassettes.push_back(new UpdateCatalogNumber(refNumber, tubingSetCode, barcode));
}



//
// size
//
unsigned int AdminUpdateCassetteDat::size (void)
{
   return _adminCassettes.size();
}



//
// read
//
void AdminUpdateCassetteDat::read ()
{
   char         loggingBuff[256];

   ifstream     inFile;
   char         catNumString[100]  = "";
   char         barcodeString[100] = "";
   unsigned int catNum             = 0, adminCode = 0;

   // This causes problems for Install so remove it
   // Match up the CRCs
/*   if (!checkFile(PNAME_SETCONFIGDAT, PNAME_SETCONFIG_CRC, PNAME_SETCONFIGDAT_BK, PNAME_SETCONFIG_CRC_BK))
   {
      updatetrimaUtils::logger("AdminUpdateCassetteDat::read Error matching cassette file CRC\n");
      _fileOK = false;
      return;
   }
*/
   inFile.open(PNAME_SETCONFIGDAT, ios::in | ios::binary);

   if (!inFile.good())
   {
      updatetrimaUtils::logger("AdminUpdateCassetteDat::read ", PNAME_SETCONFIGDAT, " doesn't exist\n");
      inFile.close();
      _fileOK = false;
      return;
   }

   int index     = 0;
   int iteration = 0;
   while (!inFile.eof())
   {
      if (!inFile.good())
      {
         updatetrimaUtils::logger("AdminUpdateCassetteDat::read Unexpected I/O error reading ", PNAME_SETCONFIGDAT, "\n");
         inFile.close();
         _fileOK = false;

         return;
      }

      char buffer[256];

      inFile.getline(buffer, 256);

      // Ignore comments, file version line, and blank lines.
      if (buffer[0] == '#'
          || strcmp(buffer, "") == 0
          ) continue;

      char* nextToken = 0;

      if (strstr(buffer, "file_version") != NULL)
      {
         bzero(_fileVersion, sizeof(_fileVersion));

         strtok_r(buffer, "=", &nextToken); // chop off the first chunk of "buffer".
         sprintf(_fileVersion, strtok_r(NULL, "\t\r\n ", &nextToken) );

         if (!_fileVersion || strcmp(_fileVersion, "") == 0)
         {
//            printf("File Version was empty.\n");
            bzero(_fileVersion, sizeof(_fileVersion));
         }
         else
         {
//            printf("File Version: %s\n", _fileVersion);
         }

         continue;
      }

      sprintf(catNumString, strtok_r(buffer, "\t ", &nextToken) );
      catNum    = atoi(catNumString);
      adminCode = atoi(strtok_r(NULL, "\t ", &nextToken) );
      sprintf(barcodeString,  strtok_r(NULL, "\t\r\n ", &nextToken) );

//      sprintf(loggingBuff, "Read Cassette %d %d %s\n", catNum, adminCode, barcodeString );
//      updatetrimaUtils::logger(loggingBuff);


      _adminCassettes.push_back(new UpdateCatalogNumber(catNumString, adminCode, barcodeString) );

      index++;
   }

   inFile.close();
   sprintf(loggingBuff, "Read %d rows from setconfig.dat\n", index);
   updatetrimaUtils::logger(loggingBuff);

   _fileOK = true;
}


void AdminUpdateCassetteDat::updateIfChanged ()
{
   if (_fileHasChanged)
      updateCassetteFile();
}



//
// UpdateCassetteFile
//
bool AdminUpdateCassetteDat::updateCassetteFile ()
{

   if (!_fileOK)
   {
      _fileHasChanged = false;
//      printf("setconfig file not OK.\n");
      return false;
   }

   // First, delete the CRC file to invalidate the old file.
   remove(PNAME_SETCONFIG_CRC);

   // Next, create the new file.
   attrib(PNAME_SETCONFIGDAT, "-R");
   ofstream outFile;
   outFile.open(PNAME_SETCONFIGDAT, ios::out | ios::trunc | ios::binary, 0777);

   if (!outFile.good())
   {
//      printf("output file not opened OK.\n");
      _fileHasChanged = false;
      return false;
   }

   decorateFile(outFile);

   for (UPDATE_CASSETTE_VECTOR_ITERATOR iter = _adminCassettes.begin(); iter != _adminCassettes.end(); ++iter)
   {
      const char* refNumStr  = (*iter)->RefNum();
      const char* barcodeStr = (*iter)->BarcodeNum();

      outFile << refNumStr << "\t" << (*iter)->AdminCode() << "   \t";

      int barcodeLength = strlen(barcodeStr);

      if (barcodeLength < 5 || barcodeLength > 80)
      {
         outFile << "0" << std::endl;
      }
      else
      {
         outFile << barcodeStr << std::endl;
      }
   }

   if (_fileVersion && strcmp(_fileVersion, "") != 0)
      outFile << "file_version=" << _fileVersion << std::endl;
//   else
//      printf("No file version to report, so not outputting one.\n");

   outFile.close();
   attrib(PNAME_SETCONFIGDAT, "+R");

//   printf("File %s updated successfully with %d entries.\n", PNAME_SETCONFIGDAT, _adminCassettes.size());

   unsigned long calculatedCrc = INITCRC_DEFAULT;

   if (AdminUpdateCassetteDat::file_crcgen32(PNAME_SETCONFIGDAT, &calculatedCrc) == -1)
   {
//      printf("CRC couldn't be generated for %s. Operation failed.\n", PNAME_SETCONFIGDAT);

      _fileHasChanged = false;
      return false;
   }

   // Store the computed CRC in the crc file

   attrib(PNAME_SETCONFIG_CRC, "-R");

   ofstream crcFile;
   crcFile.open(PNAME_SETCONFIG_CRC, ios::out | ios::trunc, 0777);

   if (!crcFile.good())
   {
      _fileHasChanged = false;
      return false;
   }

   crcFile << "0x" << std::hex << calculatedCrc << std::dec << std::endl;
   crcFile.close();

   attrib(PNAME_SETCONFIG_CRC, "+R");

//   printf("CRC (%lx) stored in %s.\n", calculatedCrc, PNAME_SETCONFIG_CRC);
   _fileHasChanged = false;

   return true;
}



//
// decorateFile
//
void AdminUpdateCassetteDat::decorateFile (ostream& o)
{
   o << "#"                                                    << std::endl;
   o << "#\tThis file lists the cassettes configured by the"   << std::endl;
   o << "#\tadministrator.  It is not a comprehensive list."   << std::endl;
   o << "#\tRather, it represents the subset of REF numbers"   << std::endl;
   o << "#\tthat will come up in the listbox on the REF"       << std::endl;
   o << "#\tnumber entry screen."                              << std::endl;
   o << "#"                                                    << std::endl;
   o << "#\tEach cassette has a 5 or 6 digit REF number and"   << std::endl;
   o << "#\tan 8-digit admin code that encrypts the"           << std::endl;
   o << "#\tfunctionality of the cassette. Also, a barcode"    << std::endl;
   o << "#\tstring is available so that those customers who"   << std::endl;
   o << "#\thave a barcode reader available can scan a"        << std::endl;
   o << "#\tbarcode instead of manually selecting a REF"       << std::endl;
   o << "#\tnumber."                                           << std::endl;
   o << "#"                                                    << std::endl;
   o << "#\tCassettes can be added or removed through the"     << std::endl;
   o << "#\tmachine config screen.  If a REF number exists in" << std::endl;
   o << "#\tboth this file and cassette.dat, but with"         << std::endl;
   o << "#\tdifferent functionality or barcode listed, this"   << std::endl;
   o << "#\tfile overrides the other one."                     << std::endl;
   o << "#"                                                    << std::endl;
   o << "#\tA list of REF numbers and the sets they are"       << std::endl;
   o << "#\tassociated with is available from the MDC"         << std::endl;
   o << "#\t(part number 377500017)."                          << std::endl;
   o << "#"                                                    << std::endl;
   o << "#\tID\tADMIN CODE   \tBARCODE"                        << std::endl;
}

int AdminUpdateCassetteDat::file_crcgen32 (const char* filename, unsigned long* pcrc)
{
   enum { MAXBUFFERLENGTH = 256 };
   char     buffer[MAXBUFFERLENGTH];
   int      count;
   int      totalBytesRead = 0;

   ifstream inStream (filename, ios::in | ios::binary | ios::nocreate);

   if (inStream.fail())
   {
      inStream.close();
      return -1;
   }

   // Start at the beginning of the stream.
   inStream.seekg(0);

   // Read in the stream on block at a time.
   do
   {
      inStream.read (buffer, MAXBUFFERLENGTH);
      count           = inStream.gcount();
      totalBytesRead += count;

      // There is something goofy with this stream class.  Fail bit is triggered
      // on the reading of the end of a file.  Don't know what causes it, but
      // ignore fail status.
      if (inStream.bad())
      {
         inStream.close();
         return -1;
      }

      crcgen32 (pcrc, (const unsigned char*)buffer, count);

   } while ( count == MAXBUFFERLENGTH );

   return 0;
}

/* FORMAT HASH 079979149177012ce4353c0557f3ecb1 */
