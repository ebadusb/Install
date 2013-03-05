

#include <fstream>
#include <map>
#include <usrLib.h>
#include <cstdio>


#include "updatecassette_dat.h"

#include "filenames.h"        // Get Trima filename locations.
#include "crcgen.h"

#define MAX_NUM_LINES 400

// defines to make 5.1 compile
#ifndef PNAME_SETCONFIGDAT
   #define PNAME_SETCONFIGDAT	CONFIG_CRC_PATH "/setconfig.dat"
#endif // #ifndef PNAME_SETCONFIGDAT

#ifndef PNAME_SETCONFIGDAT_BK
   #define PNAME_SETCONFIGDAT_BK	PNAME_SETCONFIGDAT ".bk"
#endif // #ifndef PNAME_SETCONFIGDAT_BK

#ifndef PNAME_SETCONFIG_CRC
   #define PNAME_SETCONFIG_CRC	CONFIG_CRC_PATH "/setconfig.crc"
#endif // #ifndef PNAME_SETCONFIG_CRC

#ifndef PNAME_SETCONFIG_CRC_BK
   #define PNAME_SETCONFIG_CRC_BK	PNAME_SETCONFIG_CRC ".bk"
#endif // #ifndef PNAME_SETCONFIG_CRC_BK


// THE ADMIN CASSETTE LIST ( IT'S A VECTOR TO PRESERVE ORDER ).
static UPDATE_CASSETTE_VECTOR_TYPE _adminCassettes;


static bool _initialized = false;


//
// Base class definitions
//
UpdateCassetteDat::UpdateCassetteDat()
{
}


UpdateCassetteDat::~UpdateCassetteDat()
{
}


//
// verifies that the main / backup file match
// 
bool UpdateCassetteDat::checkFile(const char* datFilePath, const char* crcFilePath, const char* datFileBkPath, const char* crcFileBkPath)
{
   bool bMainOk = false;
   bool bBackOk = false;
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
   case 1: // Main file Ok, backup is bad.
      {
         // we need to update the backup file with the main one.
         bSuccess = fixFile(fhelper, datFileBkPath, crcFileBkPath);
         break;
      }

   case 2: // Backup file is good, main file is bad.
      {
         // we need to update the main file with the backup one.
         bSuccess = fixFile(fhelperBk, datFilePath, crcFilePath);
         break;
      }

   case 3: // Both files are good, make sure they are the same.
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

   default: // neither file is good, $%#*$#@!*
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
bool UpdateCassetteDat::fixFile(CFileHelper &goodFile, const char *fname, const char *crcfname)
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
// Admin Cassette.dat methods
//
bool AdminUpdateCassetteDat::_fileOK = false;
char AdminUpdateCassetteDat::_fileVersion[80];
bool AdminUpdateCassetteDat::_fileHasChanged = false;


AdminUpdateCassetteDat::AdminUpdateCassetteDat()
{
}



AdminUpdateCassetteDat::~AdminUpdateCassetteDat()
{
}


//
// begin
// 
UPDATE_CASSETTE_VECTOR_ITERATOR AdminUpdateCassetteDat::begin()
{
   return _adminCassettes.begin();
}



//
// end
// 
UPDATE_CASSETTE_VECTOR_ITERATOR AdminUpdateCassetteDat::end()
{
   return _adminCassettes.end();
}



//
// erase
// 
void AdminUpdateCassetteDat::erase(UPDATE_CASSETTE_VECTOR_ITERATOR it)
{            
   AdminUpdateCassetteDat::_fileHasChanged = true;
   _adminCassettes.erase(it);
}



//
// clear
// 
void AdminUpdateCassetteDat::clear()
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
void AdminUpdateCassetteDat::addCassette(const char *refNumber, unsigned int tubingSetCode, const char *barcode)
{
   AdminUpdateCassetteDat::_fileHasChanged = true;
   _adminCassettes.push_back(new UpdateCatalogNumber(refNumber, tubingSetCode, barcode)); 
}



//
// size
// 
unsigned int AdminUpdateCassetteDat::size(void)
{
   return _adminCassettes.size();
}



//
// read
//
void AdminUpdateCassetteDat::read()
{
   ifstream inFile;
   char catNumString[100]="";
   char barcodeString[100]="";
   unsigned int catNum=0, adminCode=0;

    // Match up the CRCs
   if (!checkFile(PNAME_SETCONFIGDAT, PNAME_SETCONFIG_CRC, PNAME_SETCONFIGDAT_BK, PNAME_SETCONFIG_CRC_BK))
   {
//      printf("Error matching cassette file CRC\n");
      _fileOK = false;
      return;
   }

   inFile.open(PNAME_SETCONFIGDAT, ios::in | ios::binary);

   if (!inFile.good())
   {
//      printf("File %s doesn't exist.\n",PNAME_SETCONFIGDAT);
      inFile.close();
      _fileOK = false;
      return;
   }

   int index = 0;
   int iteration = 0;
   while (!inFile.eof())
   {
      if (!inFile.good())
      {
//         printf("Unexpected I/O error reading %s.\n",PNAME_SETCONFIGDAT);
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

      char *nextToken = 0;

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
      catNum = atoi(catNumString);
      adminCode = atoi(strtok_r(NULL, "\t ", &nextToken) );
      sprintf(barcodeString,  strtok_r(NULL, "\t\r\n ", &nextToken) );

//      printf("REF (%d) + admin code (%d) + barcode (%s) --> ", catNum, adminCode, barcodeString);

//      printf("Valid\n");

      _adminCassettes.push_back(new UpdateCatalogNumber(catNumString, adminCode, barcodeString ) );

      index++;
   }

   inFile.close();
//   printf("%d cassettes added to admin table.\n", index);

   _fileOK = true;
}


void AdminUpdateCassetteDat::updateIfChanged()
{ 
    if (_fileHasChanged)
       updateCassetteFile(); 
};



//
// UpdateCassetteFile
//
bool AdminUpdateCassetteDat::updateCassetteFile()
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
      const char *refNumStr = (*iter)->RefNum();
      const char *barcodeStr = (*iter)->BarcodeNum();

      outFile << refNumStr << "\t" << (*iter)->AdminCode() << "   \t";

      int barcodeLength = strlen(barcodeStr);

      if (barcodeLength < 5 || barcodeLength > 80)
      {
         outFile << "0" << endl;
      }
      else
      {
         outFile << barcodeStr << endl;
      }
   }

   if (_fileVersion && strcmp(_fileVersion, "") != 0)
      outFile << "file_version=" << _fileVersion << endl;
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

   crcFile << "0x" << hex << calculatedCrc << dec << endl;
   crcFile.close();

   attrib(PNAME_SETCONFIG_CRC, "+R");

//   printf("CRC (%lx) stored in %s.\n", calculatedCrc, PNAME_SETCONFIG_CRC);
   _fileHasChanged = false;

   return true;
}



