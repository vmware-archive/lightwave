rem ---------------------------------------------------------------------------
rem Configure DomainController and install vmidentity and SecureTokenServer
rem
rem Copyright (c) 2016 VMware, Inc.  All rights reserved.
rem ---------------------------------------------------------------------------


@echo off

set IDM_KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\Vmware, Inc.\VMware Identity Services"
set VMAFD_KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\Vmware, Inc.\VMware Afd Services"
set CONFIG_PATH="ConfigPath"
set INSTALL_PATH="InstallPath"
set LOGS_PATH="LogsPath"

for /f "tokens=2*" %%a in ('REG QUERY %IDM_KEY_NAME% /v %INSTALL_PATH%') do set "IDM_INSTALL=%%b"

for /f "tokens=2*" %%a in ('REG QUERY %IDM_KEY_NAME% /v %LOGS_PATH%') do set "idmLogsPath=%%b"

for /f "tokens=2*" %%a in ('REG QUERY %VMAFD_KEY_NAME% /v %CONFIG_PATH%') do set "vmafdConfigPath=%%b"

for /f "tokens=2*" %%a in ('REG QUERY %VMAFD_KEY_NAME% /v %INSTALL_PATH%') do set "vmafdInstallPath=%%b"


set JAVA_SECURITY_FILE=java.security.windows
set PATH=%PATH%;%vmafdInstallPath%

set /p domain=Enter domain:%=%

set /p username=Enter username:%=%

powershell -Command $pword = read-host "Enter password" -AsSecureString ; ^
    $BSTR=[System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($pword) ; ^
        [System.Runtime.InteropServices.Marshal]::PtrToStringAuto($BSTR) > .tmp.txt
set /p password=<.tmp.txt & del .tmp.txt
set CLASSPATH="%IDM_INSTALL%\*;./lstool/lib/*;C:\Program Files\VMware\CIS\vmware-sso\commonlib\*;.;*"

cd %IDM_INSTALL%

IF EXIST %vmafdConfigPath% (
   set SEC_FILE=%vmafdConfigPath%\%JAVA_SECURITY_FILE%

   echo "Configuring IDM and STS installers..."
   java -Djava.security.properties=%SEC_FILE%  ^
        -Dvmware.log.dir=%idmLogsPath%  ^
        -Dinstall.log.file=%idmLogsPath%\standalone-install.log  ^
        -Dlog4j.configuration=file:"%IDM_INSTALL%\installer-log4j.properties" ^
        -cp %CLASSPATH% com.vmware.identity.configure.VMIdentityStandaloneInstaller ^
        --domain %domain% --username %username% --password %password%  2>%idmLogsPath%\standalone-install-err.txt


) ELSE (
   echo "Java Security file not found. Cannot load certificates from VKS."
   exit 1
)
