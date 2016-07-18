#!/bin/bash
INSTALL_PATH=/opt/vmware/lib64/
cd $INSTALL_PATH
LOG_CONFIG=/usr/lib/vmidentity/install/installer-log4j.properties
JAVA_SECURITY_FILE=$JAVA_SECURITY_FILEPATH/$JAVA_SECURITY_FILE
$VMWARE_JAVA_HOME/bin/java -Dinstall.log.file=/var/log/vmware/sso/Install-upgrade.log \
    -Dlog4j.configuration=file:$LOG_CONFIG \
    -cp *:vmware-identity-install.jar -cp vmware-identity-install.jar com.vmware.identity.configure.VMIdentityStandaloneInstaller \
    --backup-folder $1 --identity-conf-file-path $2 --upgrade