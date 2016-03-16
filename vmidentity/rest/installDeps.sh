#!/usr/bin/env bash

VERSION=6.0.0
GROUP_BASE=com.vmware.identity
PACKAGE_BASE=../../vmidentity/build/vmware-sts/packages


deps=("vmware-identity-idm-client" "vmware-identity-idm-interface" "vmware-identity-diagnostics" "samltoken")

for i in "${deps[@]}"
do
  mvn install:install-file -Dfile=$PACKAGE_BASE/$i.jar -DgroupId=$GROUP_BASE \
                           -DartifactId=$i -Dversion=$VERSION -Dpackaging=jar
done

GOBUILD_RD_AUTHENTICATION_FRAMEWORK=$(find ../../build/gobuild/compcache/rd-authentication-framework/* -type d -prune -exec ls -d {} \; |tail -1)

afd_deps=("client-domain-controller-cache" "vmware-endpoint-certificate-store")

for i in "${afd_deps[@]}"
do
  mvn install:install-file -Dfile=$GOBUILD_RD_AUTHENTICATION_FRAMEWORK/publish/lib64/$i.jar \
                           -DgroupId=$GROUP_BASE -DartifactId=$i -Dversion=$VERSION -Dpackaging=jar
done