//
// decorateFile
// 
void AdminUpdateCassetteDat::decorateFile(ostream &o)
{
   o << "#"                                                    << endl; 
   o << "#\tThis file lists the cassettes configured by the"   << endl;
   o << "#\tadministrator.  It is not a comprehensive list."   << endl;
   o << "#\tRather, it represents the subset of REF numbers"   << endl;
   o << "#\tthat will come up in the listbox on the REF"       << endl;
   o << "#\tnumber entry screen."                              << endl;
   o << "#"                                                    << endl;
   o << "#\tEach cassette has a 5 or 6 digit REF number and"   << endl;
   o << "#\tan 8-digit admin code that encrypts the"           << endl;
   o << "#\tfunctionality of the cassette. Also, a barcode"    << endl;
   o << "#\tstring is available so that those customers who"   << endl;
   o << "#\thave a barcode reader available can scan a"        << endl;
   o << "#\tbarcode instead of manually selecting a REF"       << endl;
   o << "#\tnumber."                                           << endl;
   o << "#"                                                    << endl;
   o << "#\tCassettes can be added or removed through the"     << endl;
   o << "#\tmachine config screen.  If a REF number exists in" << endl;
   o << "#\tboth this file and cassette.dat, but with"         << endl;
   o << "#\tdifferent functionality or barcode listed, this"   << endl;
   o << "#\tfile overrides the other one."                     << endl;
   o << "#"                                                    << endl;    
   o << "#\tA list of REF numbers and the sets they are"       << endl;
   o << "#\tassociated with is available from the MDC"         << endl;
   o << "#\t(part number 377500017)."                          << endl;
   o << "#"                                                    << endl;    
   o << "#\tID\tADMIN CODE   \tBARCODE"                        << endl;
}

int AdminUpdateCassetteDat :: file_crcgen32 (const char *filename, unsigned long *pcrc)
{
	enum { MAXBUFFERLENGTH = 256 };
   unsigned char buffer[MAXBUFFERLENGTH];
   int count;
   int totalBytesRead = 0;

   ifstream inStream (filename, ios::in + ios::binary + ios::nocreate);

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
      count = inStream.gcount();
      totalBytesRead += count;

      // There is something goofy with this stream class.  Fail bit is triggered
      // on the reading of the end of a file.  Don't know what causes it, but
      // ignore fail status.
      if (inStream.bad())
      {
         inStream.close();
         return -1;
      }
      
      crcgen32 (pcrc, buffer, count);
   
   } while ( count == MAXBUFFERLENGTH );

   return 0;
}




