//
// $Header: K:/BCT_Development/Install/Trima_v5.1/ampro_bios_update/rcs/ampro_cbios_update.cpp 1.1 2003/11/19 18:15:09Z jl11312 Exp jl11312 $
// $Log: ampro_cbios_update.cpp $
// Revision 1.1  2003/11/05 18:02:03Z  jl11312
// Initial revision
//
// Ampro control board (LB486) BIOS update program
//

#include <conio.h>
#include <i86.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/seginfo.h>

enum { BaseBiosSize = 0x10000 };
enum { BaseBiosAddress = 0xf0000 };

enum { FlashSize = 0x20000 };
enum { FlashAddress = 0xd0000 };

#include "control_image.cpp"
#include "ampro_support.cpp"
#include "flash_support.cpp"

void pausePrompt(const char * s)
{
	if ( s != NULL )
	{
		printf("%s and press <Enter> to continue", s);
	}
	else
	{
		printf("Press <Enter> to continue");
	}

	fflush(stdout);
	do
		;
	while ( getch() != '\n' );
	printf("\n");
}

void retryPrompt(void)
{
	printf("Press <Enter> to retry");
	fflush(stdout);
	do
		;
	while ( getch() != '\n' );
	printf("\n");
}

int check12VoltSwitch(volatile unsigned char __far * const flashPtr)
{
	//
	// If 12V power is not enabled, the chip ID should
	// simply return the first two bytes of the BIOS image.
	// Otherwise, it should return the manufacturer and part
	// ID as a 16 bit word.
	//
	unsigned short id1 = readChipId(flashPtr);
	enable12Volt();
	unsigned short id2 = readChipId(flashPtr);
	disable12Volt();
	unsigned short id3 = readChipId(flashPtr);

	return ( id1 == id3 && id1 != id2 );
}

void main(void)
{
	char * romImage = NULL;

	//
	// Get a copy of the existing ROM BIOS
	//
	pausePrompt("Check that BIOS programming switch is disabled");
	int	romSegment = (int)qnx_segment_overlay(BaseBiosAddress, BaseBiosSize);
	if ( romSegment != -1 )
	{
		unsigned char __far * romPtr = (unsigned char __far *)MK_FP(romSegment, 0);
		romImage = new char[BaseBiosSize];
		_fmemcpy(romImage, romPtr, BaseBiosSize);
	}
	else
	{
		fprintf(stderr, "\nFailed to map BIOS memory area\n");
		exit(1);
	}

	//
	// Enabling the programming switch should do two things:
	//		1) Remap the flash memory to 0xd0000
	//		2) Enable the 12V programming voltage
	//
	// (1) is checked by comparing the memory at 0xd0000 with the ROM
	// BIOS image captured previously.  (2) is checked by trying the
	// chip ID command (which only works if the programming voltage
	// is applied to the flash chip.
	//
	pausePrompt("Enable BIOS programming switch");
	int	flashSegment = (int)qnx_segment_overlay(FlashAddress, FlashSize);
	unsigned char __far * newRomPtr = (unsigned char __far *)MK_FP(flashSegment, 0);
	if ( flashSegment != -1 )
	{
		while ( _fmemcmp(romImage, newRomPtr, BaseBiosSize) != 0 )
		{
			printf("\nFlash memory has not been remapped.\n");
			pausePrompt("Check BIOS programming switch and connections");
		}
	}
	else
	{
		fprintf(stderr, "Failed to map flash memory area\n");
		exit(1);
	}

  	volatile unsigned char __far * flashPtr = (unsigned char __far *)MK_FP(flashSegment, 0);
	resetFlash(flashPtr);
	while ( !check12VoltSwitch(flashPtr) )
	{
		printf("\nFailed to enable programming voltage.\n");
		pausePrompt("Check BIOS programming switch and connections");
	}

	unsigned char * temp = new unsigned char[FlashSize];
	_fmemcpy(temp, newRomPtr, FlashSize);
	FILE * fp = fopen("old_image.dat", "w");
	fwrite(&temp[0x10000], 1, 0x10000, fp);
	fwrite(temp, 1, 0x10000, fp);
	fclose(fp);

	printf("\nStarting flash programming sequence.  Do not disconnect power until\n");
	printf("instructed to do so.  Partially completing the flash programming sequence\n");
	printf("may leave the system unusable and require replacement of the CPU board.\n\n");
	pausePrompt(NULL);
	printf("\n");

	const char * result;
	do
	{
		printf("Erasing flash ...");
		fflush(stdout);

		enable12Volt();
		result = eraseFlash(flashPtr);

		if ( result )
		{
			disable12Volt();

			printf(" failed\n");
			printf("%s\n", result);
			retryPrompt();
			printf("\n");

			enable12Volt();
		}
		else
		{
			printf(" done\n");
			disable12Volt();
		}

	} while ( result != NULL );

	printf("Writing BIOS update ...");
	fflush(stdout);

	enable12Volt();

	int	idx = 0;
	while ( idx<FlashSize )
	{
		result = programFlash(flashPtr+idx, biosUpdateImage[idx]);

		if ( result )
		{
			disable12Volt();

			printf(" failed\n");
			printf("%s\n", result);
			retryPrompt();

			printf("\nWriting BIOS update ...");
			fflush(stdout);

			enable12Volt();
		}
		else
		{
			idx += 1;
		}
	}

	disable12Volt();
	printf(" done\n");

	_fmemcpy(temp, newRomPtr, FlashSize);
	fp = fopen("new_image.dat", "w");
	fwrite(&temp[0x10000], 1, 0x10000, fp);
	fwrite(temp, 1, 0x10000, fp);
	fclose(fp);

	pausePrompt("Disable BIOS programming switch");
	while ( check12VoltSwitch(flashPtr) )
	{
		printf("\nDisable check failed\n");
		pausePrompt("Check BIOS programming switch and connections");
	}
}
