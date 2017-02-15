#!/bin/sh

JAVA_OPTS="-Xss1m -Xmx128m"
JAVA_BIN="$JAVA_HOME/bin/java"

if rpm -qa | grep -q vmware-identity-sts; then
  echo "Running SSO config Tool against Vsphere release"
  VMIDENTITY_LIB_DIR=/opt/vmware/lib64
  SAMLTOKEN_JAR_DIR=/usr/lib/vmware-sso/commonlib/samltoken.jar
  LOG_CONFIG=$VMIDENTITY_LIB_DIR/../share/config/ssoconfig.log4j2.xml
else
 echo "Running SSO config Tool against Lightwave release"
 VMIDENTITY_LIB_DIR=/opt/vmware/jars
 LOG_CONFIG=$VMIDENTITY_LIB_DIR/../share/config/idm/ssoconfig.log4j2.xml
fi

CLASSPATH=$VMIDENTITY_LIB_DIR/*:$SAMLTOKEN_JAR_DIR:.:*

#unset JAVA_TOOL_OPTIONS

$JAVA_BIN $JAVA_OPTS     \
          -Dlog4j.configurationFile=$LOG_CONFIG \
          -Dvmware.log.dir=/var/log/vmware/sso \
          -cp $CLASSPATH \
com.vmware.identity.ssoconfig.SSOConfigCommand "$@"
