#!/bin/sh

autoreconf -vif ..

STAGEDIR=$PWD/stage

../configure \
     CFLAGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare" \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --with-vmevent=$STAGEDIR/opt/vmware \
    --with-vmdir=$STAGEDIR/opt/vmware \
    --with-vmdns=$STAGEDIR/opt/vmware \
    --with-afd=$STAGEDIR/opt/vmware \
    --with-likewise=/opt/likewise \
    --with-ssl=/usr \
    --with-jansson=/usr \
    --with-java=/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.121-10.b14.fc25.x86_64 \
    --with-ant=/usr/share/ant \
    --with-python=/usr \
    --with-config=./config \
    --with-version="1.3.0" \
    --enable-lightwave-build=yes
