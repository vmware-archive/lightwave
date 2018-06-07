#!/bin/bash
if [ -z "$VMDIR_PWD" ]; then
  echo "Please set VMDIR_PWD env var. (8 chars or more. 1 cap, 1 num, 1 special)"
  exit 1
fi
docker build --build-arg VMDIR_PWD_IN=${VMDIR_PWD} -t docker_vmdirtest .
