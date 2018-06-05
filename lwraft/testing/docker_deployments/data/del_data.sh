#!/bin/bash

function node_stats() {
    docker ps -f id=$1
    docker cp del.data $1:/del.data
    docker exec -t $1 ldapmodify -D "cn=Administrator,cn=Users,dc=post,dc=test"  -w $POST_PWD -p 38900 -h 127.0.0.1 -f /del.data
}

docker ps -qf network=post_test_net | xargs -n1 -I{} bash -c "$(declare -f node_stats) ; node_stats {} ;"
