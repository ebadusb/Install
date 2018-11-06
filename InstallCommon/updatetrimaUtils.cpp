/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include <iostream>
#include <time.h>

#include "updatetrima.h"
#include "updatetrimaUtils.h"
#include "updateTrimaDefines.h"
#include "updatetrimaData.h"

#include "filenames.h"

// #include "targzextract.c"

// #include "crc.c"
#include <fstream>

#include "zlib.h"

using namespace std;

// ofstream logStream;

#ifdef __cplusplus
extern "C" {
#endif

int cp (const char* from, const char* to);
int copyFileContiguous (const char* from, const char* to);

int softcrc (const char* options);

#ifdef __cplusplus
};
#endif

// stringify macros
#define strfy1(x) strfy2(x)
#define strfy2(x) #x

#ifndef INSTALL_BUILD_DATE
   #define INSTALL_BUILD_DATE "build date not available"
#endif
#ifndef INSTALL_BUILD_USER
   #define INSTALL_BUILD_USER "user name not available"
#endif
#ifndef INSTALL_BUILD_MACHINE
   #define INSTALL_BUILD_MACHINE "full computer name not available - possibly Vx 5.5 Build"
#endif

extern installLogStream installLog;

extern "C" STATUS xdelete (const char* fileName);

extern bool development_only;

extern installLogStream installLog;

//bool  updatetrimaUtils::loggingEnabled      = false;
//bool  updatetrimaUtils::logToScreen         = false;
bool  updatetrimaUtils::development_install = false;
//FILE* updatetrimaUtils::logFile             = NULL;


fourPartVersion::fourPartVersion()
   : first(0),
     second(0),
     third(0),
     fourth(0)
{}

fourPartVersion::~fourPartVersion()
{}

fourPartVersion::fourPartVersion(const fourPartVersion& obj)
   : first(obj.first),
     second(obj.second),
     third(obj.third),
     fourth(obj.fourth)
{}

fourPartVersion::fourPartVersion(const char* verString)
   : first(0),
     second(0),
     third(0),
     fourth(0)
{
   char* tokPtr = NULL;
   char  tmpNum[50];

   // Make a copy we can modify
   strcpy(tmpNum, verString);

   // Read the first
   if ( (tokPtr = strtok(tmpNum, ".")) != NULL )
   {
      first = atoi(tokPtr);
   }

   // Read the second
   if ( (tokPtr = strtok(NULL, ". ")) != NULL )
   {
      second = atoi(tokPtr);
   }

   // Read the third
   if ( (tokPtr = strtok(NULL, ". ")) != NULL )
   {
      third = atoi(tokPtr);
   }

   // Read the fourth
   if ( (tokPtr = strtok(NULL, ". ")) != NULL )
   {
      fourth = atoi(tokPtr);
   }

}

