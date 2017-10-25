#!/bin/bash
#Input:
#    - START_VALUE - used for generating IO
#    - EXEC_COUNT - used to create better logs
#Functionality:
#   1) Perform leave federation
#         - Perform some LDAP operations on the node which leaves the federation
#         - Perform ldapsearch and check whether last change has converged
#         - Verify node removed from the federation is not part of any node's replication agreements
#         - Server object tree of the node removed from the federation deleted
#         - Perform DIT check and Integrity check on all the nodes in the federation
#Usage:
#   ./LeaveFederationTests.sh "100" "1"
#   This script makes use of some exported variables, check BasicJoinAndLeaveTests.sh

#variable definitions
START_VALUE=$1
EXEC_COUNT=$2
IOGENERATED="IO/LeaveTest/"

#create directories if required
mkdir -p "$IOGENERATED"

function writeToLogAndResult
{
    CONTENT=$1
    echo "$CONTENT" >> $FINAL_RESULT
}

#Leave federation tests
function LeaveFederationTests
{
    #Perform some LDAP operations on the node which leaves the federation
    writeToLogAndResult "Perform Ldap Add and Modify operations"
    ./LdapAddModify.sh "$NODE_LEAVE_FED" $START_VALUE $NO_OF_USERS $MODIFY_COUNT > "$IOGENERATED$NODE_LEAVE_FED$EXEC_COUNT" 2>&1 &

    #Update Start value
    START_VALUE=$(($START_VALUE + $NO_OF_USERS))

     writeToLogAndResult ""
     writeToLogAndResult "Demoting vmdir"
    /opt/vmware/bin/vmafd-cli demote-vmdir --user-name administrator --password $PASSWD

    #Perform ldapsearch and check whether last change has converged
    writeToLogAndResult ""
    writeToLogAndResult "Check whether last change on demoted node has converged in the federation"
    journalctl | grep vmdird | grep "cn=users,$DOMAIN" > temp-vmdird.log
    sed -n -i '$p' temp-vmdird.log
    sed -i 's/^.*\(cn=newuser.*'$DOMAIN'\).*$/\1/' temp-vmdird.log
    USER_DN=$(cat temp-vmdird.log)
    IFS=',' read -ra RDN <<< "$USER_DN"
    USER_SN=$(echo "${RDN[0]}" | sed 's/^.\{3\}//')
    B_RESULT=$(./SearchForEntry.sh "$USER_DN" "$USER_SN" "Exists" $FINAL_RESULT "$NODE_LEAVE_FED")
    if [[ "$B_RESULT" == "true" ]]; then
        writeToLogAndResult "Failure: Last change "$USER_DN" on demoted node has not converged in the federation"
        break
    else
       writeToLogAndResult "Success: Last change "$USER_DN" on demoted node has converged in the federation"
    fi

    writeToLogAndResult ""
    writeToLogAndResult "Check whether domain controller entry for demoted node is removed"
    #Verify whether domain controller entry for node which left the federation is deleted
    B_RESULT=$(./SearchForEntry.sh  "$DC_DN" "$NODE_LEAVE_FED" "DoesNotExists" $FINAL_RESULT "$NODE_LEAVE_FED")
    DC_SEARCH_RESULT=$B_RESULT

    #Verify node removed from the federation is not part of any node's replication agreements
    #Verify node removed from the federation has its server object tree deleted
    writeToLogAndResult ""
    writeToLogAndResult "Check whether server object tree and replication agreements are removed from all the nodes in the federation"
    B_RESULT=$(./SearchForEntry.sh  "$SITES_DN" "$NODE_LEAVE_FED" "DoesNotExists" $FINAL_RESULT "$NODE_LEAVE_FED")
    SERVER_OBJ_TREE_RESULT=$B_RESULT
    if [[ "$DC_SEARCH_RESULT" == "true" ]] || [[ "$SERVER_OBJ_TREE_RESULT" == "true" ]] ; then
        writeToLogAndResult "Failure: Server object tree and replication agreements were not removed from all the nodes in the federation"
        break
    else
        writeToLogAndResult "Success: Server object tree and replication agreements of the node $NODE_LEAVE_FED are removed from all the nodes in the federation"
    fi

    #perform DIT check on all the nodes in the federation
    writeToLogAndResult ""
    writeToLogAndResult "Perform DIT check on all the nodes in the federation"
    ./DITCheckOnAllNodes.sh $NODE_LEAVE_FED "$EXEC_COUNT"

    #Perform Integriy check on the nodes in the federation
    writeToLogAndResult ""
    writeToLogAndResult "Perform integrity check on all the nodes in the federation"
    ./IntegrityCheckOnAllNodes.sh $NODE_LEAVE_FED "IntegrityCheck/AfterLeave_$EXEC_COUNT/"
}

#Perform leave federation test
LeaveFederationTests
