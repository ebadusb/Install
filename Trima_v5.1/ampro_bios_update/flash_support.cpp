//
// $Header: K:/BCT_Development/AmproBiosUpdate/rcs/flash_support.cpp 1.2 2003/11/05 18:01:11Z jl11312 Exp $
// $Log: flash_support.cpp $
// Revision 1.2  2003/11/05 18:01:11Z  jl11312
// - updated to support safety BIOS update
// Revision 1.1  2003/10/08 16:40:57Z  jl11312
// Initial revision
//
// These command sequences are described in the Intel datasheet
// for the 28F010 bulk erase flash memory.
//

#include <time.h>

//
// Timings are slightly longer than specified (approximately 2.7 usec per
// inp call).  The flash chips have internal timers to limit timed operations,
// so these delays are only to insure that the internal timers have completed.
//
#define WAIT_6USEC	inp(0x80); inp(0x80); inp(0x80);
#define WAIT_10USEC	inp(0x80); inp(0x80); inp(0x80); inp(0x80);

//
// Command values for flash
//
enum { ReadCmd = 0x00 };
enum { ReadChipIDCmd = 0x90 };
enum { EraseCmd = 0x20 };
enum { EraseVerifyCmd = 0xa0 };
enum { ProgramCmd = 0x40 };
enum { ProgramVerifyCmd = 0xc0 };
enum { ResetCmd = 0xff };

//
// Error message buffer
//
static char errorMsg[128];

//
// Reset flash chip to a known state.
//
void resetFlash(volatile unsigned char __far * const flashPtr)
{
	*flashPtr = ResetCmd;
	*flashPtr = ResetCmd;
	delay(1);
	*flashPtr = ReadCmd;
}

//
// Read manufacturer and chip ID value
//
unsigned short readChipId(volatile unsigned char __far * const flashPtr)
{
	unsigned short id;

	*flashPtr = ReadChipIDCmd;
	id = *flashPtr;
	id |= (*(flashPtr+1)) << 8;
	*flashPtr = ReadCmd;

	return id;
}

//
// Program single flash byte
//
const char * programFlash(volatile unsigned char __far * const bytePtr, unsigned char value)
{
	int retry = 25;
	int done = 0; 

	while ( retry>0 && !done )
	{
		_disable();
		*bytePtr = ProgramCmd;
		*bytePtr = value;
		WAIT_10USEC;

		*bytePtr = ProgramVerifyCmd;
		WAIT_6USEC;
		done = ( value == *bytePtr );
		_enable();

		retry -= 1;
	}

	*bytePtr = ReadCmd;
	if ( !done )
	{
		sprintf(errorMsg, "programFlash: failed to program (idx=%05lx, value=%02x, program=%02x)", FP_OFF(bytePtr), (unsigned int)*bytePtr, (unsigned int)value);
		return errorMsg;
	}

	return NULL;
}

//
// Bulk erase flash
//
const char * eraseFlash(volatile unsigned char __far * const flashPtr)
{
	//
	// Must program all bytes to 0 before starting erase
	//
	unsigned long	idx;
	for ( idx=0; idx<FlashSize; idx+=1 )
	{
	   if ( *(flashPtr+idx) != 0 && programFlash(flashPtr+idx, 0) != NULL )
		{
			sprintf(errorMsg, "eraseFlash: failed to zero (idx=%05lx, value=%02x)", idx, (unsigned int)*(flashPtr+idx));
			return errorMsg;
		}
	}

	//
	// Erase flash
	//
	int retry = 1000;
	int done = 0;
 
	while ( retry>0 && !done )
	{
		*flashPtr = EraseCmd;
		*flashPtr = EraseCmd;
		delay(10);

		done = 1;
		for ( idx=0; idx<FlashSize && done; idx+=1 )
		{
			_disable();
			*(flashPtr+idx) = EraseVerifyCmd;
			WAIT_6USEC;
			
			done = ( *(flashPtr+idx) == 0xff );
			_enable();
		}

		retry -= 1;
	}

	*flashPtr = ReadCmd;
	if ( !done )
	{
		sprintf(errorMsg, "eraseFlash: failed to erase (idx=%05lx, value=%02x)", idx, (unsigned int)*(flashPtr+idx));
		return errorMsg;
	}

	return NULL;
}