bool fourPartVersion::operator > (const fourPartVersion& other) const
{
   if ( first > other.first )
   {
      return true;
   }
   else if ( first == other.first )
   {
      if ( second > other.second )
      {
         return true;
      }
      else if ( second == other.second )
      {
         if ( third > other.third )
         {
            return true;
         }
         else if ( third == other.third )
         {
            if ( fourth > other.fourth )
            {
               return true;
            }
            else
            {
               return false;
            }
         }
         else
         {
            return false;
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
}

bool fourPartVersion::operator == (const fourPartVersion& other) const
{
   return( first == other.first && second == other.second && third == other.third && fourth == other.fourth );
}


// Default constructor
updatetrimaUtils::updatetrimaUtils()
{}

// Copy constructor
updatetrimaUtils::updatetrimaUtils(const updatetrimaUtils& obj)
{}

// Default destructor
updatetrimaUtils::~updatetrimaUtils()
{}


int updatetrimaUtils::copyFileContiguous (const char* from, const char* to)
{
#if CPU!=SIMNT
   int fromFD = open(from, O_RDONLY, 0644);

   attrib(to, "-R");
   int toFD = open(to, O_CREAT | O_RDWR, 0644);

   struct stat fileStat;
   char        buffer[512];
   int         result = ERROR;

   if ( fromFD >= 0 && toFD >= 0 )
   {
      long bytesCopied = 0;
      int  bytesRead;

      fstat(fromFD, &fileStat);
      printf("copying %s to %s: file length = %ld bytes\n", from, to, fileStat.st_size);

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
         printf("Copy failed (%ld bytes written)\n", bytesCopied);
      }
      else
      {
         result = bytesCopied;
      }
   }
   else
   {
      if ( fromFD < 0 )
      {
         perror(from);
      }

      if ( toFD < 0 )
      {
         perror(to);
      }
   }

   if ( fromFD >= 0 )
   {
      close(fromFD);
   }

   if ( toFD >= 0 )
   {
      close(toFD);
   }

   return result;
#else
// in the Sim we don't care about the contiguous, just to get the file there
   cp(from, to);

//   char cmdStr[256];
//   sprintf(cmdStr, "copy /B /Y %s %s", from, to );
//   installLog << "copy command: " << cmdStr << "\n";
//   system(cmdStr);
   return 0;
#endif
}

int updatetrimaUtils::unzipFile (const char* from, const char* to)
{
   installLog << "unzipping " << from << " to " << to << "\n";

   gzFile fromFD = gzopen(from, "r");
   int    toFD   = open(to, O_CREAT | O_RDWR, 0644);

   if ( fromFD >= 0 && toFD >= 0 )
   {
      int  bytesRead;
      int  bytesWritten  = 0;

      const int bufferSize = 1024;
      char * buffer = new char[bufferSize];

      while ( (bytesRead = gzread(fromFD, buffer, bufferSize)) > 0 )
      {
         bytesWritten += write(toFD, buffer, bytesRead);
      }

      delete[] buffer;

      installLog << "Uncompression complete\n";
      gzclose(fromFD);
      close(toFD);

      return 1;
   }
   else
   {
      if ( fromFD < 0 )
      {
         perror(from);
         installLog << "file " << from << " reports " << strerror(errno) << "\n";
      }
      else
      {
         gzclose(fromFD);
      }

      if ( toFD < 0 )
      {
         perror(to);
         installLog << "file " << to << " reports " << strerror(errno) << "\n";
      }
      else
      {
         close(toFD);
      }
   }

   return 0;
}


int updatetrimaUtils::zipFile (const char* from, const char* to)
{
   installLog << "Zipping " << from << " to " << to << "\n";
   int    fromFD = open(from, O_RDONLY, 0644);
   gzFile toFD   = gzopen(to, "w");

   if ( fromFD >= 0 && toFD >= 0 )
   {
      int  bytesRead;
      int  bytesWritten = 0;

      const int bufferSize = 1024;
      char * buffer = new char[bufferSize];

      while ( (bytesRead = read(fromFD, buffer, bufferSize)) > 0 )
      {
         bytesWritten += gzwrite(toFD, buffer, bytesRead);
      }

      delete[] buffer;

      printf("\tCompression complete\n");
      installLog << "Compression complete\n";
      gzclose(toFD);
      close(fromFD);

      return 1;
   }
   else
   {
      if ( fromFD < 0 )
      {
         perror(from);
         installLog << "file " << from << " reports " << strerror(errno) << "\n";
      }
      else
      {
         close(fromFD);
      }

      if ( toFD < 0 )
      {
         perror(to);
         installLog << "file " << to << " reports " << strerror(errno) << "\n";
      }
      else
      {
         gzclose(toFD);
      }
   }

   return 0;
}

FileCallBackStatus updatetrimaUtils::update_clean_file (const char* fullPathName)
{

#if CPU!=SIMNT
   struct stat fileStat;

   if ( stat((char*)fullPathName, &fileStat) == OK &&
        (fileStat.st_mode & S_IFDIR) != 0 )
   {
      /*
       *  Recurse through sub-directories to make sure all files are removed
       */
      fileSort(fullPathName, FILE_SORT_BY_DATE_ASCENDING, update_clean_file);
   }

//   installLog << "Trying to delete " << fullPathName << "\n";
   attrib(fullPathName, "-R");
   xdelete(fullPathName);
#endif
   return FILE_CALLBACK_CONTINUE;
}

FileCallBackStatus updatetrimaUtils::update_file_set_rdwrite (const char* fullPathName)
{
#if CPU!=SIMNT
   struct stat fileStat;

   if ( stat((char*)fullPathName, &fileStat) == OK &&
        (fileStat.st_mode & S_IFDIR) != 0 )
   {
      fileSort(fullPathName, FILE_SORT_BY_DATE_ASCENDING, update_file_set_rdwrite);
   }

   attrib(fullPathName, "-R");
#endif
   return FILE_CALLBACK_CONTINUE;
}

FileCallBackStatus updatetrimaUtils::update_file_set_rdonly (const char* fullPathName)
{
#if CPU!=SIMNT
   struct stat fileStat;

   if ( stat((char*)fullPathName, &fileStat) == OK &&
        (fileStat.st_mode & S_IFDIR) != 0 )
   {
      fileSort(fullPathName, FILE_SORT_BY_DATE_ASCENDING, update_file_set_rdonly);
      attrib(fullPathName, "-R");
   }
   else
   {
      attrib(fullPathName, "+R");
   }
#endif
   return FILE_CALLBACK_CONTINUE;
}

bool updatetrimaUtils::parseRevision (const char* revString, int& curMajorRev, int& curMinorRev, int& curBuild)
{
   bool  retval = true;
   char* tokPtr = NULL;
   char  tmpNum[50];

   // Make a copy we can modify
   strcpy(tmpNum, revString);

   // Read the majorRev
   if ( (tokPtr = strtok(tmpNum, ".")) != NULL )
   {
      curMajorRev = atoi(tokPtr);
   }

   // Read the minorRev
   if ( (tokPtr = strtok(NULL, ". ")) != NULL )
   {
      curMinorRev = atoi(tokPtr);
   }

   // Read the build
   if ( (tokPtr = strtok(NULL, ". ")) != NULL )
   {
      curBuild = atoi(tokPtr);
   }
   else
   {
      // If there's no build then the minorRev was the build (the format was <majorRev>.<build>)
      curBuild    = curMinorRev;
      curMinorRev = 0;
   }
   return true;

//    return retval;
}

buildDataStruct& updatetrimaUtils::getBuildData (int buildRef)
{
   return buildData[buildRef];
}

rangeStruct& updatetrimaUtils::getNextRangeData (bool reset)
{
   static int counter = 0;

   if ( reset )
   {
      counter = 0;
   }

   return rangeData[counter++];
}

bool updatetrimaUtils::getBuildInfo (versionStruct& versionInfo, int& buildRef)
{
   // get the upgrade data for the version given
   // this gets the build info for the version that matches the major/minor revs and
   // has the largest build number that isn't larger than the new build
   int  cntr            = 0;
   int  largestMinorRev = 0;
   int  largestBuild    = 0;
   int  toMajorRev      = 0;
   int  toMinorRev      = 0;
   int  toBuild         = 0;
   bool foundBuild      = false;

   installLog << "Looking up build info for buildNum ="
              << " " << versionInfo.majorRev
              << "." << versionInfo.minorRev
              << "." << versionInfo.buildNum
              << "\n";

   while ( strcmp(buildData[cntr].buildNum, "END") != 0 )
   {
      updatetrimaUtils::parseRevision(buildData[cntr].buildNum, toMajorRev, toMinorRev, toBuild);

      if ( toMajorRev == versionInfo.majorRev )
      {
         if ( toMinorRev >= largestMinorRev && versionInfo.minorRev >= toMinorRev )
         {
            foundBuild      = true;
            largestMinorRev = toMinorRev;
            buildRef        = cntr;

            if (toBuild > largestBuild && versionInfo.buildNum >= toBuild)
            {
               buildRef     = cntr;
               largestBuild = toBuild;
            }
//            installLog << "Build info best so far ref: " << buildRef << " ver: " << toMajorRev << "." << toMinorRev << "." << toBuild << "\n";
         }
      }
      cntr++;
   }

   if ( !foundBuild )
   {
//      installLog << "Build info match not found.\n";
      return(false);
   }
   else
   {
//      installLog << "Build info best match ref: " << buildRef << " ver: " << toMajorRev << "." << toMinorRev << "." << toBuild << "\n";
      return(true);
   }

}

/*
void updatetrimaUtils::logger (const char* stuff1, const char* stuff2, const char* stuff3, const char* stuff4)
{
   char tmpBuff[256];
   sprintf(tmpBuff, "%s%s%s%s", stuff1, stuff2, stuff3, stuff4);
   updatetrimaUtils::logger(tmpBuff);
}

void updatetrimaUtils::logger (const char* stuff1, const char* stuff2, const char* stuff3)
{
   char tmpBuff[256];
   sprintf(tmpBuff, "%s%s%s", stuff1, stuff2, stuff3);
   updatetrimaUtils::logger(tmpBuff);
}

void updatetrimaUtils::logger (const char* stuff1, int stuff2)
{
   char tmpBuff[256];
   sprintf(tmpBuff, "%s%d", stuff1, stuff2);
   updatetrimaUtils::logger(tmpBuff);
}

void updatetrimaUtils::logger (const char* stuff1, const char* stuff2)
{
   char tmpBuff[256];
   sprintf(tmpBuff, "%s%s", stuff1, stuff2);
   updatetrimaUtils::logger(tmpBuff);
}

void updatetrimaUtils::logger (unsigned long stuff)
{
   char tmpBuff[256];
   sprintf(tmpBuff, "%lu", stuff);
   updatetrimaUtils::logger(tmpBuff);
}

void updatetrimaUtils::logger (int stuff)
{
   char tmpBuff[256];
   sprintf(tmpBuff, "%d", stuff);
   updatetrimaUtils::logger(tmpBuff);
}

void updatetrimaUtils::logger (float stuff)
{
   char tmpBuff[256];
   sprintf(tmpBuff, "%f", stuff);
   updatetrimaUtils::logger(tmpBuff);
}

void updatetrimaUtils::logger (const char* stuff)
{
   // use the new stream class
   installLog << stuff;
//    if ( updatetrimaUtils::logToScreen ) {
//        cerr << stuff;
//    }
//
//    if ( updatetrimaUtils::loggingEnabled ) {
//        if ( updatetrimaUtils::logFile ) {
//            fprintf(updatetrimaUtils::logFile, "%s", stuff);
//            fflush(updatetrimaUtils::logFile);
//        }
//    }
}


bool updatetrimaUtils::initLogging ()
{
   mkdir(INSTALL_LOG_PATH);

   char fileNameBuffer[256];
   fileNameBuffer[255] = 0;
   time_t _now = time(NULL);
   sprintf(fileNameBuffer, "%s", PNAME_INSTALL_LOG);

   updatetrimaUtils::logFile = fopen(fileNameBuffer, "w");

   if ( !updatetrimaUtils::logFile )
   {
      cerr << "File not opened" << endl;
      return false;
   }
   else
   {
      char outBuff[256];
      outBuff[255] = 0;
      strftime(outBuff, sizeof(outBuff), "Trima Install Log %Y-%m-%d %H:%M:%S", localtime(&_now));
      fprintf(updatetrimaUtils::logFile, "%s\n\n", outBuff);

      return true;
   }

}

void updatetrimaUtils::closelogger ()
{
   if ( updatetrimaUtils::logFile )
   {
      fflush(updatetrimaUtils::logFile);
      fclose(updatetrimaUtils::logFile);
   }
   updatetrimaUtils::logFile = NULL;
}
*/


void updatetrimaUtils::logFileHeader (string& hdrStr)
{
   time_t _now = time(NULL);

   char outBuff[256];
   outBuff[255] = 0;
   strftime(outBuff, sizeof(outBuff), "Trima Install Log %Y-%m-%d %H:%M:%S", localtime(&_now));

   hdrStr = outBuff;
}



///////////////////////////////////////////////////////////////////
// file log stream class
// I didn't need to make it so it will open more than one file, but I just got carried away
// I tested it with multiple files, so it works if it's ever needed

installLogStream::installLogStream()
   : logToScreen(true),
     streamVect(),
     currLogLevel(NORMAL),
     maxLogLevel(NORMAL)
{}

installLogStream::installLogStream(bool logToScrn)
   : logToScreen(logToScrn),
     streamVect(),
     currLogLevel(NORMAL),
     maxLogLevel(NORMAL)
{}

installLogStream::~installLogStream()
{
   for ( vector<ofstream*>::iterator iter = streamVect.end(); iter != streamVect.begin(); iter-- )
   {
      (*iter)->close();
      delete (*iter);
   }
}

bool installLogStream::open (const char* filename)
{
   char usrStr[256];
   memset(usrStr, 0, sizeof(usrStr));
   strcpy(usrStr, INSTALL_BUILD_USER);

   char machStr[256];
   memset(machStr, 0, sizeof(machStr));
   strcpy(machStr, INSTALL_BUILD_MACHINE);

   char* namePtr = 0;
   namePtr = strtok(usrStr+9, " ");  // +9 to skip the "User name" & strtok to skip the spaces

   char* machPtr = 0;
   machPtr = strtok(machStr+18, " .");  // +18 to skip the "Full Computer name" & strtok to skip the spaces + kill the .'s

   bool retval = false;

   mkdir(INSTALL_LOG_PATH);

   ofstream* newStream = new ofstream(filename);
   if ( newStream->is_open() )
   {
#if CPU==SIMNT
      *newStream << "----------------- SIMULATOR INSTALL -----------------\n";
#endif
      string hdrString;
      updatetrimaUtils::logFileHeader(hdrString);
      *newStream << hdrString.c_str() << "\n";
      *newStream << "Install Build Date: " << INSTALL_BUILD_DATE << "\n";
      *newStream << "Install Build User: " << namePtr << "\n";
      *newStream << "Install Build Machine: " << machPtr << "\n\n";
      newStream->flush();

      streamVect.push_back(newStream);
      retval = true;
   }

   return retval;
}

bool installLogStream::is_open ()
{
   bool retval = false;

   if ( streamVect.size() > 0 )
   {
      retval = true;
   }

   return retval;
}

void installLogStream::close ()
{
//   printf("installLogStream::close\n");

   for ( vector<ofstream*>::iterator iter = streamVect.begin(); iter != streamVect.end(); iter++ )
   {
      if ( (*iter)->is_open() )
      {
         (*iter)->close();
      }
//      delete (*iter);
   }

}

void installLogStream::setMaxLogLevel (installLogLevel logLevel)
{
   maxLogLevel = logLevel;
}

// this actually does the job of writing
installLogStream& installLogStream::operator << (const char* stuff)
{
   if (currLogLevel <= maxLogLevel)
   {
      if (logToScreen) 
      {
         cerr << stuff;
      }

      for ( vector<ofstream*>::iterator iter = streamVect.begin(); iter != streamVect.end(); iter++ )
      {
         *(*iter) << stuff;
         (*iter)->flush();
      }
   }

   // if there's a \n in the string, reset the loglevel after writing
   string newStr(stuff);
   if (newStr.find("\n") != string::npos)
   {
      currLogLevel = NORMAL;
   }

   return *this;
}

installLogStream& installLogStream::operator << (const string& stuff)
{

   return (installLog << stuff.c_str());
}

installLogStream& installLogStream::operator << (const long stuff)
{
   char writebuffer[64];

   sprintf(writebuffer, "%ld", stuff);

   return (installLog << writebuffer);

}

installLogStream& installLogStream::operator << (const int stuff)
{

   return (installLog << (long)stuff);
}

installLogStream& installLogStream::operator << (const unsigned int stuff)
{

   return (installLog << (long)stuff);
}

installLogStream& installLogStream::operator << (const float stuff)
{
   char writebuffer[64];

   sprintf(writebuffer, "%.3f", stuff);

   return (installLog << writebuffer);
}

installLogStream& installLogStream::operator << (const double stuff)
{

   return (installLog << (float)stuff);
}

installLogStream& installLogStream::operator << (installLogLevel stuff)
{
   currLogLevel = stuff;
   return (installLog);
}

installLogStream& installLogStream::operator << (installLogDirective stuff)
{
   if (stuff == endline)
   {
      currLogLevel = NORMAL;
   }
   return (installLog);
}

/* FORMAT HASH 7bce6023b593a3595f10e4e78c975bf2 */
