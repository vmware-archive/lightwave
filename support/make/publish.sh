#!/bin/sh

display_usage() {
    echo "Publish Lightwave artifacts to a specified Maven repository." 
    echo "Usage: ./publish.sh repo_url artifact_version"
    echo "For example: ./publish.sh https://repohost/dir 1.0.0" 
    } 

if [ $# -le 1 ]
    then
        display_usage
        exit 1
fi

if [[ ( $1 == "--help") || $1 == "-h" ]]
    then
        display_usage
        exit 0
fi

REPO_URL=$1
ARTIFACT_VERSION=$2
REPO_ID=lightwave

#publish non-native maven projects, version number in pom.xml files need to be manually changed
artifactList=(
"com.vmware.identity diagnostics ../../vmidentity/build/vmware-sts/packages/vmware-identity-diagnostics.jar ../../vmidentity/diagnostics/pom.xml" \
"com.vmware.identity.idm vmware-identity-idm-client ../../vmidentity/build/vmware-sts/packages/vmware-identity-idm-client.jar ../../vmidentity/idm/client/pom.xml" \
"com.vmware.identity.idm vmware-identity-idm-interface ../../vmidentity/build/vmware-sts/packages/vmware-identity-idm-interface.jar ../../vmidentity/idm/interface/pom.xml" \
"com.vmware.identity platform ../../vmidentity/build/vmware-sts/packages/vmware-identity-platform.jar ../../vmidentity/platform/pom.xml" \
"com.vmware.identity samltoken ../../vmidentity/build/vmware-sts/packages/samltoken.jar ../../vmidentity/commons/samltoken/pom.xml" \
"com.vmware.identity vmware-identity-websso-client ../../vmidentity/build/vmware-sts/packages/vmware-identity-websso-client.jar ../../vmidentity/ssolib/pom.xml" \
"com.vmware.identity wsTrustClient ../../vmidentity/build/vmware-sts/packages/vmware-identity-wsTrustClient.jar ../../vmidentity/wsTrustClient/pom.xml" \
"com.vmware.identity authentication-framework ../../vmafd/build/authentication-framework/packages/authentication-framework.jar" \
"com.vmware.vmafd client-domain-controller-cache ../../vmafd/build/authentication-framework/packages/client-domain-controller-cache.jar ../../vmafd/interop/java/cdc/pom.xml" \
"com.vmware.identity vmware-endpoint-certificate-store  ../../vmafd/build/authentication-framework/packages/vmware-endpoint-certificate-store.jar ../../vmafd/interop/java/vks/pom.xml"
"com.vmware.identity vmware-vmca-client ../../vmca/build/packages/vmware-vmca-client.jar" \
"com.vmware.identity vmware-identity-depends ../../vmidentity/build/vmware-sts/packages/vmware-identity-depends.jar" \
)

for i in "${artifactList[@]}"; do
    artifact=($i)
    if [ ${#artifact[@]} -eq 3 ]
    then
        mvn deploy:deploy-file \
        -DgroupId=${artifact[0]} \
        -DartifactId=${artifact[1]} \
        -Dversion=$ARTIFACT_VERSION \
        -DrepositoryId=$REPO_ID \
        -Dfile=${artifact[2]} \
        -Durl=$REPO_URL
    elif [ ${#artifact[@]} -eq 4 ]
    then
        mvn deploy:deploy-file \
        -DgroupId=${artifact[0]} \
        -DartifactId=${artifact[1]} \
        -Dversion=$ARTIFACT_VERSION \
        -DrepositoryId=$REPO_ID \
        -Dfile=${artifact[2]} \
        -DpomFile=${artifact[3]} \
        -Durl=$REPO_URL
    else
        echo "artifactList is not in correct format."
    fi
done


#publish native maven projects
mvn -f ../../vmidentity/openidconnect/common/pom.xml deploy
mvn -f ../../vmidentity/openidconnect/client/pom.xml deploy
mvn -f ../../vmidentity/openidconnect/sample/pom.xml deploy
mvn -f ../../vmidentity/openidconnect/protocol/pom.xml deploy
mvn -f ../../vmidentity/rest/pom.xml deploy

