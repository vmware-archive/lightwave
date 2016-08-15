@echo on
set LOGDIR="logs"
set LOG=%LOGDIR%\win_build.log
set DEBUGLOG=%LOGDIR%\debug_build.log
set RELEASELOG=%LOGDIR%\release_build.log
set INTEROPDIR=..\interop\lib64

set MS_BUILD4="%windir%\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe"
(echo MS_BUILD4=%MS_BUILD4%)>>%LOG%
echo replace VMIdentity.CommonUtils.dll.config content with Brand_lw.config
copy /Y ..\common\VMIdentity.CommonUtils\Brand_lw.config ..\common\VMIdentity.CommonUtils\VMIdentity.CommonUtils.dll.config


if NOT EXIST %LOGDIR% (
  mkdir %LOGDIR%
)

(echo =======================================================================================)>>%LOG%
CALL :cleanup

CALL :buildAllInterops ALL Release

CALL :copyinterops ALL Release


(echo =======================================================================================)>>%LOG%
(echo starting windows build ...)>>%LOG%
CALL :buildWithMSBUILD4 VMCertStoreSnapIn\VMCertStoreSnapIn.sln Debug
CALL :buildWithMSBUILD4 VMDirSnapIn\VMDirSnapIn.sln Debug
CALL :buildWithMSBUILD4 VMDirSchemaSnapIn\VMDirSchemaSnapIn.sln Debug
CALL :buildWithMSBUILD4 VMCASnapIn\VMCASnapIn.sln Debug
CALL :buildWithMSBUILD4 VMRestSsoAdminSnapIn\RestSsoAdminSnapIn.sln Debug
CALL :buildWithMSBUILD4 VMPscHighAvailabilitySnapIn\VMPscHighAvailabilitySnapIn.sln Debug
CALL :buildWithMSBUILD4 wininstaller\wininstaller.sln Debug

echo ------------ Release ---------------
echo Build Release x64


CALL :buildWithMSBUILD4 VMCertStoreSnapIn\VMCertStoreSnapIn.sln Release
CALL :buildWithMSBUILD4 VMDirSnapIn\VMDirSnapIn.sln Release
CALL :buildWithMSBUILD4 VMDirSchemaSnapIn\VMDirSchemaSnapIn.sln Release
CALL :buildWithMSBUILD4 VMCASnapIn\VMCASnapIn.sln Release
CALL :buildWithMSBUILD4 VMRestSsoAdminSnapIn\RestSsoAdminSnapIn.sln Release
CALL :buildWithMSBUILD4 VMPscHighAvailabilitySnapIn\VMPscHighAvailabilitySnapIn.sln Release
CALL :buildWithMSBUILD4 wininstaller\wininstaller.sln Release

goto end


REM build all interop solutions in a particular config
:buildAllInterops
    set result = false
    if %1% == ALL set result=true
    if %1% == VMPSCHighAvailabilitySnapIn set result=true

    if %result% == true (
        REM build - psc vmdir
        CALL :buildWithMSBUILD4 ..\..\vmdir\dotnet\VMDIR.Client\VMDIR.Client.csproj %2
        REM build - psc vmafd
        CALL :buildWithMSBUILD4 ..\..\vmafd\dotnet\VMAFD.Client\VMAFD.Client.csproj %2
    )
    
    set result = false
    if %1% == ALL set result=true
    if %1% == VMDirSnapIn set result=true

    if %result% == true (
        REM build - vmdir
        CALL :buildWithMSBUILD4 ..\..\vmdir\interop\csharp\VmDirInterop\VmDirInterop.sln %2
    )
    exit /b

REM copy interops to the lib folder
:copyinterops
    
    if NOT EXIST %INTEROPDIR% (
        mkdir %INTEROPDIR%
        )

    echo 'move interops to lib64 folder started ..'
    
    set result=false
    if %1% == ALL set result=true
    if %1% == VMPSCHighAvailabilitySnapIn set result=true

    if %result% == true (
        echo 'move pscha'
        copy /Y ..\..\vmdir\dotnet\VMDIR.Client\bin\%2\* %INTEROPDIR%\
        copy /Y ..\..\vmafd\dotnet\VMAFD.Client\bin\%2\* %INTEROPDIR%\
    )

    set result=false
    if %1% == ALL set result=true
    if %1% == VMDirSnapIn set result=true

    if %result% == true (
        echo 'move vmdir'
        copy /Y ..\..\vmdir\interop\csharp\VmDirInterop\VmDirInterop\bin\%2\* %INTEROPDIR%\
    
    )

    echo 'interops moved successfully to lib64 folder'
    
    exit /b

REM build solutions in a particular config
:buildWithMSBUILD4
  if %2 == Debug (
    %MS_BUILD4% %1 /t:Rebuild /p:Configuration=Debug /l:FileLogger,Microsoft.Build.Engine;logfile=%DEBUGLOG%
    if errorlevel 1 (
      (echo VS 2012 %1 Debug FAILED)>>%DEBUGLOG%
      echo VS 2012 %1 Debug FAILED
      goto error
    )
  )
  if %2 == Release (
    %MS_BUILD4% %1 /t:Rebuild /p:Configuration=Release /l:FileLogger,Microsoft.Build.Engine;logfile=%RELEASELOG%
      if errorlevel 1 (
        (echo VS 2012 %1 Release FAILED)>>%RELEASELOG%
        echo VS 2012 %1 Release FAILED
        goto error
      )
    )
  exit /b

REM cleanup the interop lib directory
:cleanup
    if exist %INTEROPDIR% (
        rm -rf %INTEROPDIR%\*
    )
    exit /b

:error

(echo !!!!!!!!!!Build FAILED!!!!!!!!!! [Log: "%LOG%"])>>%LOG%
echo !!!!!!!!!!Build FAILED!!!!!!!!!! [Log: "%LOG%"]
if not %BUILDLOG_DIR% == "" (
    copy %LOGDIR%\*.log %BUILDLOG_DIR:/=\%
)
popd
exit /b 1

:end

(echo !!!!!!!!!!Build succeeded!!!!!!!!!!)>>%LOG%
echo !!!!!!!!!!Build succeeded!!!!!!!!!!
if not "%BUILDLOG_DIR%" == "" (
    copy %LOGDIR%\*.log %BUILDLOG_DIR:/=\%
)

popd
exit /b 0
