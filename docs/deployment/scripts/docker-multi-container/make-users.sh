#!/bin/sh -xe

docker cp ./make-users-helper.sh photon-controller-0:/

docker exec -t photon-controller-0 /make-users-helper.sh
