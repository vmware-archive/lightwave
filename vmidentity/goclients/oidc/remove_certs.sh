#!/bin/bash

DOMAIN=$1
TEMP_DIR="/tmp/certs"

mkdir -p $TEMP_DIR
certs=($(ls /etc/ssl/certs/ | grep .0))
for cert in ${certs[@]} ; do
    if openssl x509 -in /etc/ssl/certs/$cert -text -noout | grep -q "$DOMAIN"; then
        mv /etc/ssl/certs/$cert "$TEMP_DIR/$cert"
    fi
done
