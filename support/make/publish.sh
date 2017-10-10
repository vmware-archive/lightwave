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
"com.vmware.vmafd authentication-framework ../../vmafd/build/authentication-framework/packages/authentication-framework.jar" \
"com.vmware.vmafd afd-heartbeat-service ../../vmafd/build/authentication-framework/packages/afd-heartbeat-service.jar" \
"com.vmware.vmafd client-domain-controller-cache ../../vmafd/build/authentication-framework/packages/client-domain-controller-cache.jar ../../vmafd/interop/java/cdc/pom.xml" \
"com.vmware.vmafd vmware-endpoint-certificate-store  ../../vmafd/build/authentication-framework/packages/vmware-endpoint-certificate-store.jar ../../vmafd/interop/java/vks/pom.xml"
"com.vmware.vmca vmware-vmca-client ../../vmca/build/certificate-authority/packages/vmware-vmca-client.jar" \
)

for i in "${artifactList[@]}"; do
    artifact=($i)
    if [ ${#artifact[@]} -eq 3 ]
    echo "Publishing Artifact : ${artifact[1]} to artifactory instance.";
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
mvn -f ../../vmidentity/pom.xml deploy -DskipTests=true

