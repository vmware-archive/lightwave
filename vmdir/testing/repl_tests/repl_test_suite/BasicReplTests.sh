#!/bin/bash
#Input:
#    1) Domain name (example: lightwave.local)
#    2) Passowrd
#Functionality:
#    1) Extract all the nodes in the federation into HOST_FILE (serves as input for other script files)
#    2) This script acts as the test driver, for basic replication tests
#    3) launch DITCheckOnAllNodes and IntegrityCheckOnAllNodes
#    4) Perform I/O's verify whether change has converged across federation
#    5) launch DITCheckOnAllNodes and IntegrityCheckOnAllNodes again
#Output:
#    - Generate the results in BasicReplTestResults file
#Usage:
#    1) ./BasicJoinReplTests.sh "lightwave.local" "xxxx"
DIT_IOGENERATED="DIT/IOGenerated_0/"

#populate admin account
export LW_DOMAIN_NAME=$1
export PASSWD=$2

if [ -z "$LW_DOMAIN_NAME" ] || [ -z "$PASSWD" ]; then
    echo "Please provide domain name and password."
    echo "./BasicJoinReplTests.sh lightwave.local xxxx "
    exit 1
fi

export HOST_FILE="nodes"
export PORT=389
export ADMIN="cn=administrator,cn=users,"
export FINAL_RESULT="BasicReplTestResults"

for i in `echo $LW_DOMAIN_NAME | sed  -e "s|\.| |"`;
do
    if [ -z $TEMP_DOMAIN ]; then
        TEMP_DOMAIN=dc=$i
    else
        TEMP_DOMAIN=$TEMP_DOMAIN,dc=$i
fi
done

export DOMAIN="$TEMP_DOMAIN"

export ADMIN_DN=$ADMIN$DOMAIN
export ADMIN_UPN="administrator@$LW_DOMAIN_NAME"

mkdir -p $DIT_IOGENERATED

function writeToLogAndResult
{
    CONTENT=$1
    echo "$CONTENT" >> $FINAL_RESULT
}

function createPrologue
{
    #Legend for logs created
    writeToLogAndResult ""
    writeToLogAndResult "Basic replication test logs: "
    writeToLogAndResult "    DIT check results are present inside DIT folder"
    writeToLogAndResult "        ./DIT/DITContents_0 - one file per <node> containing all the entries present in node"
    writeToLogAndResult "        ./DIT/DITResults_0 - contains the diff between <node1> with all other nodes in the federation"
    writeToLogAndResult "        ./DIT/DITContents_1 - one file per <node> containing all the entries present in node after I/O"
    writeToLogAndResult "        ./DIT/DITResults_1 - contains the diff between <node1> with all other nodes after I/O"
    writeToLogAndResult "        ./DIT/IOGenerated - contains the LDAP operations triggered on corresponding node"
    writeToLogAndResult "    Integrity Check results are present inside IntegrityCheck folder"

    #Operations peformed during test
    writeToLogAndResult ""
    writeToLogAndResult "Basic replication test coverage:"
    writeToLogAndResult "    Perform DIT entry count is the same on all the nodes in the federation"
    writeToLogAndResult "    Perform DIT entries (DN) are the same on all the nodes in the federation"
    writeToLogAndResult "    Perform simple LDAP operations - add, modify, delete"
    writeToLogAndResult "    Verify change has converged in the federation"
    writeToLogAndResult "    Again perform DIT entry count is the same on all the nodes in the federation"
    writeToLogAndResult "    Again Perform DIT entries (DN) are the same on all the nodes in the federation"

    writeToLogAndResult ""
    writeToLogAndResult ""
    writeToLogAndResult "Basic replication test results: "
}


function main
{

    createPrologue

    #Get all nodes in the federation
    ./ExtractNodes.sh

    #Perform DIT and Integrity check
    ./DITCheckOnAllNodes.sh "" "0"
    ./IntegrityCheckOnAllNodes.sh "" "IntegrityCheck/"

    #Generate IO's
    START_VALUE=1
    NO_OF_USERS=500
    MODIFY_COUNT=50

    writeToLogAndResult ""
    writeToLogAndResult "Perform IO:"

    while IFS='' read -r line || [[ -n "$line" ]]; do
        ./LdapAddModify.sh "$line" $START_VALUE $NO_OF_USERS $MODIFY_COUNT > "$DIT_IOGENERATED$line"
        ./LdapDelete.sh  "$line" $START_VALUE $MODIFY_COUNT >> "$DIT_IOGENERATED$line"

        #Check whether delete has converged across the federation
        USER="newuser$((START_VALUE+MODIFY_COUNT-1))"
        USER_DN="cn=$USER,cn=users,$DOMAIN"
        writeToLogAndResult ""
        writeToLogAndResult "Delete: $USER_DN - Check whether change has converged across the federation"
        B_RESULT=$(./SearchForEntry.sh "$USER_DN" "$USER" "DoesNotExists" $FINAL_RESULT)
        if [[ "$B_RESULT" == "true" ]]; then
            exit 1
        fi

        START_VALUE=$(( $START_VALUE + $NO_OF_USERS ))
        #Check whether add has converged across the federation

        USER="newuser$((START_VALUE-1))"
        USER_DN="cn=$USER,cn=users,$DOMAIN"
        writeToLogAndResult ""
        writeToLogAndResult "Add: $USER_DN - Check whether change has converged across the federation"
        B_RESULT=$(./SearchForEntry.sh "$USER_DN" "$USER" "Exists" $FINAL_RESULT)
        if [[ "$B_RESULT" == "true" ]]; then
            exit 1
        fi
    done < "$HOST_FILE"

    writeToLogAndResult ""
    writeToLogAndResult "After performing IO's"

    #Perform DIT check and IntegrityCheck After IO's
    ./DITCheckOnAllNodes.sh "" "1"
    ./IntegrityCheckOnAllNodes.sh "" "IntegrityCheck/AfterIO/"

    writeToLogAndResult ""
    writeToLogAndResult "done"
}

#perform basic replication tests
main
