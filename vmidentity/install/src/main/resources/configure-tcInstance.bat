rem ---------------------------------------------------------------------------
rem Configure STS
rem
rem Copyright (c) 2015 VMware, Inc.  All rights reserved.
rem ---------------------------------------------------------------------------


Set WEBAPPS_PATH="C:\Program Files\VMware\CIS\VMware Identity Services\Tomcat\webapps"
set TC_WEBAPPS_PATH="C:\ProgramData\VMware\CIS\runtime\VMwareSTSService\webapps\"
set SERVER_CERT="C:\ProgramData\VMware\CIS"

xcopy /E /Y /I %WEBAPPS_PATH% %TC_WEBAPPS_PATH%
xcopy %SERVER_CERT%\cfg\sso\keys\ssoserver.crt   %SERVER_CERT%\runtime\VMwareSTSService\conf