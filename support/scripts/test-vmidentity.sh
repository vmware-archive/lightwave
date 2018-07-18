#!/bin/bash

cat /etc/hosts

PROJECT_ROOT=$(pwd)

retVal=0

#unit tests
unitTestsRetVal=0
unitTestsCurrentRet=0
#integration tests
oidcClientRetVal=0
restIdmClientRetVal=0
restVmdirClientRetVal=0
idmServerRetVal=0
wsTrustClientRetVal=0

function unit_test_commons
{
    unitTestsCurrentRet=0
    cd "$PROJECT_ROOT/vmidentity/commons"
    printf "\n=============================================\n"
    printf "\n=    Unit tests vmidentity/commons          =\n"
    printf "\n=============================================\n"
    echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn test -DskipIntegrationTests=true \
             -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    unitTestsCurrentRet=$?
    if [ ! $unitTestsCurrentRet -eq 0 ]; then
        echo "Error: unit tests in vmidentity/commons"
        unitTestsRetVal=1
    fi
}

function unit_test_diagnostics
{
    unitTestsCurrentRet=0
    cd "$PROJECT_ROOT/vmidentity/diagnostics"
    printf "\n=============================================\n"
    printf "\n=    Unit tests vmidentity/diagnostics      =\n"
    printf "\n=============================================\n"
    echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn test -DskipIntegrationTests=true \
             -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    unitTestsCurrentRet=$?
    if [ ! $unitTestsCurrentRet -eq 0 ]; then
        echo "Error: unit tests in vmidentity/diagnostics"
        unitTestsRetVal=1
    fi
}

function unit_test_samlauthority
{
    unitTestsCurrentRet=0
    cd "$PROJECT_ROOT/vmidentity/samlauthority"
    printf "\n=============================================\n"
    printf "\n=    Unit tests vmidentity/samlauthority    =\n"
    printf "\n=============================================\n"
    echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn test -DskipIntegrationTests=true \
             -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    unitTestsCurrentRet=$?
    if [ ! $unitTestsCurrentRet -eq 0 ]; then
        echo "Error: unit tests in vmidentity/samlauthority"
        unitTestsRetVal=1
    fi
}

function unit_test_rest
{
    unitTestsCurrentRet=0
    cd "$PROJECT_ROOT/vmidentity/rest"
    printf "\n=============================================\n"
    printf "\n=    Unit tests vmidentity/rest             =\n"
    printf "\n=============================================\n"
    echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn test -DskipIntegrationTests=true \
             -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    unitTestsCurrentRet=$?
    if [ ! $unitTestsCurrentRet -eq 0 ]; then
        echo "Error: unit tests in vmidentity/rest"
        unitTestsRetVal=1
    fi
}

function unit_test_openidconnect
{
    unitTestsCurrentRet=0
    cd "$PROJECT_ROOT/vmidentity/openidconnect"
    printf "\n=============================================\n"
    printf "\n=    Unit tests vmidentity/openidconnect    =\n"
    printf "\n=============================================\n"
    echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn test -DskipIntegrationTests=true \
             -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    unitTestsCurrentRet=$?
    if [ ! $unitTestsCurrentRet -eq 0 ]; then
        echo "Error: unit tests in vmidentity/openidconnect"
        unitTestsRetVal=1
    fi
}

function unit_test_idm_server
{
    unitTestsCurrentRet=0
    cd "$PROJECT_ROOT/vmidentity/idm/server"
    printf "\n=============================================\n"
    printf "\n=    Unit tests vmidentity/idm/server       =\n"
    printf "\n=============================================\n"
    echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn test -DskipIntegrationTests=true \
             -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    unitTestsCurrentRet=$?
    if [ ! $unitTestsCurrentRet -eq 0 ]; then
        echo "Error: unit tests in vmidentity/idm/server"
        unitTestsRetVal=1
    fi
}

