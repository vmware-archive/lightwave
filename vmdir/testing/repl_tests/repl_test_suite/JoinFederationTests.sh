#!/bin/bash
#Input:
#    - START_VALUE - used for generating IO
#    - EXEC_COUNT - used to create better logs
#    - MASTER_NODE - hostname of the master node
#    - PARTNER_RA_DN - Node's replication agreement path
#    - MASTER_RA_DN - Master node's replication agreement path
#Functionality:
#    - Perform some LDAP operations on the node which joined the federation
#    - Perform ldapsearch and check whether last change has converged
#    - Verify whether domain controller entry for node joined the federation is added
#    - Verify node joined to the federation is part of replication agreements and replication agreement is replicated
#    - Verify node joined to the federation is part of federation's server object tree
#    - Perform DIT check and Integrity check on all the nodes in the federation
#Usage:
#   ./JoinFederationTests.sh "200" "1" "lightwave-test.lightwave.local" "Parnter RA DN" "Master RA DN"
#    sample for RA DN - cn=lightwave-i-068bdbfe984fd1e9a.lightwave.local,cn=Servers,cn=Default-first-site,cn=Sites,cn=Configuration,dc=lightwave,dc=local
#   This script makes use of some exported variables, check BasicJoinAndLeaveTests.sh

START_VALUE=$1
EXEC_COUNT=$2
MASTER_NODE=$3
PARTNER_RA_DN=$4
MASTER_RA_DN=$5
IOGENERATED="IO/JoinTest/"

#create directories if required
mkdir -p "$IOGENERATED"

function writeToLogAndResult
{
    CONTENT=$1
    echo "$CONTENT" >> $FINAL_RESULT
}

function PerformJoinTests
{
    writeToLogAndResult ""
    writeToLogAndResult "Removing all the mdb files"
    rm /var/lib/vmware/vmdir/*.mdb
    sleep 10

    writeToLogAndResult "Restart vmdir and sleep for a minute"
    /opt/likewise/bin/lwsm restart vmdir
    sleep 1m

    writeToLogAndResult "Perform Join"
    /opt/vmware/bin/vdcpromo -u Administrator -w 'VMware123!' -H "$MASTER_NODE"

    writeToLogAndResult "Perform Ldap Add and Modify operations"
    ./LdapAddModify.sh "$NODE_LEAVE_FED" $START_VALUE $NO_OF_USERS $MODIFY_COUNT > "$IOGENERATED$NODE_LEAVE_FED$EXEC_COUNT"

    #Update Start value
    START_VALUE=$(($START_VALUE + $NO_OF_USERS))

    #Perform ldapsearch and check whether last change has converged
    writeToLogAndResult ""
    writeToLogAndResult "Check whether last change on promoted node has converged in the federation"
    USER="newuser$((START_VALUE-1))"
    USER_DN="cn=$USER,cn=users,$DOMAIN"
    B_RESULT=$(./SearchForEntry.sh "$USER_DN" "$USER" "Exists" $FINAL_RESULT)
    if [[ "$B_RESULT" == "true" ]]; then
        writeToLogAndResult "Failure: Last change "$USER_DN" on promoted node has not converged in the federation"  >> $FINAL_RESULT
        break
    else
       writeToLogAndResult "Success: Last change "$USER_DN" on promoted node has converged in the federation"  >> $FINAL_RESULT
    fi

     #Verify whether domain controller entry for node which left the federation is added
     writeToLogAndResult ""
     writeToLogAndResult "Check whether domain controller entry for the joined node is created"
     B_RESULT=$(./SearchForEntry.sh "$DC_DN" "$NODE_LEAVE_FED" "Exists" $FINAL_RESULT)
     DC_SEARCH_RESULT=$B_RESULT

     writeToLogAndResult ""
     writeToLogAndResult "Check whether server object tree for the the newly joined entry is created"
     #Verify node joined to the federation is part of replication agreements
     #Verify node joined to the federation is part of federation's server object tree
     B_RESULT=$(./SearchForEntry.sh "$SITES_DN" "$NODE_LEAVE_FED" "Exists" $FINAL_RESULT)
     SERVER_OBJ_TREE_RESULT=$B_RESULT
     if [[ "$DC_SEARCH_RESULT" == "true" ]] || [[ "$SERVER_OBJ_TREE_RESULT" == "true" ]] ; then
         writeToLogAndResult "Failure: Domain controller entry or server object tree for the the newly joined node is not present in all the nodes in the federation" >> $FINAL_RESULT
         break
     else
         writeToLogAndResult "Success: Domain controller entry or server object tree for the the newly joined node is present in all the nodes in the federation" >> $FINAL_RESULT
     fi

     writeToLogAndResult ""
     writeToLogAndResult "Check whether replication agreement has been created on the Master node"
     B_RESULT=$(./SearchForEntry.sh "$MASTER_RA_DN" "$NODE_LEAVE_FED" "Exists" $FINAL_RESULT)
     if [[ "$B_RESULT" == "true" ]]; then
         writeToLogAndResult "Failure: replication agreement for the newly joined node is not present on all nodes in the federation" >> $FINAL_RESULT
         break
     else
         writeToLogAndResult "Success: replication agreement for the newly joined node is present on all nodes in the federation" >> $FINAL_RESULT
     fi

     writeToLogAndResult ""
     writeToLogAndResult "Check whether replication agreement has been created on the Partner node"
     B_RESULT=$(./SearchForEntry.sh "$PARTNER_RA_DN" "$MASTER_NODE" "Exists" $FINAL_RESULT)
     if [[ "$B_RESULT" == "true" ]]; then
         writeToLogAndResult "Failure: replication agreement for the newly joined node is not present on all nodes in the federation" >> $FINAL_RESULT
         break
     else
         writeToLogAndResult "Success: replication agreement for the newly joined node is present on all nodes in the federation" >> $FINAL_RESULT
     fi

     #perform DIT check on all the nodes in the federation
     writeToLogAndResult ""
     writeToLogAndResult "Perform DIT check on all the nodes in the federation"
     ./DITCheckOnAllNodes.sh "NO_IO" "$EXEC_COUNT"

     #Perform Integriy check
     writeToLogAndResult ""
     writeToLogAndResult "Perform integrity check on all the nodes in the federation"
     ./IntegrityCheckOnAllNodes.sh "" "IntegrityCheck/AfterJoin_$EXEC_COUNT/"
}

#Perform join federation test
PerformJoinTests
