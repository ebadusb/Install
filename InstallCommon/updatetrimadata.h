/*
 * Copyright (C) 2010 CaridianBCT, Inc.  All rights reserved.
 *
 * Install program for the Trima/vxWorks system
 */
#ifndef UPDATETRIMADATA_H
#define UPDATETRIMADATA_H

/*
// Here's the struct definitions for reference only - this may be out of date
// the real definition is in updateTrimaUtils.h

struct buildDataStruct
{
   const char * buildNum;
   int extractType;
   bool ampro;
   bool versalogic;
   bool python;
   bool ocelot;
   bool adjPASRAS;
   int calFileType;
   bool adjPostcount;
   bool updateTrapFiles;
   bool setConfigCopy;
   RANGETYPE rangeType;
};


struct rangeStruct {
   RANGETYPE rangeType;
   const char * section;
   const char * dataKey;
   RANGECOMPARETYPE compareType;
   RANGEVARIABLETYPE valType;
   const char * value;
   bool useLimit;
};
*/

// This is a list of the various supported builds and the conversion flags for the installer to use for
// determining what functions to run when installing that build.
// If a build is installed that isn't listed, the closest build with a smaller build number is used.
// (e.g if build 6.327 is installed, the flags for 6.301 will be used)
// The last value is the rangeType which maps to the rangeData list (below) to determine
// what config.dat vales to range check.
buildDataStruct buildData[] =
{
   {"6.301", 5, true, true, false, false, false, 5, true, false, false, V510},
   {"6.328", 5, true, true, false, false, false, 5, true, false, false, V510},
   {"6.343", 5, true, true, false, false, false, 5, true, false, false, V510},
   {"6.363", 5, true, true, false, false, false, 5, true, true, false, V510},
   {"6.365", 5, true, true, false, false, false, 5, true, true, false, V510},
   {"6.1.003", 5, true, true, false, false, false, 5, true, true, false, V510},
   {"6.381", 5, true, true, false, false, false, 5, true, true, false, V510},
   {"6.511", 5, true, true, false, false, false, 5, true, true, false, V510},
   {"6.00648", 5, true, true, true, false, false, 5, true, true, false, V510},
   {"6.00704", 5, true, true, true, true, false, 5, true, true, false, V510},
   {"7.303", 5, true, true, false, false, false, 6, false, false, false, V520},
   {"7.308", 5, true, true, false, false, false, 6, false, false, false, V520},
   {"7.316", 5, true, true, false, false, false, 6, false, false, false, V520},
   {"7.310", 5, true, true, false, false, false, 6, false, false, false, V520},
   {"8.232", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.235", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.241", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.630", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.661", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.662", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.672", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.0.714", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.1.003", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.0.748", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.4.00034", 6, true, true, true, false, true, 6, true, true, false, V600},
   {"8.5.00038", 6, true, true, true, true, true, 6, true, true, false, V600},
   {"8.7.00016", 6, true, true, true, true, true, 6, true, true, false, V600},
   {"8.8.00074", 6, true, true, true, true, true, 6, true, true, false, V600},
   {"8.9.00001", 6, true, true, true, true, true, 6, true, true, false, V600},
   {"9.0.034", 6, true, true, true, true, true, 6, false, true, false, V600},
   {"9.0.061", 6, true, true, true, true, true, 6, false, true, false, V600},
   {"9.0.100", 6, true, true, true, true, true, 6, false, true, false, V600},
   {"9.0.117", 6, true, true, true, true, true, 6, false, true, false, V600},
   {"9.0.118", 6, true, true, true, true, true, 6, false, true, false, V600},
   {"9.0.00394", 6, true, true, true, true, true, 6, false, true, false, V600},
   {"10.0.00037", 6, true, true, true, true, true, 6, true, true, false, V600},
   {"11.0.00368", 6, false, true, true, true, true, 6, true, true, true, V630},
   {"12.0.00001", 6, false, true, true, true, true, 6, true, true, false, V630},
   {"12.1.00001", 6, false, true, true, true, true, 6, true, true, false, V640},
   {"20.0.00001", 6, false, true, true, true, true, 6, true, true, false, V700},
   {"END", 6, false, true, true, true, true, 6, true, true, false, V640}
};

