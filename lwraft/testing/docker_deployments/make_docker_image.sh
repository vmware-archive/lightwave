#!/bin/bash
if [ -z "$POST_PWD" ]; then
  echo "Please set POST_PWD env var. (8 chars or more. 1 cap, 1 num, 1 special)"
  exit 1
fi
docker --build-arg POST_PWD_IN=${POST_PWD} build -t docker_posttest .
