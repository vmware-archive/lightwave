#!/bin/bash

INSTALL_PATH=/opt/vmware/lib64/
JAVA_SECURITY_FILEPATH=/usr/lib/vmware-vmafd/share/config
JAVA_SECURITY_FILE=java.security.linux

#Log Settings
LOG_CONFIG=/usr/lib/vmidentity/install/installer-log4j.properties

echo "Enter the domain: "
read domain

echo "Enter the username: "
read username

stty -echo
echo -n "Enter Password: "
read password
stty echo


if [ -d $INSTALL_PATH ]; then
  cd $INSTALL_PATH
fi

if [ -d $JAVA_SECURITY_FILEPATH ]; then

  JAVA_SECURITY_FILE=$JAVA_SECURITY_FILEPATH/$JAVA_SECURITY_FILE

  java -Djava.security.properties=$JAVA_SECURITY_FILE \
    -Dinstall.log.file=/var/log/vmware/sso/standalone-install.log \
    -Dlog4j.configuration=file:$LOG_CONFIG \
    -cp vmware-identity-install.jar com.vmware.identity.configure.VMIdentityStandaloneInstaller \
    --domain $domain --username $username --password $password


else
  echo "Java Security file not found. Cannot load certificates from VKS."
  exit 1
fi