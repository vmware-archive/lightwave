@echo off
set VMWARE_IDENTITY_SERVICES_HOME=%VMWARE_CIS_HOME%\VMware Identity Services
echo ***** Loading all the necessary jars from directory : %VMWARE_IDENTITY_SERVICES_HOME% *****
setLocal EnableDelayedExpansion
set CLASSPATH="
 for /R "%VMWARE_IDENTITY_SERVICES_HOME%" %%a in (*.jar) do (
   set CLASSPATH=!CLASSPATH!%%a;
 )
set CLASSPATH=!CLASSPATH!%VMWARE_CIS_HOME%\vmware-sso\commonlib\commons-cli-1.2.jar"
echo ***** Loaded JARs successfully *****
"%VMWARE_JAVA_HOME%\bin\java" -cp %CLASSPATH% ^
    -Dlog4j.configurationFile=file://"%VMWARE_IDENTITY_SERVICES_HOME%"\ssoconfig.log4j2.xml ^
    -Dvmware.log.dir=%VMWARE_LOG_HOME%\sso\ ^
    com.vmware.identity.ssoconfig.SSOConfigCommand %*
