/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * Save the latest log files in preparation for a hard disk
 *  file system change.
 *
 * $Header$
 * $Log$
 *
 */

#include <dirent.h>
#include <fcntl.h>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/disk.h>

#define BLOCK_SIZE     ( 512 )              // bytes
#define MIN_DISK_SPACE ( 10*1024*1024 )     // bytes
struct gtchar
{
  bool operator()(char* f1, char* f2) const
  {
     if ( f2 )
        if ( f1 )
           return strcmp(f1, f2) > 0;
        else
           return 0;
     else
        return 1;
  }
};

set< char*, gtchar > rundataFiles;

void readAndSortLogFiles()
{
   DIR *d = opendir( "/d/run_data" );
   dirent *lf; 
   while ( ( lf = readdir( d ) ) != NULL ) 
   {
      if (    lf->d_name[0] == '1' 
           && lf->d_name[1] == 'T' )
      {
         //
         // Save the entry ...
         char *lfname = new char[ strlen( lf->d_name ) + 1 ];
         strcpy( lfname, lf->d_name );
         rundataFiles.insert( lfname );
      }
   }
}

long int copyFile( int toFD,            // File to write into 
                   int fromFD,          // File to copy from
                   long int *bytesUsed, // Running total of bytes
                   long int bytesFree, 
                   int blockSize 
                 )
{
   long int bytesRead;
   long int bytesCopied;
   long int filelength=0;

   char *buffer = new char[blockSize];
	while ( (bytesRead = read(fromFD, buffer, blockSize)) > 0 )
   {
		bytesCopied = write( toFD, buffer, bytesRead);
      filelength += bytesCopied;
      *bytesUsed += bytesCopied;

      if ( *bytesUsed > bytesFree )
         break;
   }
   delete [] buffer;
   
   return filelength;
}

long archiveFile( int fd, int fromFD, const char *fname, long int availspace )
{
   if ( !fd || !fromFD )
      return 0;

   long int currpos;
   long int fsizepos;
   long int bytesRead;
   long int bytesCopied=0;
   long int filelength;
   long int totalBytes=0;

   //
   // Write length of filename and then the filename
   bytesRead = strlen( fname );
   totalBytes += write( fd, &bytesRead, 4 );
   totalBytes += write( fd, fname, strlen( fname )-1 );

   //
   // Save the file time 
   struct stat fs;
   fstat( fromFD, &fs );
   totalBytes += write( fd, &( fs.st_mtime ), 4 );
   
   //
   // Get the file position to save the size after we're done writing ...
   bytesRead = 0;
   fsizepos = tell( fd );
   totalBytes += write( fd, &bytesRead, 4 );

   filelength = copyFile( fd, fromFD, &totalBytes, availspace, 16*BLOCK_SIZE );

   currpos = tell( fd );
   lseek( fd, fsizepos, SEEK_SET );
   write( fd, &filelength, 4 );
   lseek( fd, currpos, SEEK_SET );
   
   return totalBytes;
}

