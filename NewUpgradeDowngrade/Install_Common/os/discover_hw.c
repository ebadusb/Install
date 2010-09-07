/*
 * $Header$
 * $Log: discover_hw.c $
 * Revision 1.1  2009/03/19 19:37:52Z  ms10234
 * Initial revision
 *
 */

#include <vxworks.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <drv/pci/pciConfigLib.h>
#include <drv/pci/pciIntLib.h>

#include "os/discover_hw.h"


#define	DISPLAY_PCI_DEVICE_CLASS		0x030000

//
// The following strings are used to identify ID/copyright
// strings within ROM data
//
static const char * romIDString[] =
{
    "copyright",
    "Copyright",
    "COPYRIGHT",
    "BIOS",
    NULL        // must be last entry
};

char * findIDString( const char * memPtr, unsigned int memLength, 
							const char * pattern, unsigned int * memStart, 
							unsigned int * stringLength );

unsigned char IsVendor( const char * vendor );


unsigned char checkedHWID = 0;
unsigned char checkedVersalogicType = 0;
unsigned char checkedGraphics = 0;
unsigned char amproStatus = 0;
unsigned char versalogicStatus = 0;
unsigned char vsbc6Status = 0;
unsigned char pythonStatus = 0;
unsigned char ctStatus = 0;
unsigned char geodeLxStatus = 0;
static const int MaxIDStringLength = 256;

unsigned char IsVendor( const char * vendor )
{
    const unsigned int BIOSAddress = 0xf0000;
    const unsigned int BIOSLength = 0x10000;
	 unsigned int  searchStartIdx = 0;
	 unsigned int  stringLength = 0; 
	 const char *  stringInfo = 0;

    const char * romPtr = (const char *)(BIOSAddress);
    int  stringIdx = 0;

	 printf( "in IsVendor\n");

	 while ( romIDString[stringIdx] )
    {
		  printf( "in IsVendor, looking for %s\n", romIDString[stringIdx]);

        stringInfo = findIDString(romPtr, BIOSLength, romIDString[stringIdx], &searchStartIdx, &stringLength);

        while ( stringInfo )
        {
				printf( "stringInfo is true\n");
            if ( stringLength < MaxIDStringLength )
            {
					 printf( "stringLength < MaxIDStringLength\n");
                if ( strstr(stringInfo, vendor ) != NULL )
                {
						  printf( "strstr(stringInfo, vendor ) != NULL\n");
                    return 1;
                }
            }

            stringInfo = findIDString(romPtr, BIOSLength, romIDString[stringIdx], &searchStartIdx, &stringLength);
        }

        stringIdx += 1;
    }
    return 0;
}



static void checkHWID(void)
{

	amproStatus = versalogicStatus = 0;

	printf( "in checkHWID\n");

	if ( IsVendor("Ampro") )
	{
		 printf( "IsVendor true, amproStatus = 1\n");
		amproStatus = 1;
	}
	else
	{
		 printf( "biosIDString false\n");
	
		if ( IsVendor("Versalogic" ) )
		{
			 printf( "biosIDString true, versalogicStatus = 1\n");
			versalogicStatus = 1;
		}
	}
	checkedHWID = 1;
}
/*
static void checkHWID(void)
{
	const unsigned int BIOSAddress = 0xf0000;
	const unsigned int BIOSLength = 0x10000;
	unsigned int  searchStartIdx = 0;
	unsigned int  stringLength = 0; 
	
	const char * romPtr = (const char *)(BIOSAddress);
	char * biosIDString;

	amproStatus = versalogicStatus = 0;

	printf( "in checkHWID\n");

	biosIDString = findIDString(romPtr, BIOSLength, "Ampro", 
											 &searchStartIdx, &stringLength);
	if ( biosIDString )
	{
		 printf( "biosIDString true, amproStatus = 1\n");
		amproStatus = 1;
	}
	else
	{
		 printf( "biosIDString false\n");
		searchStartIdx = 0;
		stringLength = 0; 
	
		romPtr = (const char *)(BIOSAddress);
	
		biosIDString = findIDString(romPtr, BIOSLength, "Versalogic", 
											 &searchStartIdx, &stringLength);
		if ( biosIDString )
		{
			 printf( "biosIDString true, versalogicStatus = 1\n");
			versalogicStatus = 1;
		}
	}
	checkedHWID = 1;
}
*/
unsigned char isAmpro(void)
{
	if ( !checkedHWID )
	{
		 printf( "in isAmpro\n");
		checkHWID();
	}

	return amproStatus;
}

unsigned char isVersalogic(void)
{
	if ( !checkedHWID )
	{
		 printf( "in isVersalogic\n");
		checkHWID();
	}

	return versalogicStatus;
}

static void checkVersalogicType(void)
{
	pythonStatus = vsbc6Status = 0;

	printf( "in isVersalogicType\n");
	if ( isVersalogic() )
	{
		 printf( "is isVersalogic\n");

		 if ( isCT655XXGraphics() )
		{
			vsbc6Status = 1;
		}
		else if ( isGeodeLXGraphics() )
		{
			pythonStatus = 1;
		}
	}
	checkedVersalogicType = 1;
}

