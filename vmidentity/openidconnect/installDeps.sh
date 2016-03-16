#!/usr/bin/env bash

VERSION=6.0.0
PACKAGE_BASE=../../vmidentity/build/vmware-sts/packages

GROUP_BASE=com.vmware.identity.idm
deps=("vmware-identity-idm-client" "vmware-identity-idm-interface")

for i in "${deps[@]}"
do
  mvn install:install-file -Dfile=$PACKAGE_BASE/$i.jar \
                           -DgroupId=$GROUP_BASE \
                           -DartifactId=$i \
                           -Dversion=$VERSION \
                           -Dpackaging=jar
done

GROUP_BASE=com.vmware.identity
deps=("vmware-identity-diagnostics")

for i in "${deps[@]}"
do
  mvn install:install-file -Dfile=$PACKAGE_BASE/$i.jar \
                           -DgroupId=$GROUP_BASE \
                           -DartifactId=$i \
                           -Dversion=$VERSION \
                           -Dpackaging=jar
done

GOBUILD_RD_AUTHENTICATION_FRAMEWORK=$(find ../../build/gobuild/compcache/rd-authentication-framework/* -type d -prune -exec ls -d {} \; |tail -1)

mvn install:install-file -Dfile=$GOBUILD_RD_AUTHENTICATION_FRAMEWORK/publish/lib64/client-domain-controller-cache.jar \
                         -DgroupId=$GROUP_BASE \
                         -DartifactId=client-domain-controller-cache \
                         -Dversion=$VERSION \
                         -Dpackaging=jar

mvn install:install-file -Dfile=$GOBUILD_RD_AUTHENTICATION_FRAMEWORK/publish/lib64/vmware-endpoint-certificate-store.jar \
                         -DgroupId=$GROUP_BASE \
                         -DartifactId=vmware-endpoint-certificate-store \
                         -Dversion=$VERSION \
                         -Dpackaging=jar
