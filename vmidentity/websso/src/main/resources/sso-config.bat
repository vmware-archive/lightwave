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
echo Running command : java -cp %CLASSPATH% com.vmware.identity.ssoconfig.SsoConfig %*
"%VMWARE_JAVA_HOME%\bin\java" -cp %CLASSPATH% com.vmware.identity.ssoconfig.SsoConfig %*
