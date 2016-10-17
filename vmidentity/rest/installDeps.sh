#!/usr/bin/env bash

VERSION=6.6.1
GROUP_BASE=com.vmware.identity
PACKAGE_BASE=../../vmidentity/build/vmware-sts/packages


deps=("vmware-identity-idm-client" "vmware-identity-idm-interface" "vmware-identity-diagnostics" "samltoken")

for i in "${deps[@]}"
do
  mvn install:install-file -Dfile=$PACKAGE_BASE/$i.jar -DgroupId=$GROUP_BASE \
                           -DartifactId=$i -Dversion=$VERSION -Dpackaging=jar
done

AUTHENTICATION_FRAMEWORK=../../vmafd/build/authentication-framework/packages/

afd_deps=("client-domain-controller-cache" "vmware-endpoint-certificate-store")

echo "auth rrot : ${AUTHENTICATION_FRAMEWORK}"

for i in "${afd_deps[@]}"
do
  mvn install:install-file -Dfile=$AUTHENTICATION_FRAMEWORK/$i.jar \
                           -DgroupId=$GROUP_BASE -DartifactId=$i -Dversion=$VERSION -Dpackaging=jar
done
