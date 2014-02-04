/*
 * Copyright (C) 2013 TerumoBCT, Inc.  All rights reserved.
 *
 * This is a clone of cassette_dat.h from trima_common with stuff that the
 * Install script can't use stripped out and renamed to prevent collisions
 *
 */


#ifndef _UPDATE_CASSETTE_DAT_H_
#define _UPDATE_CASSETTE_DAT_H_

#include <vector>
#include <string>

#include "filehelp.h"                 // Get CFileHelper
#include "updatecassette.h"           // Gets the cassette container

// FOR TRAVERSING THE CASSETTE LIST OUTSIDE THIS CLASS
typedef vector<UpdateCatalogNumber*> UPDATE_CASSETTE_VECTOR_TYPE;
typedef UPDATE_CASSETTE_VECTOR_TYPE::iterator UPDATE_CASSETTE_VECTOR_ITERATOR;

typedef map<string, UpdateCatalogNumber*> UPDATE_CASSETTE_MAP_TYPE;
typedef UPDATE_CASSETTE_MAP_TYPE::iterator UPDATE_CASSETTE_MAP_ITERATOR;

class UpdateCassetteDat
{
public:
   UpdateCassetteDat();
   virtual ~UpdateCassetteDat();

protected:
   static bool checkFile (const char* datFilePath, const char* crcFilePath, const char* datFileBkPath, const char* crcFileBkPath);
   static bool fixFile (CFileHelper& goodFile, const char* fname, const char* crcfname);
};


class MasterUpdateCassetteDat
   : public UpdateCassetteDat
{
public:
   MasterUpdateCassetteDat();
   virtual ~MasterUpdateCassetteDat();

   // Exported iterator methods
   static UPDATE_CASSETTE_MAP_ITERATOR find (const char* cassetteIdString);

private:
public:
   // READ THE MASTER CASSETTE FILE
   static void read ();
   static void clear ();

   static UPDATE_CASSETTE_MAP_ITERATOR begin ();
   static UPDATE_CASSETTE_MAP_ITERATOR end ();

   static bool fileOK (void) {return _fileOK; }

protected:
   static bool _fileOK;

   static char _fileVersion[80];
};


class AdminUpdateCassetteDat
   : public UpdateCassetteDat
{
public:
   AdminUpdateCassetteDat();
   virtual ~AdminUpdateCassetteDat();

   // READ THE ADMIN CASSETTE FILE
   static void read ();

   static bool fileOK (void){return _fileOK; }

   // Exported iterator methods
   static UPDATE_CASSETTE_VECTOR_ITERATOR begin ();
   static UPDATE_CASSETTE_VECTOR_ITERATOR end ();

   static void erase (UPDATE_CASSETTE_VECTOR_ITERATOR it);

   static unsigned int size (void);
   static void         clear (void);

   static void addCassette (const char* refNumber, unsigned int tubingSetCode, const char* barcode);

   // UPDATE THE CASSETTE FILE
   static bool updateCassetteFile ();
   static void updateIfChanged ();

   static int file_crcgen32 (const char* filename, unsigned long* pcrc);

protected:
   // ADD THE BEGINNING-FILE DECORATIONS TO THE ADMIN FILE
   static void decorateFile (ostream& o);

protected:
   static bool _fileOK;

   static char _fileVersion[80];
   static bool _fileHasChanged;
};



#endif

/* FORMAT HASH 44b10509771551c93f47b0048891c395 */
