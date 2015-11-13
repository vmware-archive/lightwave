rem ---------------------------------------------------------------------------
rem Configure STS
rem
rem Copyright (c) 2015 VMware, Inc.  All rights reserved.
rem ---------------------------------------------------------------------------

Set JAVA_HOME=C:\Program Files\VMware\vCenter Server\jre
Set WEBAPPS_PATH="C:\Program Files\VMware\vCenter Server\VMware Identity Services\Tomcat\webapps"
set TC_INSTANCE_ROOT="C:\Program Files\VMware\vCenter Server\tcServer"
set TC_INSTANCE_NAME=VMwareSTSService
Set TC_INSTANCE_BASE=%TC_INSTANCE_ROOT%\%TC_INSTANCE_NAME%
set TOMCAT_INSTALL="C:\Program Files\VMware\vCenter Server\VMware Identity Services\Tomcat"

call:init

:init

xcopy /E /Y /I %TOMCAT_INSTALL%\temp\bio-custom %TC_INSTANCE_ROOT%\templates\bio-custom
xcopy /E /Y /I %TOMCAT_INSTALL%\temp\bio-ssl-localhost %TC_INSTANCE_ROOT%\templates\bio-ssl-localhost

cd C:\ProgramData\VMware\vCenterServer\runtime

%TC_INSTANCE_ROOT%\tcruntime-instance.bat create -t bio-custom --property bio-custom.http.port=7080 --force -t bio-ssl-localhost --property bio-ssl-localhost.https.port=7444 %TC_INSTANCE_NAME%

xcopy /E /Y /I %WEBAPPS_PATH% %TC_INSTANCE_BASE%\webapps\

