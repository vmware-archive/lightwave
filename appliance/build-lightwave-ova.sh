#!/bin/bash -xe
# Copyright 2017 VMware, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed
# under the License is distributed on an "AS IS" BASIS, without warranties or
# conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the License for the
# specific language governing permissions and limitations under the License.

# if staging path is not given, use working dir
WORKING_PATH=`pwd`
if [ -n "$1" ]; then
    STAGE_PATH="$1"
    if [[ ! -d $STAGE_PATH ]]; then
        echo "Invalid staging directory: $STAGE_PATH"
        exit  -1
    fi
else
    STAGE_PATH=$WORKING_PATH
fi

# only build photon ova if directed
if [ $PHOTON_BUILD ]; then
  echo "Building Photon OVA"
  pushd ${WORKING_PATH}/../photon-ova
  ./build.sh
  popd
fi

export SOURCE_OVA=${STAGE_PATH}/photon-ova-build/`basename ${STAGE_PATH}/photon-ova-build/photon*.ova`

packer build -force -var "build_path=$STAGE_PATH/lw-ova-build" $WORKING_PATH/lightwave-packer.json

# make ova vmware compatible
${WORKING_PATH}/convert_ova.sh ${STAGE_PATH}/lw-ova-build/lightwave-vb \
               ${STAGE_PATH}/lightwave ${WORKING_PATH}/add-ovf-params.sh
