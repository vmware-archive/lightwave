#!/bin/bash

function show_mdb_env() {
  db_path=/var/lib/vmware/vmdir
  docker ps -f id=$1
  docker exec -t $1 mdb_stat -e "$db_path" | grep 'Number of\|Last trans'
  docker exec -t $1 ls -alh "$db_path/data.mdb"
}

docker ps -qf name=vmdir.test | xargs -n1 -I{} bash -c "$(declare -f show_mdb_env) ; show_mdb_env {} ;"
