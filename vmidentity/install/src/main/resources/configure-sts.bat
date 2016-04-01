rem ---------------------------------------------------------------------------
rem Configure STS
rem
rem Copyright (c) 2015 VMware, Inc.  All rights reserved.
rem ---------------------------------------------------------------------------


set TC_KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\Vmware, Inc.\vtcServer"
set TC_INSTALL_PATH="InstallPath"
for /f "tokens=2*" %%a in ('REG QUERY %TC_KEY_NAME% /v %TC_INSTALL_PATH%') do set "tcInstallPath=%%b"

Set JAVA_HOME=C:\Progra~1\Java\jre1.8.0_45
Set WEBAPPS_PATH="C:\Program Files\VMware\CIS\VMware Identity Services\Tomcat\webapps"
set TC_WEBAPPS_PATH="C:\ProgramData\VMware\CIS\runtime\VMwareSTSService\webapps\"
set TC_INSTANCE_ROOT="%tcInstallPath%\tcServer"
set TC_INSTANCE_NAME=VMwareSTSService
Set TC_INSTANCE_BASE=%TC_INSTANCE_ROOT%\%TC_INSTANCE_NAME%
set TOMCAT_INSTALL="C:\Program Files\VMware\CIS\VMware Identity Services\Tomcat"

xcopy /E /Y /I %WEBAPPS_PATH% %TC_WEBAPPS_PATH%
goto init

:init
xcopy /E /Y /I %TOMCAT_INSTALL%\temp\bio-custom %TC_INSTANCE_ROOT%\templates\bio-custom
xcopy /E /Y /I %TOMCAT_INSTALL%\temp\bio-ssl-localhost %TC_INSTANCE_ROOT%\templates\bio-ssl-localhost

mkdir C:\ProgramData\VMware\CIS\runtime

cd C:\ProgramData\VMware\CIS\runtime

%TC_INSTANCE_ROOT%\tcruntime-instance.bat create -t bio-custom --property bio-custom.http.port=7080 --force -t bio-ssl-localhost --property bio-ssl-localhost.https.port=7444 %TC_INSTANCE_NAME%