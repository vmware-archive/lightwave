#!/bin/bash

cat /etc/hosts

PROJECT_ROOT=$(pwd)

retVal=0

#unit tests

unitTestsRetVal=0
unitTestsCurrentRet=0
cd "$PROJECT_ROOT/vmidentity/rest"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/rest"
    unitTestsRetVal=1
fi

# Integration tests

# OIDC Client
cd "$PROJECT_ROOT/vmidentity/openidconnect/client/"
echo mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$1 -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$1 -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
oidcClientRetVal=$?

# REST Idm Client
cd "$PROJECT_ROOT/vmidentity/rest/idm/client/"
echo verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$1 -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$1 -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
restClientRetVal=$?

if [ ! $unitTestsRetVal -eq 0 ]; then
    echo "Error: vmidentity unit tests failed."
    retVal=1
fi

if [ ! $oidcClientRetVal -eq 0 ]; then
    echo "Error: vmidentity/openidconnect/client/"
    cat "$PROJECT_ROOT/vmidentity/openidconnect/client/target/failsafe-reports/failsafe-summary.xml"
    retVal=1
fi

if [ ! $restClientRetVal -eq 0 ]; then
    echo "Error: vmidentity/rest/idm/client/"
    cat "$PROJECT_ROOT/vmidentity/rest/idm/client/target/failsafe-reports/failsafe-summary.xml"
    retVal=1
fi

if [ ! $retVal -eq 0 ]; then
   echo --------FAILURE----------
else
   echo --------SUCCESS----------
fi

exit $retVal

