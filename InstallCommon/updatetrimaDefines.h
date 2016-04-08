/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */
#ifndef UPDATETRIMADEFINES_H
#define UPDATETRIMADEFINES_H

// This is to tell filenames.h to declare the installSpecialFiles variables
#define INSTALL_BUILD

// define the root for the install simulator as a relative path from where the simulator is run
#if (CPU==SIMNT)
   #ifdef SIMROOT
      #undef SIMROOT
   #endif
   #define SIMROOT Install
#endif

#ifdef __COMPILE_FOR_VX_54__

    #include "trima_opsys.h"

    #ifndef TEMPLATES_PATH
        #define TEMPLATES_PATH TRIMA_PATH "/config/templates"
    #endif // #ifndef TEMPLATES_PATH

    #ifndef FILE_CASSETTE_DAT
        #define FILE_CASSETTE_DAT "cassette.dat"
    #endif // #ifndef FILE_CASSETTE_DAT

    #ifndef FILE_SETCONFIG_DAT
        #define FILE_SETCONFIG_DAT "setconfig.dat"
    #endif // #ifndef FILE_SETCONFIG_DAT

    #ifndef FILE_SOUNDS_DAT
        #define FILE_SOUNDS_DAT "sounds.dat"
    #endif // #ifndef FILE_SOUNDS_DAT

    #ifndef PNAME_SOUNDSDAT
        #define PNAME_SOUNDSDAT CONFIG_PATH "/" FILE_SOUNDS_DAT
    #endif // #ifndef PNAME_SOUNDSDAT

    #ifndef CLINICAL_BUILD
        #define CLINICAL_BUILD CONFIG_PATH "/clinical_build"
    #endif // #ifndef CLINICAL_BUILD

    #ifndef TEST_BUILD
        #define TEST_BUILD CONFIG_PATH "/test_build"
    #endif // #ifndef TEST_BUILD

    #ifndef SOFTCRC_PATH
        #define SOFTCRC_PATH TRIMA_PATH "/softcrc"
    #endif // #ifndef SOFTCRC_PATH

    #ifndef FILELISTS_PATH
        #define FILELISTS_PATH SOFTCRC_PATH "/filelists"
    #endif // #ifndef FILELISTS_PATH

    #ifndef STRING_DIRECTORY
        #define STRING_DIRECTORY CONFIG_PATH "/strings"
    #endif // #ifndef STRING_DIRECTORY

    #ifndef DROP_IN_FONTS_DIR
        #define DROP_IN_FONTS_DIR CONFIG_PATH "/fonts"
    #endif // #ifndef DROP_IN_FONTS_DIR

    #ifndef DATA_DIRECTORY
        #define DATA_DIRECTORY CONFIG_PATH "/data"
    #endif // #ifndef DATA_DIRECTORY

    #ifndef GRAPHICS_PATH
        #define GRAPHICS_PATH      CONFIG_PATH "/graphics"
    #endif // #ifndef GRAPHICS_PATH

    #ifndef FILE_GUI_GRAPHICS
        #define FILE_GUI_GRAPHICS     "graphics_package.out"
    #endif // #ifndef FILE_GUI_GRAPHICS

    #ifndef FILE_SERVICE_GRAPHICS
        #define FILE_SERVICE_GRAPHICS "service_" FILE_GUI_GRAPHICS
    #endif // #ifndef FILE_SERVICE_GRAPHICS

    #ifndef PNAME_GUI_GRAPHICS
        #define PNAME_GUI_GRAPHICS      GRAPHICS_PATH "/"  FILE_GUI_GRAPHICS
    #endif // #ifndef PNAME_GUI_GRAPHICS

    #ifndef PNAME_SERVICE_GRAPHICS
        #define PNAME_SERVICE_GRAPHICS  GRAPHICS_PATH "/"  FILE_SERVICE_GRAPHICS
    #endif // #ifndef PNAME_SERVICE_GRAPHICS

    #ifndef PNAME_GUI_GRAPHICS_CRC
        #define PNAME_GUI_GRAPHICS_CRC  CONFIG_CRC_PATH "/graphics.crc"
    #endif // #ifndef PNAME_GUI_GRAPHICS_CRC

    #ifndef PNAME_FONT_CRC_FILE
        #define PNAME_FONT_CRC_FILE     CONFIG_CRC_PATH "/fonts.crc"
    #endif // #ifndef PNAME_FONT_CRC_FILE

    #ifndef PNAME_DATA_CRC_FILE
        #define PNAME_DATA_CRC_FILE     CONFIG_CRC_PATH "/data.crc"
    #endif // #ifndef PNAME_DATA_CRC_FILE

    #ifndef PNAME_STRING_CRC_FILE
        #define PNAME_STRING_CRC_FILE   CONFIG_CRC_PATH "/strings.crc"
    #endif // #ifndef PNAME_STRING_CRC_FILE

   #ifdef SAFETY_VXWORKS_PXE_IMAGE
      #undef SAFETY_VXWORKS_PXE_IMAGE
   #endif
    #define SAFETY_VXWORKS_PXE_IMAGE    SAFETY_BOOT_PATH "/vxWorks_pxe"

    #ifdef SAFETY_BOOTROM_PXE_IMAGE
      #undef SAFETY_BOOTROM_PXE_IMAGE
   #endif
    #define SAFETY_BOOTROM_PXE_IMAGE    SAFETY_BOOT_PATH "/bootrom_pxe.sys"

    #ifdef SAFETY_BOOTROM_IMAGE
        #undef SAFETY_BOOTROM_IMAGE
    #endif
    #define SAFETY_BOOTROM_IMAGE        SAFETY_BOOT_PATH "/bootrom.sys"

    #ifdef SAFETY_VXWORKS_IMAGE
        #undef SAFETY_VXWORKS_IMAGE
    #endif
    #define SAFETY_VXWORKS_IMAGE        SAFETY_BOOT_PATH "/vxWorks"

    #ifdef TRIMA_PROJECT_REVISION_FILE
        #undef TRIMA_PROJECT_REVISION_FILE
    #endif
    #define TRIMA_PROJECT_REVISION_FILE TRIMA_PATH "/projectrevision"

   #ifndef FILE_TCHSCRN_DAT
      #define FILE_TCHSCRN_DAT   "touch_screen.dat"
   #endif

   #ifndef PNAME_TCHSCRNDAT
      #define PNAME_TCHSCRNDAT   CONFIG_PATH "/" FILE_TCHSCRN_DAT
   #endif

