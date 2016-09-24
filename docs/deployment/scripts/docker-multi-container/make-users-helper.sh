#!/bin/sh -xe

PHOTON_USER=photon
PHOTON_USER_PASSWORD='Photon123$'
PHOTON_USER_FIRST_NAME="Light"
PHOTON_USER_LAST_NAME="Wave"
LIGHTWAVE_PASSWORD='Admin!23'

echo "Creating Groups in Lightwave Directory"
/opt/vmware/bin/dir-cli ssogroup create --name admins --password $LIGHTWAVE_PASSWORD

rc=$?
if [ $rc -ne 0 ]; then
    echo "Failed to create group [Name: ssogroup]"
    exit 1
fi

echo "Creating Users  in Lightwave Directory"
/opt/vmware/bin/dir-cli user create --account $PHOTON_USER \
                                    --user-password $PHOTON_USER_PASSWORD \
                                    --first-name $PHOTON_USER_FIRST_NAME \
                                    --last-name $PHOTON_USER_LAST_NAME \
                                    --password $LIGHTWAVE_PASSWORD
rc=$?
if [ $rc -ne 0 ]; then
    echo "Failed to create user [ Name: $PHOTON_USER ]"
    exit 1
fi

echo "Adding Users to Groups in Lightwave Directory"

/opt/vmware/bin/dir-cli group modify --name admins \
                                     --add $PHOTON_USER \
                                     --password $LIGHTWAVE_PASSWORD
rc=$?
if [ $rc -ne 0 ]; then
    echo "Failed to add user [$PHOTON_USER] to group [ admins ]"
    exit 1
fi
