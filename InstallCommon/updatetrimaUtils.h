/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */
#ifndef UPDATETRIMAUTILS_H
#define UPDATETRIMAUTILS_H

#include <fstream>
#include <vector>
#include "filesort.h"

#define INSTALL_LOG_PATH  MACHINE_PATH "/install"

enum RANGETYPE {V510, V520, V600, V630, V640, V700, END};
enum RANGECOMPARETYPE {MIN, MAX, NOT, FORCE};
enum RANGEVARIABLETYPE {INT, FLOAT};

struct versionStruct
{
/*
   versionStruct() :
      majorRev(0),
      minorRev(0),
      buildNum(0)
   {}
*/
   int majorRev;
   int minorRev;
   int buildNum;
};

struct buildDataStruct
{
/*
   buildDataStruct() :
      buildNum(NULL),
      extractType(0),
      ampro(false),
      versalogic(false),
      python(false),
      ocelot(false),
      adjPASRAS(false),
      calFileType(0),
      adjPostcount(false),
      updateTrapFiles(false),
      setConfigCopy(false),
      rangeType(V510)
   {}
*/
   const char* buildNum;
   int         extractType;
   bool        ampro;
   bool        versalogic;
   bool        python;
   bool        ocelot;
   bool        adjPASRAS;
   int         calFileType;
   bool        adjPostcount;
   bool        updateTrapFiles;
   bool        setConfigCopy;
   RANGETYPE   rangeType;
};


struct rangeStruct
{
/*
   rangeStruct() :
      rangeType(V510),
      section(NULL),
      dataKey(NULL),
      compareType(MIN),
      valType(INT),
      value(NULL)
   {}
*/
   RANGETYPE         rangeType;
   const char*       section;
   const char*       dataKey;
   RANGECOMPARETYPE  compareType;
   RANGEVARIABLETYPE valType;
   const char*       value;
   bool              useLimit;
};


class fourPartVersion
{
public:
   fourPartVersion();
   virtual ~fourPartVersion();
   fourPartVersion(const fourPartVersion& obj);
   fourPartVersion(const char* verString);
   bool operator > (const fourPartVersion& other) const;
   bool operator == (const fourPartVersion& other) const;

   int first;
   int second;
   int third;
   int fourth;
};


// extern ofstream logStream;

class updatetrimaUtils
{
public:
   // Default constructor
   updatetrimaUtils();

   // Default destructor
   virtual ~updatetrimaUtils();

private:
   // Copy constructor
   updatetrimaUtils(const updatetrimaUtils& obj);


public:

   static int copyFileContiguous (const char* from, const char* to);

   static int unzipFile (const char* from, const char* to);

   static int zipFile (const char* from, const char* to);

//   bool parseRevision(const char *revString, TrimaVersion &parsedVersion);

   static FileCallBackStatus update_clean_file (const char* fullPathName);

   static FileCallBackStatus update_file_set_rdwrite (const char* fullPathName);

   static FileCallBackStatus update_file_set_rdonly (const char* fullPathName);

   static bool parseRevision (const char* revString, int& curMajorRev, int& curMinorRev, int& curBuild);

   static void logFileHeader (std::string& hdrStr);

   static bool development_install;

   static bool getBuildInfo (versionStruct& versionInfo, int& buildRef);

   static buildDataStruct& getBuildData (int buildRef);
   static rangeStruct&     getNextRangeData (bool reset = false);

//////////////////////////////////////////////////
// The logger calls are all depricated - they are being replaced by the installLogStream
// They will be removed if/when I get the time to go through the code and switch
// all of the logging to use the stream class.
// The logger calls now all use the stream class behind the scenes to do the logging
   static bool loggingEnabled;
   static bool logToScreen;
   static void logger (const char* stuff);
   static void logger (int stuff);
   static void logger (float stuff);
   static void logger (unsigned long stuff);
   static void logger (const char* stuff1, const char* stuff2);
   static void logger (const char* stuff1, int stuff2);
   static void logger (const char* stuff1, const char* stuff2, const char* stuff3);
   static void logger (const char* stuff1, const char* stuff2, const char* stuff3, const char* stuff4);
   static bool initLogging ();
   static void closelogger ();

protected:

   static FILE* logFile;

};


///////////////////////////////////////////////////////////////////
// file log stream class
class installLogStream
{
public:
   installLogStream();
   installLogStream(bool logToScrn);
   virtual ~installLogStream();
   bool                       open (const char* filename);
   bool                       is_open ();
   void                       close ();
   installLogStream& operator << (const char* stuff);
   installLogStream& operator << (const std::string& stuff);
   installLogStream& operator << (const long stuff);
   installLogStream& operator << (const int stuff);
   installLogStream& operator << (const unsigned int stuff);
   installLogStream& operator << (const float stuff);
   installLogStream& operator << (const double stuff);
//    installLogStream& operator << (ostream& stuff);

   void LogToScreen (bool onOff) {logToScreen = onOff; }

private:
   installLogStream(const installLogStream& obj);

public:

protected:
   bool                   logToScreen;
   std::vector<ofstream*> streamVect;
};

#endif // UPDATETRIMAUTILS_H

/* FORMAT HASH 57053f4d83a7aac5e1cfff0a5215ae2d */
