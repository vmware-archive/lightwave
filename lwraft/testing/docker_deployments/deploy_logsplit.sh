#!/bin/bash
#deploys with log split turned on
node=1
if [[ $1 ]]; then
  node=$1
fi
./deploy.sh $node 1
