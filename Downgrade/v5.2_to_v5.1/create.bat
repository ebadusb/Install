if not defined WIND_HOST_TYPE goto find_tornado
if not defined WIND_BASE goto find_tornado
goto tornado_ok

:tornado_ok
echo Start of Downgrade build
date/t
time/t
rem 
rem Build the trima software
rem
make -f makefile.vx %*
if ERRORLEVEL 1 goto end_of_script

rem
rem Build the trima current_build archive
rem
make -f makefile.vx archive %*
rem make -f makefile.vx %* latest_trima_build_dir.mk
goto end_of_script

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

:not_trima_sandbox
echo create must be run from the base sandbox directory
goto end_of_script

:no_tornado
echo Unable to locate the Tornado installation directory.
echo The file torVars.bat from the Tornado host tools must be run to
echo properly set the build environment
goto end_of_script

:end_of_script

