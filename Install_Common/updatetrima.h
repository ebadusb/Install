/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */

#include <vxWorks.h>
#include <cbioLib.h>
#include <dosFsLib.h>
#include <private/dosFsLibP.h>
#include <bootLib.h>
#include <sysLib.h>
#include <taskLib.h>
#include <unistd.h>
#include <usrLib.h>
#include <fcntl.h>

#include <cmath>
#include <cstdio>
#include <string>

#define TRIMA_PRIORITY 250


