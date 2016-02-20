/*
 * $Header$

 * $Log: $
 *
 */

#ifndef _DISCOVER_HW_INCLUDE
#define _DISCOVER_HW_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

unsigned char isAmpro(void);            /* 0-Not Ampro      1-Ampro */
unsigned char isVersalogic(void);       /* 0-Not Versalogic 1-Versalogic */
unsigned char isVersalogicVSBC6(void);  /* 0-Not VSBC-6     1-VSBC-6*/
unsigned char isVersalogicPython(void); /* 0-Not Python     1-Python */
unsigned char isVersalogicFox(void);    /* 0-Not Fox        1-Fox */

unsigned char isCT655XXGraphics(void);  /* 0-Not Chip & Tech.  1-Chips & Tech. */
unsigned char isGeodeLXGraphics(void);  /* 0-Not Geode LX Chip 1-Geode LX Chip */
unsigned char isRDCGraphics(void);      /* 0-Not RDC Gfx       1-RDC Gfx       */

unsigned int getGraphicsVendorID(void);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _DISCOVER_HW_INCLUDE */