unsigned char isVersalogicVSBC6(void)
{
	if ( !checkedVersalogicType )
	{
		 printf( "in isVersalogicVSBC6\n");
		checkVersalogicType();
	}

	return vsbc6Status;
}

unsigned char isVersalogicPython(void)
{
	if ( !checkedVersalogicType )
	{
		 printf( "in isVersalogicPython\n");
		checkVersalogicType();
	}

	return pythonStatus;
}

static void checkGraphics(void)
{
	const unsigned int CT655XXVendorID = 0x00E0102C;
	const unsigned int GeodeLXVendorID = 0x20811022;
	unsigned int currentVendorID;

	printf( "in checkGraphics\n");

	ctStatus = geodeLxStatus = 0;
	currentVendorID = getGraphicsVendorID();
	if ( currentVendorID == CT655XXVendorID )
	{
		 printf( "currentVendorID == CT655XXVendorID\n");
		ctStatus = 1;
	}
	else if ( currentVendorID == GeodeLXVendorID )
	{
		 printf( "currentVendorID == GeodeLXVendorID\n");
		geodeLxStatus = 1;
	}
	checkedGraphics = 1;
}

unsigned char isCT655XXGraphics(void)
{
	if ( !checkedGraphics )
	{
		 printf( "in isCT655XXGraphics\n");
		checkGraphics();
	}

	return ctStatus;
}

unsigned char isGeodeLXGraphics(void)
{
	if ( !checkedGraphics )
	{
		 printf( "in isGeodeLXGraphics\n");
		checkGraphics();
	}

	return geodeLxStatus;
}


unsigned int getGraphicsVendorID(void)
{

	int index = 0;
	STATUS pciStatus;
	int busNo;
	int deviceNo;
	int funcNo;
	UINT32 vendorID=0;

#if CPU != SIMNT

	printf( "in getGraphicsVendorID\n");
	/* Query the PCI bus for display devices */
	do
	{
		pciStatus = pciFindClass( DISPLAY_PCI_DEVICE_CLASS, index, &busNo, &deviceNo, &funcNo );

		if( pciStatus == OK )
		{
			 printf( "pciStatus OK\n");
			/* Is this device recognized by the system? */
			if( pciConfigInLong( busNo, deviceNo, funcNo, PCI_CFG_VENDOR_ID, &vendorID ) == OK )
			{
				 printf( "vendorID = %d\n", vendorID);
				if( vendorID != 0 )
				{
					break;
				}
			}
		}

		index++;

	} while( pciStatus == OK );

#endif

	return vendorID;
}


/*
// Scan memory for specified character pattern.  Used to find ID strings
// in BIOS ROM and expansion ROM memory areas.
*/
char * findIDString(
   const char * memPtr, 			/* start of memory block */
   unsigned int memLength,       /* length of memory block (in bytes) */
   const char * pattern,         /* pattern to search for */
   unsigned int * memStart,      /* starting byte within memory block for search */
   unsigned int * stringLength   /* length of found string */
   )
{
   int patternLength = strlen(pattern);
   const char * resultString = NULL;

	printf( "in findIDString\n");
	printf( "pattern %s\n", pattern);
   while (*memStart < memLength-patternLength &&
          !resultString)
   {
//		 printf( "memPtr[*memStart] %s\n", memPtr[*memStart]);

		 if (memPtr[*memStart] == pattern[0] &&
          memcmp(&memPtr[*memStart], pattern, patternLength) == 0)
      {
			 printf( "found resultString\n");
			 resultString = &memPtr[*memStart];
      }
      else
      {
			 *memStart += 1;
      }
   }

   if (resultString)
   {
      /*
      // Found the specified pattern.  Extend the string to get the printable
      // characters surrounding it.
      */
		 unsigned int   stringStart = *memStart;
      while (stringStart > 0 &&
             isprint((int)(memPtr[stringStart-1])) &&
             (memPtr[stringStart-1] & 0x80) == 0)
      {
         stringStart -= 1;
      }

      *stringLength = patternLength + (*memStart-stringStart);
      while (stringStart+(*stringLength) < memLength &&
             *stringLength < 256 &&
             isprint((int)(memPtr[stringStart+(*stringLength)])) &&
             (memPtr[stringStart+(*stringLength)] & 0x80) == 0)
      {
			 *stringLength += 1;
      }

      resultString = &memPtr[stringStart];
      *memStart = stringStart+(*stringLength);

      if (stringStart+(*stringLength) < memLength &&
          resultString[(*stringLength)] != '\0' &&
          resultString[(*stringLength)] != '\r' &&
          resultString[(*stringLength)] != '\n')
      {
         /*
         // Reject strings that do not end at the end of the ROM, or do not
         // end with \0, \n, or \r.
         */
         resultString = NULL;
      }
      else
      {
         /*
         // Strip trailing spaces from string
         */
			 while (resultString[(*stringLength)] == ' ' && *stringLength > 0)
         {
				 *stringLength -= 1;
         }
      }
   }

   return (char *)resultString;
}

