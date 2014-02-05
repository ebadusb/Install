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
   {"END", 6, false, true, true, true, true, 6, true, true, false, V640}
};

// This is a list of range values for settings that have different ranges for different versions.
// Settings that have the same ranges across all versions aren't included because the Trima software
// will enforce ranges prior to installing a new version.
// The rangeType member corresponds to the rangeType in the buildData data (above)
// look in cfg_vars.cpp and configdef.h for values
rangeStruct rangeData[] =
{
//   {V510, "LANGUAGE_UNIT_CONFIG", "key_lang", NOT, INT, "4"},
   {V510, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0"},
   {V510, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "7"},
   {V510, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.8"},
   {V510, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.2"},
   {V510, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "6"},
   {V510, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "3"},
   {V510, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "8"},
   {V510, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "3"},

   {V520, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0"},
   {V520, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "7"},
   {V520, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.8"},
   {V520, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.2"},
   {V520, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "6"},
   {V520, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "3"},
   {V520, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "8"},
   {V520, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "3"},

   {V600, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0"},
   {V600, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "15"},
   {V600, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.9"},
   {V600, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.1"},
   {V600, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_1", MIN, INT, "80"},
   {V600, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_2", MIN, INT, "80"},
   {V600, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_3", MIN, INT, "80"},
   {V600, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "10"},
   {V600, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "3"},
   {V600, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "8"},
   {V600, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "3"},

   {V630, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0"},
   {V630, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "63"},
   {V630, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.9"},
   {V630, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.1"},
   {V630, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_1", MIN, INT, "80"},
   {V630, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_2", MIN, INT, "80"},
   {V630, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_3", MIN, INT, "80"},
   {V630, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "10"},
   {V630, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "3"},
   {V630, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "9"},
   {V630, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "3"},

   {V640, "LANGUAGE_UNIT_CONFIG", "key_lang", FORCE, INT, "0"},
   {V640, "PROCEDURE_CONFIG", "key_tbv_vol_setting", MAX, INT, "63"},
   {V640, "PREDICTION_CONFIG", "key_rsf", MIN, FLOAT, "0.9"},
   {V640, "PREDICTION_CONFIG", "key_rsf", MAX, FLOAT, "1.1"},
   {V640, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_1", MIN, INT, "80"},
   {V640, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_2", MIN, INT, "80"},
   {V640, "PRODUCT_TEMPLATES", "key_rbc_mss_volume_3", MIN, INT, "80"},
   {V640, "PRODUCT_DEFINITIONS", "key_platelet_", MAX, INT, "10"},      // NUM_YIELD_VOLUME
   {V640, "PRODUCT_DEFINITIONS", "key_platelet_amap_", MAX, INT, "6"},  // MAX_KEY_PLT_AMAP
   {V640, "PRODUCT_DEFINITIONS", "key_plasma_", MAX, INT, "9"},         // MAX_KEY_PLASMA
   {V640, "PRODUCT_DEFINITIONS", "key_rbc_", MAX, INT, "6"},            // NUM_CRIT_DOSE

   {END, "xxx", "xxx", MAX, INT, "0"}
};

#endif // UPDATETRIMADATA_H

/* FORMAT HASH 7b2013258dc62416b0e21909079581bc */