#else // #ifdef __COMPILE_FOR_VX_54__

// These are the real include files.
    #include "node_data.h"
    #include "trimaOs.h"

#endif // #ifdef __COMPILE_FOR_VX_54__

// Make sure these are defined for everybody
#ifndef FILE_VISTA_IP_DAT
   #define FILE_VISTA_IP_DAT   "vista_ip.dat"
#endif

#ifndef PNAME_VISTAIPDAT
   #define PNAME_VISTAIPDAT   CONFIG_PATH "/" FILE_VISTA_IP_DAT
#endif

#ifndef FILE_FEATURES
   #define FILE_FEATURES "features.bin"
#endif

#ifndef PNAME_FEATURES
   #define PNAME_FEATURES CONFIG_PATH "/" FILE_FEATURES
#endif

// special files
#ifndef CLINICAL_BUILD
   #define CLINICAL_BUILD CONFIG_PATH "/clinical_build"
#endif

#ifndef TEST_BUILD
   #define TEST_BUILD CONFIG_PATH "/test_build"
#endif

#ifndef INSTALL_SPECIAL_FILES
const static char* installSpecialFiles[] = {CLINICAL_BUILD, TEST_BUILD};
const static int   numInstallSpecialFiles = (int)(sizeof(installSpecialFiles) / sizeof(installSpecialFiles[0]));
#endif

// To fix a change in 6.0 that renames these file defines
#ifndef PNAME_STRING_CRC
    #define PNAME_STRING_CRC PNAME_STRING_CRC_FILE
#endif

#ifndef PNAME_FONT_CRC
    #define PNAME_FONT_CRC PNAME_FONT_CRC_FILE
#endif

#ifndef PNAME_DATA_CRC
    #define PNAME_DATA_CRC PNAME_DATA_CRC_FILE
#endif

// for the new logging
#ifndef INSTALL_LOG_PATH
   #define INSTALL_LOG_PATH MACHINE_PATH "/install"
#endif

#ifndef FILE_INSTALL_LOG
   #define FILE_INSTALL_LOG "install_log.txt"
#endif

#ifndef PNAME_INSTALL_LOG
   #define PNAME_INSTALL_LOG INSTALL_LOG_PATH "/" FILE_INSTALL_LOG
#endif

#ifndef FILE_BARCODECATEGORY_DAT
   #define FILE_BARCODECATEGORY_DAT    "barcode_categories.dat"
#endif

#ifndef PNAME_BARCODECATEGORYDAT
   #define PNAME_BARCODECATEGORYDAT CONFIG_PATH "/" FILE_BARCODECATEGORY_DAT
#endif

#ifndef FILE_RTSCONFIG_DAT
   #define FILE_RTSCONFIG_DAT    "rts_config.dat"
#endif

#ifndef PNAME_RTSCONFIGDAT
   #define PNAME_RTSCONFIGDAT CONFIG_PATH "/" FILE_RTSCONFIG_DAT
#endif

#ifndef PNAME_BARCODE_LOG
   #define PNAME_INSTALL_LOG INSTALL_LOG_PATH "/" FILE_INSTALL_LOG
#endif

// 6.4 removes terror from filenames.dat
#ifndef TERROR_CONFIG_FILE
    #define TERROR_CONFIG_FILE CONFIG_PATH       "/terror_config.dat"
#endif

#ifndef FILE_MACHINE_ID
    #define FILE_MACHINE_ID     "machine.id"
#endif

#ifndef FILE_MACHINE_ID_TMP
    #define FILE_MACHINE_ID_TMP "machine.tmp"
#endif

#ifndef PNAME_MACHINE_ID
    #define PNAME_MACHINE_ID        CONFIG_PATH "/" FILE_MACHINE_ID
#endif

#ifndef PNAME_MACHINE_ID_TMP
    #define PNAME_MACHINE_ID_TMP    TEMP_PATH "/" FILE_MACHINE_ID_TMP
#endif

// common kernel defines
#ifndef SAFETY_COMMON_KERNEL_INIT_PATH
   #define SAFETY_COMMON_KERNEL_INIT_PATH  CONFIG_PATH "/kernel_init/safety"
#endif

#ifndef SAFETY_KERNEL_INIT_SRC
   #define SAFETY_KERNEL_INIT_SRC          TRIMA_PATH  "/kernel_init/safety_init.out"
#endif

#ifndef SAFETY_KERNEL_INIT_TMP
   #define SAFETY_KERNEL_INIT_TMP          TEMP_PATH   "/safety_init.out"
#endif

#endif // UPDATETRIMADEFINES_H

/* FORMAT HASH f17f6b050d151b51abd3475c85bd595e */
