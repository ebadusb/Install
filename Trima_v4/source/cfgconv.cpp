
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:      Terry Wahl

*     Class name:  MAIN

*     File name:   cfgconv.cpp

*     Contents:    Implementations of class methods

*     Description: Main driver of the Config conversion utility

*************************< TRIMA CONFIG Conversion >****************************

$Header: K:/BCT_Development/Common/disk_build/install_source/cfgconv_50_to_40/rcs/cfgconv.cpp 1.1 2001/08/16 13:49:47 jl11312 Exp $
$Log: cfgconv.cpp $
Revision 1.1  2001/08/16 13:49:47  jl11312
Initial revision
Revision 1.1  1999/12/10 14:37:28  BS04481
Initial revision
*/

// EXTERNAL REFERENCES

#include "old_cfg_collect.hpp"
#include "new_drvr_collect.hpp"


// System #INCLUDES

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>



////////////////////////////////////////////////////////////////////////////////
//LOAD_DRIVER_ELEMENTS
void
NEW_drvr_collect::load_driver_elements ()
{

#include "new_driver_rules.h"


};  // END of NEW_drvr_collect::load_driver_elements




// THE START OF THE CONFIG CONVERSION PROCESS


int main(int , char** )
{

 #include "filenames.h"

 //Collection to hold all the keyword/value pairs in the old configuration file
 OLD_cfg_collect   old_cnf_repository ;

 //Collection of all the conversion rules / methods to generate the new configuration file
 NEW_drvr_collect  new_driver_collection ;


 ///////////////////////////////////////////////////////////////////////////

 //Check to see if we have an archive of the original OLD file. If so, make a copy of it as the original OLD file.
 if (access(RENAMED_OLD_CONFIG_FILE,  F_OK) == 0) // archive exists of the original OLD_CONFIG_FILE
 {
    //Make a copy of the archive using the OLD version's name
    char copy_command[200] = "cp " ;
    strcat(copy_command, RENAMED_OLD_CONFIG_FILE) ;
    strcat(copy_command, " ") ;
    strcat(copy_command, OLD_CONFIG_FILE) ;
    system (copy_command) ;
 }


 //Load up the old configuration file
 old_cnf_repository.load_old_config_collection (OLD_CONFIG_FILE) ;

 //Initialize the driver collection
 new_driver_collection.load_driver_elements();


 //Rename the old config file to an archive version, if it hasn't already been archived
 if (access(RENAMED_OLD_CONFIG_FILE,  F_OK) != 0)          // couldn't find or had problems
                                                           // finding a valid file
 {
    rename(OLD_CONFIG_FILE, RENAMED_OLD_CONFIG_FILE) ;
 }


 //Generate the new configuration file
 new_driver_collection.generate_new_config_file (NEW_CONFIG_FILE , &old_cnf_repository) ;

 //Truly serious errors have their individual non-zero "exits" throughout the code.
 return (0) ;

};  // End of the Main


