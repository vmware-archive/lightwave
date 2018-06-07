#!/bin/bash

function node_promote() {
    docker ps -f id=$1
    docker exec -t $1 /opt/vmware/bin/post-cli node promote --password $POST_PWD --partner-name post1.post.test
}

#exec a promote of node n

docker ps -qf name=post$1.post.test | xargs -n1 -I{} bash -c "$(declare -f node_promote) ; node_promote {} ;"
