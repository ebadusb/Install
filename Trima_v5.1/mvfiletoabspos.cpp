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

long int copyFile( int fromFD,       // File to copy from 
                   int toFD          // File to write into
                 )
{
   long int bytesRead;
   long int bytesCopied;
   long int filelength=0;

   char *buffer = new char[BLOCK_SIZE];
	while ( (bytesRead = read(fromFD, buffer, BLOCK_SIZE)) > 0 )
   {
		bytesCopied = write( toFD, buffer, bytesRead);
      filelength += bytesCopied;
   }
   delete [] buffer;
   
   return filelength;
}

long mvFileToAbsPos( int fromFD, int toFD, long int absPos )
{
   if ( !toFD || !fromFD )
      return 0;

   long int currpos;
   long int fsizepos;
   long int bytesRead;
   long int filelength;
   long int totalBytes=0;

   //
   // Go to the requested position ...
   lseek( toFD, absPos, SEEK_SET );

   //
   // Get the file position to save the size after we're done writing ...
   bytesRead = 0;
   fsizepos = tell( toFD );
   write( toFD, &bytesRead, 4 );

   filelength = copyFile( fromFD, toFD );

   currpos = tell( toFD );
   lseek( toFD, fsizepos, SEEK_SET );
   write( toFD, &filelength, 4 );
   lseek( toFD, currpos, SEEK_SET );
   
   return filelength;
}

int main( int argc, char **argv )
{
   if ( argc < 4 )
   {
      cout << "Usage: mvfiletoabspos origfile destfile position" << endl;
      cout << " origfile - regular file" << endl;
      cout << " newfile  - destination regular file or device" << endl;
      cout << " position - absolute position within the destination file in bytes" << endl;
      return -1;
   }

   char *fromFile = argv[1];
   char *toFile   = argv[2];
   long int pos   = atol( argv[3] );

   int fd = open( "/saved_files", O_RDONLY, 0644);
   int raw = open( "/dev/hd0", O_RDWR , 0644);

   if ( fd == -1 || raw == -1 )
   {
      cout << "Unable to open files (" << fd << "," << raw << ")" << endl;
      return 1;
   }

   if ( pos < 0 )
   {
      cout << "Unable to move to abspos " << pos << endl;
      return 2;
   }

   if ( mvFileToAbsPos( fd, raw, pos ) <= 0 )
   {
      cout << "Move failed" << endl;
      return 3;
   }

   close( fd );
   close( raw );

   return 0;
}