void saveLogFiles( int fd )
{
   cout << "Sorting log files ..." << endl;
   readAndSortLogFiles();
   cout << "Sorted log files:  size->" << rundataFiles.size() << endl; 

   long int free, total;
   disk_space( fd, &free, &total );

   cout << "Saving log files" << endl; 
   //
   // Write off log files till we reach our end of 400MB ...

   int bytesCopied = 0;
   int baseLen = strlen( "1TXXXXX_YYYYMMDD_NNN" );
   int rdLen = strlen( "/d/run_data" );
   int lLen = strlen( "/d/log" );
   char *basefile        = new char[ baseLen             + 1 ];
   char *rundatafilebase = new char[ baseLen + 7         + 1 ];
   char *rundatafile     = new char[ baseLen + 7 + rdLen + 1 ];
   char *ctrfilebase     = new char[ baseLen + 7         + 1 ];
   char *ctrfile         = new char[ baseLen + 7 + lLen  + 1 ];
   char *strfilebase     = new char[ baseLen + 7         + 1 ];
   char *strfile         = new char[ baseLen + 7 + lLen  + 1 ];

   //
   // Iterate over our sorted list ...
   set< char*, gtchar >::iterator liter;
   int fromFD;
   bool zippingOn;
   for ( liter  = rundataFiles.begin() ;
         liter != rundataFiles.end() && ( free*BLOCK_SIZE - bytesCopied ) > MIN_DISK_SPACE ; 
         liter++ )
   {
      char *lfname = (*liter);
      zippingOn = ( strpbrk( lfname, "z" ) != NULL );
      strncpy( basefile, lfname, baseLen ); basefile[baseLen] = '\0';
      rundataFiles.erase( liter );
      delete [] lfname;

      strcpy( rundatafile, "/d/run_data/" );
      strcpy( rundatafilebase, basefile );
      strcat( rundatafile, basefile );
      strcat( rundatafilebase, ".csv" );
      strcat( rundatafile, ".csv" );
   
      strcpy( ctrfile, "/d/log/" );
      strcpy( ctrfilebase, basefile );
      strcat( ctrfile, basefile );
      strcpy( strfile, ctrfile );
      strcpy( strfilebase, ctrfilebase );
      strcat( ctrfilebase, ".ctr" );
      strcat( ctrfile, ".ctr" );
      strcat( strfilebase, ".str" );
      strcat( strfile, ".str" );

      if ( zippingOn )
      {
         strcat( rundatafile, ".gz" );
         strcat( rundatafilebase, ".gz" );
         strcat( ctrfile, ".gz" );
         strcat( ctrfilebase, ".gz" );
         strcat( strfile, ".gz" );
         strcat( strfilebase, ".gz" );
      }
      
      //
      // Open each file and save it in the archive location ...
      fromFD = open( rundatafile, O_RDONLY, 0644 ); 
      bytesCopied += archiveFile( fd, fromFD, rundatafilebase, free*BLOCK_SIZE - bytesCopied - MIN_DISK_SPACE );
      close( fromFD );
      fromFD = open( ctrfile, O_RDONLY, 0644 ); 
      bytesCopied += archiveFile( fd, fromFD, ctrfilebase, free*BLOCK_SIZE - bytesCopied - MIN_DISK_SPACE );
      close( fromFD );
      fromFD = open( strfile, O_RDONLY, 0644 ); 
      bytesCopied += archiveFile( fd, fromFD, strfilebase, free*BLOCK_SIZE - bytesCopied - MIN_DISK_SPACE );
      close( fromFD );
      cout << ".";
   }

   delete [] basefile;
   delete [] rundatafilebase;
   delete [] rundatafile;
   delete [] ctrfilebase;
   delete [] ctrfile;
   delete [] strfilebase;
   delete [] strfile;

   cout << "\nDone saving log files" << endl;
}

void saveMachineFiles( int fd )
{
   long int free, total;
   disk_space( fd, &free, &total );

   //
   // Create a tar file from the machine directory ...
   system( "tar -cvf /d/update/machine.tar /d/machine" );

   cout << "Saving machine.tar" << endl;
   //
   // Save off the machine specific data ...
   int fromFD = open( "/d/update/machine.tar", O_RDONLY, 0644 ); 
   archiveFile( fd, fromFD, "machine.tar", free*BLOCK_SIZE );
   close( fromFD );
   cout << "Done saving machine.tar" << endl;
}

void saveUpdateFiles( int fd )
{
   long int free, total;
   disk_space( fd, &free, &total );

   cout << "Saving updatetrima" << endl;
   //
   // Save off the machine specific data ...
   int fromFD = open( "/d/update/updatetrima", O_RDONLY, 0644 ); 
   archiveFile( fd, fromFD, "updatetrima", free*BLOCK_SIZE );
   close( fromFD );
   cout << "Done saving updatetrima" << endl;
}

int main( int argc, char **argv )
{
   if ( argc != 2 )
   {
      cout << "Must specify a destination file name" << endl;
      return -1;
   }

   char *destFile = argv[1];
   int fd = open( destFile, O_RDWR | O_CREAT, 0644);
   if ( fd == -1 )
   {
      cout << "Failed to open destination file" << endl;
      return 1;
   }

   saveMachineFiles( fd );
   saveUpdateFiles( fd );
   saveLogFiles( fd );

   int filler = 0;
   write( fd, &filler, 4 ); // End with zeros ...
   write( fd, &filler, 4 );
   write( fd, &filler, 4 );
   write( fd, &filler, 4 );
   
   long int free, total;
   disk_space( fd, &free, &total );
   cout << "Archive step finished  :  remaining space on disk->" << free/2 << "Kb" << endl;
   close( fd );

   return 0;
}
