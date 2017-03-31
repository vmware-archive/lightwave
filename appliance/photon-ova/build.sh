#!/bin/bash -xe

# source ../scripts/http.sh

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

# we are defaulting to PhotonOS version 1.0 full iso
PHOTON_ISO_URL=${ISO_URL:="https://bintray.com/artifact/download/vmware/photon/photon-1.0-13c08b6.iso"}
PHOTON_ISO_NAME=`basename $PHOTON_ISO_URL`
PHOTON_ISO_PATH="$STAGE_PATH/$PHOTON_ISO_NAME"

pushd $STAGE_PATH
if [ ! -f $PHOTON_ISO_PATH ]; then
    wget --no-proxy --no-check-certificate -nv -N  $PHOTON_ISO_URL
fi

PHOTON_ISO_SHA1=`sha1sum $PHOTON_ISO_PATH | cut -d' ' -f1`

packer build -force  \
	-var "photon_iso_url=file://localhost/$PHOTON_ISO_PATH" \
	-var "photon_iso_sha1=$PHOTON_ISO_SHA1" \
        -var "script_path=$WORKING_PATH/.." \
        -var "build_path=$STAGE_PATH/photon-ova-build" \
	$WORKING_PATH/photon-ova.json
popd
