/*
 * Copyright (C) 2013 TerumoBCT, Inc.  All rights reserved.
 * 
 * This is a clone of cassette.cpp from trima_common_reentrant with stuff that the
 * Install script can't use stripped out and renamed to prevent collisions
 * 
 */

#include <vxworks.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "updatecassette.h"

UpdateCatalogNumber::UpdateCatalogNumber(const char* refNum, unsigned int adminCode, const char* barcodeString):
   _adminCode(adminCode),
   _functBits(_adminCode >> 16)
{
   strcpy(_refNum, refNum);
   strcpy(_barcode, barcodeString);

   unsigned int newRefNum = atoi(refNum);
};


