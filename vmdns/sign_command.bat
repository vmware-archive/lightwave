rem
rem A wrapper for digitalsign tool
rem
rem Original Author: vpokrovskii
rem 

if "%TCROOT%" == "" (
    echo ERROR: TCROOT not set
    exit /b 1
)

if "%BUILD_NUMBER%" == "" (
    set BUILD_NUMBER=%2
)

if "%BUILD_NUMBER%" == "" (
    echo ERROR: BUILD_NUMBER not set
    exit /b 1
)

if not exist %TCROOT%\win32 (
    echo Could not find toolchain %TCROOT%\win32
    exit /b 1
)

if not exist %TCROOT%\noarch (
    echo Could not find toolchain %TCROOT%\noarch
    exit /b 1
)

set PERL=%TCROOT%\win32\perl-5.8.8\bin\perl.exe
set PYTHON=%TCROOT%\win32\python-2.6.1\python.exe
set WINLOCK=%PYTHON% %TCROOT%\noarch\vmware\winlock\winlock.py
set WINLOCK_DIGITALSIGN=%WINLOCK% -v -m digitalsign --

set DIGITALSIGN=%PERL% %TCROOT%\noarch\vmware\digitalsign\digitalsign.pl^
 sign -b %BUILD_NUMBER%^
 -s %TCROOT%\win32\winddk-6000.16386\bin\SelfSign^
 -v -a

if "%OFFICIALKEY%" == "1" (
    set DIGITALSIGN=%DIGITALSIGN% -ob
)

set DIGITALSIGN_LOCKED=%WINLOCK_DIGITALSIGN% %DIGITALSIGN%

%DIGITALSIGN_LOCKED% %1
