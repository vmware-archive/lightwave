rem ---------------------------------------------------------------------------
rem Configure DomainController and install vmidentity and SecureTokenServer
rem
rem Copyright (c) 2016 VMware, Inc.  All rights reserved.
rem ---------------------------------------------------------------------------


@echo off

set IDM_KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\Vmware, Inc.\VMware Identity Services"
set INSTALL_PATH="InstallPath"
set CONFIG_PATH="ConfigPath"
set INSTALL_PATH="InstallPath"
set LOGS_PATH="LogsPath"

for /f "tokens=2*" %%a in ('REG QUERY %IDM_KEY_NAME% /v %INSTALL_PATH%') do set "IDM_INSTALL=%%b"

for /f "tokens=2*" %%a in ('REG QUERY %IDM_KEY_NAME% /v %LOGS_PATH%') do set "IDM_LOG_PATH=%%b"

for /f "tokens=2*" %%a in ('REG QUERY %IDM_KEY_NAME% /v %CONFIG_PATH%') do set "IDM_CONFIG_PATH=%%b"

set CLASSPATH="%IDM_INSTALL%*";".\lstool\lib\*";"C:\Program /Files\VMware\CIS\vmware-sso\commonlib\*";.;*

cd %IDM_INSTALL%

set SEC_FILE=%IDM_CONFIG_PATH%\%JAVA_SECURITY_FILE%

"%VMWARE_JAVA_HOME%\bin\java" -Djava.security.properties=%SEC_FILE%  ^
 -Dvmware.log.dir=%IDM_LOG_PATH%  ^
 -Dinstall.log.file=%IDM_LOG_PATH%\standalone-install.log  ^
 -Dlog4j.configuration=file:"%IDM_INSTALL%\installer-log4j.properties" ^
 -cp %CLASSPATH% com.vmware.identity.configure.VMIdentityStandaloneInstaller ^
 --backup-folder %1 --identity-conf-file-path %2 --upgrade