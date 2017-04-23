#!/bin/sh
./configure \
     CFLAGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wimplicit-function-declaration -Wno-address -Wno-enum-compare" \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware/vmdir \
    --with-java=/var/ZZ
    --with-vmevent=/home/brownj/workspaces/git/pslabs/lightwave/stage/opt/vmware \
    --with-likewise=/opt/likewise \
    --with-config=./config \
    --with-ssl=/usr \
    --with-version="4.4.4" \
    --enable-lightwave-build=yes


#
#    --with-vmevent=./vmevent \
#    --with-vmdir=./vmdir \
#    --with-vmafd=./vmafd \
#    --with-vmdns=./vmdns \
#
# vmevent options
#
# vmdir options
#    --enable-server=yes
#    --with-datastore=mdb
#    --with-sasl=/usr
#    --with-logdir=/var/log
#
#
# vmafd options
#
# vmca options
#
# vmdns options
#
