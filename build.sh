#!/bin/sh

autoreconf -vif .

./configure \
     CFLAGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare" \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --with-vmevent=/home/brownj/workspaces/git/pslabs/lightwave/stage/opt/vmware \
    --with-vmdir=/home/brownj/workspaces/git/pslabs/lightwave/stage/opt/vmware \
    --with-vmafd=/home/brownj/workspaces/git/pslabs/lightwave/stage/opt/vmware \
    --with-vmdns=/home/brownj/workspaces/git/pslabs/lightwave/stage/opt/vmware \
    --with-likewise=/opt/likewise \
    --with-ssl=/usr \
    --with-jansson=/usr \
    --with-java=/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.121-10.b14.fc25.x86_64 \
    --with-ant=/usr/share/ant \
    --with-config=./config \
    --with-version="4.4.4" \
    --enable-lightwave-build=yes
