@echo off
rem
rem create_upgrade.bat - Rebuild Trima under vxWorks
rem
rem $Header: \\bctquad3\home\BCT_Development\Install\Trima_v6.0 1.8 2004/01/30 23:15:26Z ms10234 Exp $
rem
rem $Log: create_upgrade.bat $
rem

if not defined WIND_HOST_TYPE goto find_tornado
if not defined WIND_BASE goto find_tornado
goto tornado_ok

:tornado_ok
echo Start of Upgrade build
date/t
time/t
rem 
rem Build the upgrade software
rem
make -f makefile6X.vx %*
if ERRORLEVEL 1 goto build_error
goto build_success

:find_tornado
if exist c:\tornado\host\x86-win32\bin\torVars.bat goto tornado_on_c
if exist d:\tornado\host\x86-win32\bin\torVars.bat goto tornado_on_d
goto no_tornado

:tornado_on_c
call c:\tornado\host\x86-win32\bin\torVars.bat
goto tornado_ok

:tornado_on_d
call d:\tornado\host\x86-win32\bin\torVars.bat
goto tornado_ok

:no_tornado
echo Unable to locate the Tornado installation directory.
echo The file torVars.bat from the Tornado host tools must be run to
echo properly set the build environment
goto build_error

:build_error
echo Upgrade build failed.
goto end_of_script

:build_success
echo Upgrade build successful.

:end_of_script