function unit_test_sts
{
    unitTestsCurrentRet=0
    cd "$PROJECT_ROOT/vmidentity/sts"
    printf "\n=============================================\n"
    printf "\n=    Unit tests vmidentity/sts              =\n"
    printf "\n=============================================\n"
    ln -sf "$PROJECT_ROOT/vmidentity/service/src/main/webapp/WEB-INF" \
           "$PROJECT_ROOT/vmidentity/sts/src/test/resources/WEB-INF"
    echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn test -DskipIntegrationTests=true \
             -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    unitTestsCurrentRet=$?
    if [ ! $unitTestsCurrentRet -eq 0 ]; then
        echo "Error: unit tests in vmidentity/sts"
        unitTestsRetVal=1
    fi
    rm "$PROJECT_ROOT/vmidentity/sts/src/test/resources/WEB-INF"
}

function unit_test_wstrustclient
{
    unitTestsCurrentRet=0
    cd "$PROJECT_ROOT/vmidentity/wsTrustClient"
    printf "\n=============================================\n"
    printf "\n=    Unit tests vmidentity/wsTrustClient    =\n"
    printf "\n=============================================\n"
    echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn test -DskipIntegrationTests=true \
             -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    unitTestsCurrentRet=$?
    if [ ! $unitTestsCurrentRet -eq 0 ]; then
        echo "Error: unit tests in vmidentity/wsTrustClient"
        unitTestsRetVal=1
    fi
}

function unit_test_websso
{
    unitTestsCurrentRet=0
    cd "$PROJECT_ROOT/vmidentity/websso"
    printf "\n=============================================\n"
    printf "\n=    Unit tests vmidentity/websso              =\n"
    printf "\n=============================================\n"
    echo mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn test -DskipIntegrationTests=true -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    unitTestsCurrentRet=$?
    if [ ! $unitTestsCurrentRet -eq 0 ]; then
        echo "Error: unit tests in vmidentity/websso"
        unitTestsRetVal=1
    fi
}

function run_all_unit_tests
{
    unit_test_commons
    unit_test_diagnostics
    unit_test_samlauthority
    unit_test_rest
    unit_test_openidconnect
    unit_test_idm_server
    unit_test_websso
    unit_test_sts
    unit_test_wstrustclient
}

function integration_test_idm_server
{
    local sts_hostname=$1

    cd "$PROJECT_ROOT/vmidentity/idm/server/"
    printf "\n=============================================\n"
    printf "\n= Integration tests idm/server              =\n"
    printf "\n=============================================\n"

    echo mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$sts_hostname -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn verify -DskipTests=true \
               -DskipIntegrationTests=false \
               -Dhost=$sts_hostname \
               -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    idmServerRetVal=$?
}

function integration_test_openidconnect_client
{
    local sts_hostname=$1

    cd "$PROJECT_ROOT/vmidentity/openidconnect/client/"
    printf "\n=============================================\n"
    printf "\n= Integration tests openidconnect/client    =\n"
    printf "\n=============================================\n"
    echo mvn verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$sts_hostname -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn verify -DskipTests=true \
               -DskipIntegrationTests=false \
               -Dhost=$sts_hostname \
               -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    oidcClientRetVal=$?
}

function integration_test_rest_idm_client
{
    local sts_hostname=$1

    cd "$PROJECT_ROOT/vmidentity/rest/idm/client/"
    printf "\n=============================================\n"
    printf "\n= Integration tests rest/idm/client         =\n"
    printf "\n=============================================\n"
    echo verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$sts_hostname -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn verify -DskipTests=true \
               -DskipIntegrationTests=false \
               -Dhost=$sts_hostname \
               -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    restIdmClientRetVal=$?
}

function integration_test_rest_vmdir_client
{
    local sts_hostname=$1

    cd "$PROJECT_ROOT/vmidentity/rest/vmdir/client/"
    printf "\n=============================================\n"
    printf "\n= Integration tests rest/vmdir/client         =\n"
    printf "\n=============================================\n"
    echo verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$sts_hostname -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
        mvn verify -DskipTests=true \
               -DskipIntegrationTests=false \
               -Dhost=$sts_hostname \
               -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    restVmdirClientRetVal=$?
}

