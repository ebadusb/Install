/*
 * Copyright (C) 2013 TerumoBCT, Inc.  All rights reserved.
 * 
 * This is a clone of cassette.h from trima_common with stuff that the
 * Install script can't use stripped out and renamed to prevent collisions
 * 
 */

#ifndef UPDATE_CASSETTE_H
#define UPDATE_CASSETTE_H


class UpdateCatalogNumber
{
public:
   typedef enum { BLACK_STAMP = 0, WHITE_STAMP = 1 } STAMP_COLOR;

   UpdateCatalogNumber(const char* refNum, unsigned int adminCode, const char* barcodeString);

   const char *RefNum () const { return _refNum; };
   const char *BarcodeNum () const { return _barcode; };
   unsigned int AdminCode() const { return _adminCode; };
   unsigned int FunctBits() const { return _functBits; };

private:
   unsigned int _adminCode, _functBits;
   char _refNum[16];
   char _barcode[256];

   UpdateCatalogNumber(); //default constructor disallowed (private)
};


#endif

