#!/bin/bash

cat /etc/hosts

PROJECT_ROOT=$(pwd)

retVal=0

#unit tests
unitTestsRetVal=0
unitTestsCurrentRet=0
cd "$PROJECT_ROOT/vmidentity/commons"
printf "\n=============================================\n"
printf "\n=    Unit tests vmidentity/commons          =\n"
printf "\n=============================================\n"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/commons"
    unitTestsRetVal=1
fi

unitTestsCurrentRet=0
cd "$PROJECT_ROOT/vmidentity/diagnostics"
printf "\n=============================================\n"
printf "\n=    Unit tests vmidentity/diagnostics      =\n"
printf "\n=============================================\n"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/diagnostics"
    unitTestsRetVal=1
fi

unitTestsCurrentRet=0
cd "$PROJECT_ROOT/vmidentity/samlauthority"
printf "\n=============================================\n"
printf "\n=    Unit tests vmidentity/samlauthority    =\n"
printf "\n=============================================\n"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/samlauthority"
    unitTestsRetVal=1
fi

unitTestsCurrentRet=0
cd "$PROJECT_ROOT/vmidentity/rest"
printf "\n=============================================\n"
printf "\n=    Unit tests vmidentity/rest             =\n"
printf "\n=============================================\n"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/rest"
    unitTestsRetVal=1
fi

unitTestsCurrentRet=0
cd "$PROJECT_ROOT/vmidentity/openidconnect"
printf "\n=============================================\n"
printf "\n=    Unit tests vmidentity/openidconnect    =\n"
printf "\n=============================================\n"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/openidconnect"
    unitTestsRetVal=1
fi

unitTestsCurrentRet=0
cd "$PROJECT_ROOT/vmidentity/idm/server"
printf "\n=============================================\n"
printf "\n=    Unit tests vmidentity/idm/server       =\n"
printf "\n=============================================\n"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/idm/server"
    unitTestsRetVal=1
fi

unitTestsCurrentRet=0
cd "$PROJECT_ROOT/vmidentity/sts"
printf "\n=============================================\n"
printf "\n=    Unit tests vmidentity/sts              =\n"
printf "\n=============================================\n"
ln -sf "$PROJECT_ROOT/vmidentity/service/src/main/webapp/WEB-INF" "$PROJECT_ROOT/vmidentity/sts/src/test/resources/WEB-INF"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/sts"
    unitTestsRetVal=1
fi
rm "$PROJECT_ROOT/vmidentity/sts/src/test/resources/WEB-INF"

unitTestsCurrentRet=0
cd "$PROJECT_ROOT/vmidentity/wsTrustClient"
printf "\n=============================================\n"
printf "\n=    Unit tests vmidentity/wsTrustClient    =\n"
printf "\n=============================================\n"
echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
unitTestsCurrentRet=$?
if [ ! $unitTestsCurrentRet -eq 0 ]; then
    echo "Error: unit tests in vmidentity/wsTrustClient"
    unitTestsRetVal=1
fi

# Integration tests
oidcClientRetVal=0
restClientRetVal=0
idmServerRetVal=0

if [[ ! -z $1 ]]; then
    # IDM Server
    cd "$PROJECT_ROOT/vmidentity/idm/server/"
    printf "\n=============================================\n"
    printf "\n= Integration tests idm/server              =\n"
    printf "\n=============================================\n"

    echo mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$1 -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$1 -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    idmServerRetVal=$?

    # OIDC Client
    cd "$PROJECT_ROOT/vmidentity/openidconnect/client/"
    printf "\n=============================================\n"
    printf "\n= Integration tests openidconnect/client    =\n"
    printf "\n=============================================\n"
    echo mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$1 -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$1 -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    oidcClientRetVal=$?

    # REST Idm Client
    cd "$PROJECT_ROOT/vmidentity/rest/idm/client/"
    printf "\n=============================================\n"
    printf "\n= Integration tests rest/idm/client         =\n"
    printf "\n=============================================\n"
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