function integration_test_wstrust_client
{
    local sts_hostname=$1

    cd "$PROJECT_ROOT/vmidentity/wsTrustClient/"
    printf "\n=============================================\n"
    printf "\n= Integration tests wsTrustClient           =\n"
    printf "\n=============================================\n"
    echo verify -DskipTests=true -DskipIntegrationTests=false -Dhost=$sts_hostname -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    mvn verify -DskipTests=true \
               -DskipIntegrationTests=false \
               -Dhost=$sts_hostname \
               -Dmaven.repo.local="$PROJECT_ROOT/build/vmidentity/repo/"
    wsTrustClientRetVal=$?
}

function run_all_integration_tests
{
    sts_hostname=$1

    integration_test_idm_server $sts_hostname
    integration_test_openidconnect_client $sts_hostname
    integration_test_rest_idm_client $sts_hostname
    integration_test_rest_vmdir_client $sts_hostname
    # integration_test_wstrust_client $sts_hostname
}

#
# Main
#

STS_HOSTNAME=
TEST_FILTER=all

if [ $# -gt 1 ]; then
    STS_HOSTNAME=$1
    TEST_FILTER=$2
elif [ $# -eq 1 ]; then
    STS_HOSTNAME=$1
fi

case $TEST_FILTER in
    all)
        run_all_unit_tests
        run_all_integration_tests $STS_HOSTNAME
        ;;
    unit-test-commons)
        unit_test_commons
        ;;
    unit-test-diagnostics)
        unit_test_diagnostics
        ;;
    unit-test-samlauthority)
        unit_test_samlauthority
        ;;
    unit-test-rest)
        unit_test_rest
        ;;
    unit-test-openidconnect)
        unit_test_openidconnect
        ;;
    unit-test-idm-server)
        unit_test_idm_server
        ;;
    unit-test-websso)
        unit_test_websso
        ;;
    unit-test-sts)
        unit_test_sts
        ;;
    unit-test-wstrustclient)
        unit_test_wstrustclient
        ;;
    integration-test-idm-server)
        integration_test_idm_server $STS_HOSTNAME
        ;;
    integration-test-openidconnect-client)
        integration_test_openidconnect_client $STS_HOSTNAME
        ;;
    integration-test-rest-idm-client)
        integration_test_rest_idm_client $STS_HOSTNAME
        ;;
    integration_test_rest_vmdir_client)
        integration_test_rest_vmdir_client $STS_HOSTNAME
        ;;
    integration-test-wstrust-client)
        integration_test_wstrust_client $STS_HOSTNAME
        ;;
    *)
        echo "Error: Invalid test filter - ($TEST_FILTER)"
        retVal=1
        ;;
esac

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

if [ ! $restIdmClientRetVal -eq 0 ]; then
    printf "\nError: integration tests: vmidentity/rest/idm/client/\n"
    cat "$PROJECT_ROOT/vmidentity/rest/idm/client/target/failsafe-reports/failsafe-summary.xml"
    retVal=1
fi

if [ ! $restVmdirClientRetVal -eq 0 ]; then
    printf "\nError: integration tests: vmidentity/rest/vmdir/client/\n"
    cat "$PROJECT_ROOT/vmidentity/rest/vmdir/client/target/failsafe-reports/failsafe-summary.xml"
    retVal=1
fi

if [ ! $wsTrustClientRetVal -eq 0 ]; then
    printf "\nError: integration tests: vmidentity/wsTrustClient/\n"
    cat "$PROJECT_ROOT/vmidentity/wsTrustClient/target/failsafe-reports/failsafe-summary.xml"
    retVal=1
fi

if [ ! $retVal -eq 0 ]; then
   printf "\n--------FAILURE----------\n"
else
   printf "\n--------SUCCESS----------\n"
fi

exit $retVal

