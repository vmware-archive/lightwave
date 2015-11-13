#!/bin/sh

JAVA_HOME="/usr/java/jre-vmware"
JAVA_OPTS="-Xss1m -Xmx128m -XX:MaxPermSize=256M"
JAVA_BIN="$JAVA_HOME/bin/java"

_VMIDENTITY_LIBDIR=/opt/vmware/lib64
_SAMLTOKEN=/usr/lib/vmware-sso/commonlib/samltoken.jar
CLASSPATH=$_VMIDENTITY_LIBDIR/*:$_SAMLTOKEN:.:*

unset JAVA_TOOL_OPTIONS

$JAVA_BIN $JAVA_OPTS     \
          -cp $CLASSPATH \
          com.vmware.identity.ssoconfig.SsoConfig "$@"
