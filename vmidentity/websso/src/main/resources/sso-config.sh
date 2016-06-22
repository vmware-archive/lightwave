#!/bin/sh

JAVA_HOME="/usr/java/jre-vmware"
JAVA_OPTS="-Xss1m -Xmx128m"
JAVA_BIN="$JAVA_HOME/bin/java"

PREFIX=/opt/vmware/
_VMIDENTITY_LIBDIR=$PREFIX/lib64
_SAMLTOKEN=/usr/lib/vmware-sso/commonlib/samltoken.jar
CLASSPATH=$_VMIDENTITY_LIBDIR/*:$_SAMLTOKEN:.:*

unset JAVA_TOOL_OPTIONS

$JAVA_BIN $JAVA_OPTS     \
          -Dlog4j.configurationFile=file://$PREFIX/share/config/ssoconfig.log4j2.xml \
          -Dvmware.log.dir=/var/log/vmware/sso \
          -cp $CLASSPATH \
          com.vmware.identity.ssoconfig.SsoConfig "$@"
