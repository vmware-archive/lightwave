#!/bin/bash

function node_demote() {
    docker ps -f id=$1
    docker exec -t $1 /opt/vmware/bin/post-cli node demote --server-name  localhost --password $POST_PWD --demote-host-name $2
}

node=1
#need a node number passed in
if [[ $1 ]]; then
  node=$1
fi

#exec a demote of node n at first node

demote_host_name=post$1.post.test
docker ps -qf name=post1.post.test | xargs -n1 -I{} bash -c "$(declare -f node_demote) ; node_demote {} $demote_host_name ;"
