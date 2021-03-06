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

#include "discover_hw.h"


#define DISPLAY_PCI_DEVICE_CLASS        0x030000

char* findIDString (const char* memPtr, unsigned int memLength,
                    const char* pattern, unsigned int* memStart,
                    unsigned int* stringLength);

unsigned char checkedHWID           = 0;
unsigned char checkedVersalogicType = 0;
unsigned char checkedGraphics       = 0;
unsigned char amproStatus           = 0;
unsigned char versalogicStatus      = 0;
unsigned char vsbc6Status           = 0;
unsigned char pythonStatus          = 0;
unsigned char foxStatus             = 0;
unsigned char rdcStatus             = 0;
unsigned char ctStatus              = 0;
unsigned char geodeLxStatus         = 0;
const char*   biosIDString          = NULL;


static void checkHWID (void)
{
   const unsigned int BIOSAddress    = 0xf0000;
   const unsigned int BIOSLength     = 0x10000;
   unsigned int       searchStartIdx = 0;
   unsigned int       stringLength   = 0;

   const char* romPtr = (const char*)(BIOSAddress);

   amproStatus = versalogicStatus = 0;

   biosIDString = findIDString(romPtr, BIOSLength, "Ampro",
                               &searchStartIdx, &stringLength);
   if ( biosIDString )
   {
      amproStatus = 1;
   }
   else
   {
      searchStartIdx = 0;
      stringLength   = 0;

      romPtr = (const char*)(BIOSAddress);

      biosIDString = findIDString(romPtr, BIOSLength, "VersaLogic",
                                  &searchStartIdx, &stringLength);

      if ( NULL == biosIDString )
      {
         /* Lastly, try for E-Box, another Versalogic variant */
         searchStartIdx = 0;
         stringLength   = 0;
         biosIDString   = findIDString(romPtr, BIOSLength, "Vortex86DX2",
                                       &searchStartIdx, &stringLength);
      }

      if ( biosIDString )
      {
         versalogicStatus = 1;
      }
   }
   checkedHWID = 1;

   // printf("biosIDString: %s\n", (biosIDString ? biosIDString : "<NotFound>"));
}

unsigned char isAmpro (void)
{
#if CPU!=SIMNT
   if ( !checkedHWID )
   {
      checkHWID();
   }

   return amproStatus;
#else
   if ( fopen("Install/machine/update/control_ampro", "r") ) 
   {
      return 1;
   }
   else
   {
      return 0;
   }
#endif
}

unsigned char isVersalogic (void)
{
   if ( !checkedHWID )
   {
      checkHWID();
   }

   return versalogicStatus;
}

static void checkVersalogicType (void)
{
   foxStatus = pythonStatus = vsbc6Status = 0;

   if ( isVersalogic() )
   {
      if ( isCT655XXGraphics() )
      {
         vsbc6Status = 1;
      }
      else if ( isGeodeLXGraphics() )
      {
         pythonStatus = 1;
      }
      else if ( isRDCGraphics() )
      {
         foxStatus = 1;
      }
   }
   checkedVersalogicType = 1;
}

unsigned char isVersalogicVSBC6 (void)
{
#if CPU!=SIMNT
   if ( !checkedVersalogicType )
   {
      checkVersalogicType();
   }

   return vsbc6Status;
#else
   if ( fopen("Install/machine/update/control_versalogic", "r") ) 
   {
      return 1;
   }
   else
   {
      return 0;
   }
#endif
}

unsigned char isVersalogicPython (void)
{
#if CPU!=SIMNT
   if ( !checkedVersalogicType )
   {
      checkVersalogicType();
   }

   return pythonStatus;
#else
   if ( fopen("Install/machine/update/control_python", "r") ) 
   {
      return 1;
   }
   else
   {
      return 0;
   }
#endif

}

unsigned char isVersalogicFox (void)
{
   if ( !checkedVersalogicType )
   {
      checkVersalogicType();
   }

   return foxStatus;
}

static void checkGraphics (void)
{
   const unsigned int CT655XXVendorID = 0x00E0102C;
   const unsigned int GeodeLXVendorID = 0x20811022;
   const unsigned int RDCVendorID     = 0x201217F3;
   unsigned int       currentVendorID;

   ctStatus        = geodeLxStatus = rdcStatus = 0;
   currentVendorID = getGraphicsVendorID();
   if ( currentVendorID == CT655XXVendorID )
   {
      ctStatus = 1;
   }
   else if ( currentVendorID == GeodeLXVendorID )
   {
      geodeLxStatus = 1;
   }
   else if ( currentVendorID == RDCVendorID )
   {
      rdcStatus = 1;
   }
   checkedGraphics = 1;

   // printf("currentVendorID == %d\n", currentVendorID);
}

unsigned char isCT655XXGraphics (void)
{
   if ( !checkedGraphics )
   {
      checkGraphics();
   }

   return ctStatus;
}

unsigned char isGeodeLXGraphics (void)
{
   if ( !checkedGraphics )
   {
      checkGraphics();
   }

   return geodeLxStatus;
}

unsigned char isRDCGraphics (void)
{
   if ( !checkedGraphics )
   {
      checkGraphics();
   }

   return rdcStatus;
}

unsigned int getGraphicsVendorID (void)
{

   int    index = 0;
   STATUS pciStatus;
   int    busNo;
   int    deviceNo;
   int    funcNo;
   UINT32 vendorID = 0;

#if CPU != SIMNT

   /* Query the PCI bus for display devices */
   do
   {
      pciStatus = pciFindClass(DISPLAY_PCI_DEVICE_CLASS, index, &busNo, &deviceNo, &funcNo);

      if ( pciStatus == OK )
      {
         /* Is this device recognized by the system? */
         if ( pciConfigInLong(busNo, deviceNo, funcNo, PCI_CFG_VENDOR_ID, &vendorID) == OK )
         {
            if ( vendorID != 0 )
            {
               // printf("vendorID = %d\n", vendorID);
               break;
            }
         }
      }

      index++;

   } while ( pciStatus == OK );

#endif

   return vendorID;
}


/*
// Scan memory for specified character pattern.  Used to find ID strings
// in BIOS ROM and expansion ROM memory areas.
*/
char* findIDString (const char* memPtr,        /* start of memory block */
                    unsigned int memLength,    /* length of memory block (in bytes) */
                    const char* pattern,       /* pattern to search for */
                    unsigned int* memStart,    /* starting byte within memory block for search */
                    unsigned int* stringLength /* length of found string */
                    )
{
   int         patternLength = strlen(pattern);
   const char* resultString  = NULL;

   while (*memStart < memLength - patternLength &&
          !resultString)
   {
      if (memPtr[*memStart] == pattern[0] &&
          memcmp(&memPtr[*memStart], pattern, patternLength) == 0)
      {
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
      unsigned int stringStart = *memStart;
      while (stringStart > 0 &&
             isprint((int)(memPtr[stringStart - 1])) &&
             (memPtr[stringStart - 1] & 0x80) == 0)
      {
         stringStart -= 1;
      }

      *stringLength = patternLength + (*memStart - stringStart);
      while (stringStart + (*stringLength) < memLength &&
             *stringLength < 256 &&
             isprint((int)(memPtr[stringStart + (*stringLength)])) &&
             (memPtr[stringStart + (*stringLength)] & 0x80) == 0)
      {
         *stringLength += 1;
      }

      resultString = &memPtr[stringStart];
      *memStart    = stringStart + (*stringLength);

      if (stringStart + (*stringLength) < memLength &&
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

   return (char*)resultString;
}

/* FORMAT HASH cb1388a8cdfca65c2406514596bfa60f */
