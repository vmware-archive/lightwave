@echo on
set LOGDIR=%CD%
set LOG=%CD%\build.log
set DEBUGLOG=%CD%\debug_build.log
set RELEASELOG=%CD%\release_build.log
set PATH=%TCROOT%\win32\winsdk-7.0.7600\bin;%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\SysWOW64\wbem;%SystemRoot%\system32\wbem;%PATH%

(echo starting windows build ...)>%LOG%

set BUILD_VS2013="%ProgramFiles%\MSBuild\12.0\Bin\MsBuild.exe"
(echo BUILD_VS2013=%BUILD_VS2013%)>>%LOG%
set SIGN_CMD="sign_command.bat"
(echo SIGN_CMD=%SIGN_CMD%)>>%LOG%

pushd .

(echo  )>>%LOG%
(echo =======================================================================================)>>%LOG%
(echo Build Debug x64: %BUILD_VS2013% VMDns.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64)>>%LOG%
echo Build Debug x64
%BUILD_VS2013% VMDns.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64 >>%LOG%
if errorlevel 1 (
  (echo VS 2013 VMDns.sln Debug x64 FAILED)>>%DEBUGLOG%
  echo VS 2013 VMDns.sln Debug x64 FAILED
  goto error
)

(echo  )>>%LOG%
(echo =======================================================================================)>>%LOG%
(echo Build Release x64: %BUILD_VS2013% VMDns.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64)>>%LOG%
echo Build Release x64
%BUILD_VS2013% VMDns.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 >>%LOG%
if errorlevel 1 (
  (echo VS 2013 VMDns.sln Release x64 FAILED)>>%RELEASELOG%
  echo VS 2013 VMDns.sln Release x64 FAILED
  goto error
)

if "%1" == "SIGN" (
    (echo SIGN parameter specified. signing the built bits...)>>%LOG%
    (call %SIGN_CMD% x64\Debug)>>%DEBUGLOG%
    if errorlevel 1 (
      (echo 'call %SIGN_CMD% x64\Debug' FAILED)>>%DEBUGLOG%
      goto error
    )
    (call %SIGN_CMD% x64\Release)>>%RELEASELOG%
    if errorlevel 1 (
      (echo 'call %SIGN_CMD% x64\Release' FAILED)>>%RELEASELOG%
      goto error
    )
)

(echo  )>>%LOG%
(echo =======================================================================================)>>%LOG%
(echo Build Setup Debug x64: %BUILD_VS2013% wininstaller\WinInstallers.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64)>>%LOG%
(echo Build Setup Debug x64: %BUILD_VS2013% wininstaller\WinInstallers.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64)>>%DEBUGLOG%
echo Build Setup Debug x64
(%BUILD_VS2013% wininstaller\WinInstallers.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64)>>%DEBUGLOG%
if errorlevel 1 (
  (echo VS 2013 WinInstallers.sln Debug x64 FAILED)>>%DEBUGLOG%
  echo VS 2013 WinInstallers.sln Debug x64 FAILED
  goto error
)

(echo  )>>%LOG%
(echo =======================================================================================)>>%LOG%
(echo Build Setup Release x64: %BUILD_VS2013% wininstaller\WinInstallers.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64)>>%LOG%
(echo Build Setup Release x64: %BUILD_VS2013% wininstaller\WinInstallers.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64)>>%RELEASELOG%
echo Build Setup Release x64
(%BUILD_VS2013% wininstaller\WinInstallers.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64)>>%RELEASELOG%
if errorlevel 1 (
  (echo VS 2013 WinInstallers.sln Release x64 FAILED)>>%RELEASELOG%
  echo VS 2013 WinInstallers.sln Release x64 FAILED
  goto error
)

