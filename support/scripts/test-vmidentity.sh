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

cd "$PROJECT_ROOT/vmidentity/openidconnect"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/openidconnect"
    unitTestsRetVal=1
fi

cd "$PROJECT_ROOT/vmidentity/idm/server"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/idm/server"
    unitTestsRetVal=1
fi

# Integration tests
oidcClientRetVal=0
restClientRetVal=0
idmServerRetVal=0

if [[ ! -z $1 ]]; then
    # IDM Server
    cd "$PROJECT_ROOT/vmidentity/idm/server/"
    echo mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$1 -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$1 -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    idmServerRetVal=$?

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
fi

if [ ! $unitTestsRetVal -eq 0 ]; then
    printf "\nError: vmidentity unit tests failed.\n"
    retVal=1
fi

if [ ! $idmServerRetVal -eq 0 ]; then
    printf "\nError: vmidentity/idm/server/\n"
    cat "$PROJECT_ROOT/vmidentity/idm/server/target/failsafe-reports/failsafe-summary.xml"
    retVal=1
fi

if [ ! $oidcClientRetVal -eq 0 ]; then
    printf "\nError: integration tests: vmidentity/openidconnect/client/\n"
    cat "$PROJECT_ROOT/vmidentity/openidconnect/client/target/failsafe-reports/failsafe-summary.xml"
    retVal=1
fi

if [ ! $restClientRetVal -eq 0 ]; then
    printf "\nError: integration tests: vmidentity/rest/idm/client/\n"
    cat "$PROJECT_ROOT/vmidentity/rest/idm/client/target/failsafe-reports/failsafe-summary.xml"
    retVal=1
fi

if [ ! $retVal -eq 0 ]; then
   printf "\n--------FAILURE----------\n"
else
   printf "\n--------SUCCESS----------\n"
fi

exit $retVal

