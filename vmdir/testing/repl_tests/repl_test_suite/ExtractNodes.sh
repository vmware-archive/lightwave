#!/bin/bash
#Functionality:
#   1) Perform a ldap search on domain controllers list
#   2) Extract all the nodes in the federation
#Output:
#   - Creates a file named nodes which contains all the nodes listed under domain controllers
#Usage:
#   ./extractnodes.sh
#   Some of the variables are exported from BasicReplTests.sh/BasicJoinAndLeaveTests.sh

function extractNodes
{
    HOST="localhost"

    SEARCH_BASE="ou=domain controllers,dc=lightwave,dc=local"
    FILTER="cn"

    HOSTS_FILE="nodes"

    ldapsearch -h $HOST -p $PORT -o ldif-wrap=no -LLL  -Y "SRP" -U "$ADMIN_UPN" -w $PASSWD -b "$SEARCH_BASE" "$FILTER" > $HOSTS_FILE

    #clean up the search result
    #delete lines containing dn:
    sed -i '/dn:/d' nodes

    #delete empty lines
    sed -i '/^[[:space:]]*$/d' nodes

    #delete cn: tag in all the lines
    sed -i 's/cn: //g' nodes
}

#Get all the nodes in the federation
extractNodes