if "%1" == "SIGN" (
    (echo SIGN parameter specified. signing the installer bits...)>>%LOG%
    (echo =======================================================================================)>>%LOG%
    (echo %SIGN_CMD% wininstaller\vmware-dns-server-msm\x64\Debug)>>%DEBUGLOG%
    (call %SIGN_CMD% wininstaller\vmware-dns-server-msm\x64\Debug)>>%DEBUGLOG%
    if errorlevel 1 (
      (echo 'call %SIGN_CMD% wininstaller\vmware-dns-server-msm\x64\Debug' FAILED)>>%DEBUGLOG%
      goto error
    )
    (echo %SIGN_CMD% wininstaller\vmware-dns-server-msm\x64\Release)>>%RELEASELOG%
    (call %SIGN_CMD% wininstaller\vmware-dns-server-msm\x64\Release)>>%RELEASELOG%
    if errorlevel 1 (
      (echo 'call %SIGN_CMD% wininstaller\vmware-dns-server-msm\x64\Release' FAILED)>>%RELEASELOG%
      goto error
    )
    (echo %SIGN_CMD% wininstaller\vmware-dns-server-msi\x64\Debug)>>%DEBUGLOG%
    (call %SIGN_CMD% wininstaller\vmware-dns-server-msi\x64\Debug)>>%DEBUGLOG%
    if errorlevel 1 (
      (echo 'call %SIGN_CMD% wininstaller\vmware-dns-server-msi\x64\Debug' FAILED)>>%DEBUGLOG%
      goto error
    )
    (echo %SIGN_CMD% wininstaller\vmware-dns-server-msi\x64\Release)>>%RELEASELOG%
    (call %SIGN_CMD% wininstaller\vmware-dns-server-msi\x64\Release)>>%RELEASELOG%
    if errorlevel 1 (
      (echo 'call %SIGN_CMD% wininstaller\vmware-dns-server-msi\x64\Release' FAILED)>>%RELEASELOG%
      goto error
    )
    (echo %SIGN_CMD% wininstaller\vmware-dns-client-msm\x64\Debug)>>%DEBUGLOG%
    (call %SIGN_CMD% wininstaller\vmware-dns-client-msm\x64\Debug)>>%DEBUGLOG%
    if errorlevel 1 (
      (echo 'call %SIGN_CMD% wininstaller\vmware-dns-client-msm\x64\Debug' FAILED)>>%DEBUGLOG%
      goto error
    )
    (echo %SIGN_CMD% wininstaller\vmware-dns-client-msm\x64\Release)>>%RELEASELOG%
    (call %SIGN_CMD% wininstaller\vmware-dns-client-msm\x64\Release)>>%RELEASELOG%
    if errorlevel 1 (
      (echo 'call %SIGN_CMD% wininstaller\vmware-dns-client-msm\x64\Release' FAILED)>>%RELEASELOG%
      goto error
    )
    (echo %SIGN_CMD% wininstaller\vmware-dns-client-msi\x64\Debug)>>%DEBUGLOG%
    (call %SIGN_CMD% wininstaller\vmware-dns-client-msi\x64\Debug)>>%DEBUGLOG%
    if errorlevel 1 (
      (echo 'call %SIGN_CMD% wininstaller\vmware-dns-client-msi\x64\Debug' FAILED)>>%DEBUGLOG%
      goto error
    )
    (echo %SIGN_CMD% wininstaller\vmware-dns-client-msi\x64\Release)>>%RELEASELOG%
    (call %SIGN_CMD% wininstaller\vmware-dns-client-msi\x64\Release)>>%RELEASELOG%
    if errorlevel 1 (
      (echo 'call %SIGN_CMD% wininstaller\vmware-dns-client-msi\x64\Release' FAILED)>>%RELEASELOG%
      goto error
    )
)


goto end

:error

(echo !!!!!!!!!!Build FAILED!!!!!!!!!! [Log: "%LOG%"])>>%LOG%
echo !!!!!!!!!!Build FAILED!!!!!!!!!! [Log: "%LOG%"]
if not %BUILDLOG_DIR% == "" (
    copy %LOGDIR%\*.log %BUILDLOG_DIR:/=\%
)
popd
exit /B 1

:end

(echo !!!!!!!!!!Build succeeded!!!!!!!!!!)>>%LOG%
echo !!!!!!!!!!Build succeeded!!!!!!!!!!
if not "%BUILDLOG_DIR%" == "" (
    copy %LOGDIR%\*.log %BUILDLOG_DIR:/=\%
)
mkdir %LOGDIR%\wininstaller\vmware-dns-server-msm\x64\Debug
mkdir %LOGDIR%\wininstaller\vmware-dns-server-msm\x64\Release
mkdir %LOGDIR%\wininstaller\vmware-dns-server-msi\x64\Debug
mkdir %LOGDIR%\wininstaller\vmware-dns-server-msi\x64\Release
copy %LOGDIR%\wininstaller\vmware-dns-server-msm\x64\Debug\*.msm %LOGDIR%\wininstaller\vmware-dns-server-msm\x64\Debug\
copy %LOGDIR%\wininstaller\vmware-dns-server-msm\x64\Release\*.msm %LOGDIR%\wininstaller\vmware-dns-server-msm\x64\Release\
copy %LOGDIR%\wininstaller\vmware-dns-server-msi\x64\Debug\*.msi %LOGDIR%\wininstaller\vmware-dns-server-msi\x64\Debug\
copy %LOGDIR%\wininstaller\vmware-dns-server-msi\x64\Release\*.msi %LOGDIR%\wininstaller\vmware-dns-server-msi\x64\Release\

popd
exit /B 0
