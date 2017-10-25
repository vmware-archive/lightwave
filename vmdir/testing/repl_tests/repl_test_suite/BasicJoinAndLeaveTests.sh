#!/bin/bash
#Input:
#    1) Domain name (example: lightwave.local)
#    2) Password
#Functionality:
#   1) Perform leave federation
#         - Perform some LDAP operations on the node which leaves the federation
#         - Perform ldapsearch and check whether last change has converged
#         - Verify node removed from the federation is not part of any node's replication agreements
#         - Server object tree of the node removed from the federation deleted
#         - Perform DIT check and Integrity check on all the nodes in the federation
#   2) Perform join test
#        - Perform some LDAP operations on the node which joined the federation
#        - Perform ldapsearch and check whether last change has converged
#        - Verify whether domain controller entry for node joined the federation is added
#        - Verify node joined to the federation is part of replication agreements and replication agreement is replicated
#        - Verify node joined to the federation is part of federation's server object tree
#        - Perform DIT check and Integrity check on all the nodes in the federation
#Usage:
#   ./BasicJoinAndLeaveTests.sh "lightwave.local" "xxx"

#export necessary variables  needed for this script and scripts called by this script
#populate admin account
export LW_DOMAIN_NAME=$1
export PASSWD=$2

if [ -z "$LW_DOMAIN_NAME" ] || [ -z "$PASSWD" ]; then
    echo "Please provide domain name and password."
    echo "./BasicJoinReplTests.sh lightwave.local xxxx "
    exit 1
fi

export FINAL_RESULT="BasicJoinAndLeaveTestResults"
export HOST_FILE="nodes"
export PORT=389
export ADMIN="cn=administrator,cn=users,"
export NO_OF_USERS=500
export MODIFY_COUNT=50

for i in `echo $LW_DOMAIN_NAME | sed  -e "s|\.| |"`;
do
    if [ -z $TEMP_DOMAIN ]; then
        TEMP_DOMAIN=dc=$i
    else
        TEMP_DOMAIN=$TEMP_DOMAIN,dc=$i
fi
done

export DOMAIN="$TEMP_DOMAIN"

export ADMIN_DN="$ADMIN$DOMAIN"
export DC_DN="ou=domain controllers,$DOMAIN"
export SITES_DN="cn=sites,cn=configuration,$DOMAIN"

NODE_LEAVE_FED=$(hostname)
export NODE_LEAVE_FED="$NODE_LEAVE_FED.$LW_DOMAIN_NAME"
export ADMIN_UPN="administrator@$LW_DOMAIN_NAME"

#Variable declarations
declare -a HOST_ARR

#variable definitions
ITERATION_COUNT=2
NODE_COUNT=0
START_VALUE=0

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
    writeToLogAndResult "        ./DIT/DITContents_0 - one file per <node> containing all the entries present in node after leave"
    writeToLogAndResult "        ./DIT/DITResults_0 - contains the diff between <node1> with all other nodes in the federation after leave"
    writeToLogAndResult "        ./DIT/DITContents_1 - one file per <node> containing all the entries present in node after join"
    writeToLogAndResult "        ./DIT/DITResults_1 - contains the diff between <node1> with all other nodes after join"
    writeToLogAndResult "         Number present at the end of DITContents_x, x - denotes the exec count (x is even denotes leave test, odd denotes join test)"
    writeToLogAndResult "        ./IO/JoinTest/ - contains the LDAP operations triggered on node during join test"
    writeToLogAndResult "        ./IO/LeaveTest/ - contains the LDAP operations triggered on node during leave test"
    writeToLogAndResult "    Integrity Check results are present inside IntegrityCheck folder"

    #Operations peformed during test
    writeToLogAndResult ""
    writeToLogAndResult "Basic replication join and leave coverage:"
    writeToLogAndResult "Perform leave federation"
    writeToLogAndResult "    Perform some LDAP operations on the node which leaves the federation"
    writeToLogAndResult "    Perform ldapsearch and check whether last change has converged"
    writeToLogAndResult "    Verify node removed from the federation is not part of any node's replication agreements"
    writeToLogAndResult "    Server object tree of the node removed from the federation deleted"
    writeToLogAndResult "    Perform DIT check and Integrity check on all the nodes in the federation"
    writeToLogAndResult "Perform join test"
    writeToLogAndResult "    Perform some LDAP operations on the node which joined the federation"
    writeToLogAndResult "    Perform ldapsearch and check whether last change has converged"
    writeToLogAndResult "    Verify whether domain controller entry for node joined the federation is added"
    writeToLogAndResult "    Verify node joined to the federation is part of replication agreements and replication agreement is replicated"
    writeToLogAndResult "    Verify node joined to the federation is part of federation's server object tree"
    writeToLogAndResult "    Perform DIT check and Integrity check on all the nodes in the federation"

    writeToLogAndResult ""
    writeToLogAndResult ""
    writeToLogAndResult "Basic replication join and leave results: "
}

function main
{

    #create prologue
    createPrologue

    #Get all nodes in the federation
    ./ExtractNodes.sh

    #Extract the HOST file to HOST_ARR and get NODE_COUNT
    while IFS='' read -r line || [[ -n "$line" ]]; do
        HOST_ARR+=("$line")
        NODE_COUNT=$(( $NODE_COUNT + 1 ))
    done < "$HOST_FILE"

    #Get partner RA DN - DSE Root search and find the server name
    PARTNER_RA_DN=$(ldapsearch -h localhost -LLL -o ldif-wrap=no -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD -b "" -s base | grep serverName)
    PARTNER_RA_DN="$(echo $PARTNER_RA_DN | sed 's/serverName: //g')"

    #From the replication agreement find the corresponding partner node
    for instance in "${HOST_ARR[@]}"
    do
        if [[ "$instance" == "$NODE_LEAVE_FED" ]]; then
            continue;
        fi

        if [[ ! -z $(ldapsearch -h localhost -LLL -o ldif-wrap=no -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD -b "$PARTNER_RA_DN" -s sub dn | grep $instance) ]]; then
            MASTER_NODE="$instance"
            MASTER_RA_DN=$(ldapsearch -h $instance -LLL -o ldif-wrap=no -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD  -b "" -s base | grep serverName)
            break
        fi
    done

    #check if MASTER_RA_DN is NULL
    if [[ -z "${MASTER_RA_DN// }" ]]; then
        exit 1
    fi

    #Get Master RA DN - DSE Root search and find the server name
    MASTER_RA_DN="$(echo $MASTER_RA_DN | sed 's/serverName: //g')"

    #Update Start value
    START_VALUE=$(($NO_OF_USERS * $NODE_COUNT))
    START_VALUE=18000

    #Perform below specified tests for 10 iterations
    for ((x = 0; x < ITERATION_COUNT; x++))
    do
        #Update Start value
        START_VALUE=$(($START_VALUE + $NO_OF_USERS))

        #Perform leave federation and tests
        n=$((x%2))
        if [ $n -eq 0 ]; then
            writeToLogAndResult ""
            writeToLogAndResult "Performing Leave federation tests, exec count: $x"
           ./LeaveFederationTests.sh "$START_VALUE" "$x"
        else
            writeToLogAndResult ""
            writeToLogAndResult "Performing Join federation tests, exec count: $x"
            ./JoinFederationTests.sh "$START_VALUE" "$x" "$MASTER_NODE" "$PARTNER_RA_DN" "$MASTER_RA_DN"
    fi
    done
}

# Start perfomring join and leave replication tests
main
