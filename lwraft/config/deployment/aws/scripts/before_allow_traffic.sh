#!/bin/bash

source $(dirname $(realpath $0))/common.sh

ERRCNT=0

echo "Step 1: Wait for leader election (mostly for 2nd node promotion)"

get_tag_value "POST_PASSWORD" POST_PASSWORD

echo '/opt/vmware/bin/post-cli node state --server-name localhost --login administrator --password <censored>'

MAX_RETRY=10
RETRY=1

while [ ${RETRY} -le ${MAX_RETRY} ]
do
    sleep 2
    /opt/vmware/bin/post-cli node state --server-name localhost --login administrator --password ${POST_PASSWORD} &> ${LOGDIR}/post_cli_node_state.log
    RET=$?
    echo "Attempt ${RETRY}: ${RET}"
    if [ ${RET} -eq 0 ]
    then
        break
    fi
    let RETRY++
done

if [ ${RET} -ne 0 ]
then
    echo "Error: Returned ${RET} / Expected 0"
    let ERRCNT++
elif [[ -z $(grep 'Leader' ${LOGDIR}/post_cli_node_state.log) ]]
then
    cat ${LOGDIR}/post_cli_node_state.log
    echo "Error: No POST leader is present"
    let ERRCNT++
else
    cat ${LOGDIR}/post_cli_node_state.log
    echo "Leader exists"
fi


echo "Step 2: Test LDAP search - DSE root entry"

echo 'ldapsearch -h localhost -p 38900 -x -s base dn'
ldapsearch -h localhost -p 38900 -x -s base dn &> ${LOGDIR}/ldapsearch_dseroot.log

RET=$?
if [ ${RET} -ne 0 ]
then
    echo "Error: Returned ${RET} / Expected 0"
    let ERRCNT++
elif [[ -z $(grep 'dn: cn=DSE Root' ${LOGDIR}/ldapsearch_dseroot.log) ]]
then
    cat ${LOGDIR}/ldapsearch_dseroot.log
    echo "Error: DSE Root entry not found"
    let ERRCNT++
else
    echo "Search successful"
fi


echo "Step 3: Test HTTP LDAP search - DSE root entry"

echo 'curl -X GET http://localhost:7577/v1/post/ldap?dn=cn%3DDSE%20Root'
curl -X GET 'http://localhost:7577/v1/post/ldap?dn=cn%3DDSE%20Root' &> ${LOGDIR}/http_ldapsearch_dseroot.log

RET=$?
if [ ${RET} -ne 0 ]
then
    echo "Error: Returned ${RET} / Expected 0"
    let ERRCNT++
elif [[ -z $(grep '"dn":"cn=DSE Root"' ${LOGDIR}/http_ldapsearch_dseroot.log) ]]
then
    cat ${LOGDIR}/http_ldapsearch_dseroot.log
    echo "Error: DSE Root entry not found"
    let ERRCNT++
else
    echo "Search successful"
fi


echo "All tests executed (total failed test count = ${ERRCNT})"
exit ${ERRCNT}
