#!/bin/bash

function node_stats() {
    docker ps -f id=$1
    docker cp add.data $1:/add.data
    docker exec -t $1 ldapadd -D "cn=Administrator,cn=Users,dc=post,dc=test"  -w $POST_PWD -p 38900 -h 127.0.0.1 -f /add.data
}

docker ps -qf network=post_test_net | xargs -n1 -I{} bash -c "$(declare -f node_stats) ; node_stats {} ;"
