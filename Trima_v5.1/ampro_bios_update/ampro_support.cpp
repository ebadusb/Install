//
// $Header: K:/BCT_Development/Install/Trima_v5.1/ampro_bios_update/rcs/ampro_support.cpp 1.1 2003/11/19 18:15:08Z jl11312 Exp jl11312 $
// $Log: ampro_support.cpp $
// Revision 1.2  2003/11/05 18:01:10Z  jl11312
// - updated to support safety BIOS update
// Revision 1.1  2003/10/08 16:40:57Z  jl11312
// Initial revision
//
// These I/O sequences were obtained by tracing through the interrupt
// calls provided in the Ampro BIOS for enabling and disabling the
// 12V programming voltage to the flash chip.
//

#ifdef SAFETY

//
// Much of the code for the safety processor relies on the ACC2087 controller
// chip.  Documentation for that chip is included in the ACC directory for this
// project.
//

void mapBiosImage(void)
{
	unsigned char	romMask, romSetup;

	// Make sure ROM isn't mapped until after we handle initialization
	_disable();
	outp(0xf2, 0x28);
	inp(0x80);
	romMask = inp(0xf3);
	romMask &= 0x3f;
	inp(0x80);
	outp(0xf3, romMask);
	_enable();

	// Make sure we have default ROM BIOS settings
	_disable();
	outp(0xf2, 0x00);
	inp(0x80);
	romSetup = inp(0xf3);
	romSetup &= 0x9f;
	inp(0x80);
	outp(0xf3, romSetup);
	_enable();

	// Setup output latch for AMPRO chip select and 12Volt enable logic
	_disable();
	outp(0xf2, 0x24);
	inp(0x80);
	outp(0xf3, 0x82);
	inp(0x80);
	outp(0x80f3, 0x00);
	inp(0x80);
	_enable();

	// Map ROM to d0000
	_disable();
	outp(0xf2, 0x28);
	inp(0x80);
	romMask = inp(0xf3);
	romMask |= 0xc0;
	inp(0x80);
	outp(0xf3, romMask);
	_enable();
}

void enable12Volt(void)
{
	_disable();
	outp(0xf2, 0x24);
	inp(0x80);
	outp(0xf3, 0x02);
	_enable();	

	delay(10);
}

void disable12Volt(void)
{
	_disable();
	outp(0xf2, 0x24);
	inp(0x80);
	outp(0xf3, 0x82);
	_enable();

	delay(10);
}

#else /* ifdef SAFETY */

static void enable2F2Port(void)
{
	_disable();
	outp(0x1f8, 0x55);
	outp(0x1f8, 0x55);
	outp(0x1f8, 0x01);

	unsigned char val = inp(0x1f9);
	val |= 0x0f;
	outp(0x1f9, val);
	outp(0x1f8, 0xaa);
	_enable();
}

static void disable2F2Port(void)
{
	_disable();
	outp(0x1f8, 0x55);
	outp(0x1f8, 0x55);
	outp(0x1f8, 0x01);

	unsigned char val = inp(0x1f9);
	val &= 0xfc;
	outp(0x1f9, val);
	outp(0x1f8, 0xaa);
	_enable();
}

void enable12Volt(void)
{
	enable2F2Port();

	_disable();
	unsigned char	val = inp(0x2f2);
	val = (val & 0xfd) | 0x02;
	outp(0x2f2, val);
	_enable();

	disable2F2Port();
	delay(10);
}

void disable12Volt(void)
{
	enable2F2Port();

	_disable();
	unsigned char	val = inp(0x2f2);
	val = val & 0xfd;
	outp(0x2f2, val);
	_enable();

	disable2F2Port();
	delay(10);
}

#endif /* ifdef SAFETY */