// This is a list of range values for settings that have different ranges for different versions.
// Settings that have the same ranges across all versions aren't included because the Trima software
// will enforce ranges prior to installing a new version.
// The rangeType member corresponds to the rangeType in the buildData data (above)
// look in cfg_vars.cpp and configdef.h for values
rangeStruct rangeData[] =
{
//   {V510, "LANGUAGE_UNIT_CONFIG", "key_lang", NOT, INT, "4", false},
   {V510, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0", false},
   {V510, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "7", false},
   {V510, "PROCEDURE_CONFIG", "key_return_press", MIN, INT, "100", false},
   {V510, "PROCEDURE_CONFIG", "key_return_press", MAX, INT, "310", false},
   {V510, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.8", false},
   {V510, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.2", false},
   {V510, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "6", false},
   {V510, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "3", false},
   {V510, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "8", false},
   {V510, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "3", false},

   {V520, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0", false},
   {V520, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "7", false},
   {V520, "PROCEDURE_CONFIG", "key_return_press", MIN, INT, "100", false},
   {V520, "PROCEDURE_CONFIG", "key_return_press", MAX, INT, "310", false},
   {V520, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.8", false},
   {V520, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.2", false},
   {V520, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "6", false},
   {V520, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "3", false},
   {V520, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "8", false},
   {V520, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "3", false},

   {V600, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0", false},
   {V600, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "15", false},
   {V600, "PROCEDURE_CONFIG", "key_return_press", MIN, INT, "100", false},
   {V600, "PROCEDURE_CONFIG", "key_return_press", MAX, INT, "310", false},
   {V600, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.9", false},
   {V600, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.1", false},
   {V600, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_1", MIN, INT, "80", false},
   {V600, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_2", MIN, INT, "80", false},
   {V600, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_3", MIN, INT, "80", false},
   {V600, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "10", false},
   {V600, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "3", false},
   {V600, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "8", false},
   {V600, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "3", false},

   {V630, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0", false},
   {V630, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "63", false},
   {V630, "PROCEDURE_CONFIG", "key_return_press", MIN, INT, "100", false},
   {V630, "PROCEDURE_CONFIG", "key_return_press", MAX, INT, "310", false},
   {V630, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.9", false},
   {V630, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.1", false},
   {V630, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_1", MIN, INT, "80", false},
   {V630, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_2", MIN, INT, "80", false},
   {V630, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_3", MIN, INT, "80", false},
   {V630, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "10", false},
   {V630, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "3", false},
   {V630, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "9", false},
   {V630, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "3", false},

   {V640, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0", false},
   {V640, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "63", false},
   {V640, "PROCEDURE_CONFIG", "key_return_press", MIN, INT, "100", false},
   {V640, "PROCEDURE_CONFIG", "key_return_press", MAX, INT, "310", false},
   {V640, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.9", false},
   {V640, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.1", false},
   {V640, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_1", MIN, INT, "80", false},
   {V640, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_2", MIN, INT, "80", false},
   {V640, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_3", MIN, INT, "80", false},
   {V640, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "10", false},      // NUM_YIELD_VOLUME
   {V640, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "6", false},  // MAX_KEY_PLT_AMAP
   {V640, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "9", false},         // MAX_KEY_PLASMA
   {V640, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "6", false},            // NUM_CRIT_DOSE

   {V700, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0", false},
   {V700, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "63", false},
   {V700, "PROCEDURE_CONFIG", "key_return_press", MIN, INT, "230", true},
   {V700, "PROCEDURE_CONFIG", "key_return_press", MAX, INT, "310", false},
   {V700, "PROCEDURE_CONFIG", "key_return_press", NOT, INT, "310", false},     // use the default if the value = 310
   {V700, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.9", false},
   {V700, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.1", false},
   {V700, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_1", MIN, INT, "80", false},
   {V700, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_2", MIN, INT, "80", false},
   {V700, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_3", MIN, INT, "80", false},
   {V700, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "10", false},      // NUM_YIELD_VOLUME
   {V700, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "6", false},  // MAX_KEY_PLT_AMAP
   {V700, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "9", false},         // MAX_KEY_PLASMA
   {V700, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "6", false},            // NUM_CRIT_DOSE

   {END, "xxx", "xxx", MAX, INT, "0"}
};

#endif // UPDATETRIMADATA_H

/* FORMAT HASH 8970359ba07d5c7ffe6a1eddc8a8776b */
