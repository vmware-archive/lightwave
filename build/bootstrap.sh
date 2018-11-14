#!/bin/sh

autoreconf -vif .. \
&& \
../configure \
    CFLAGS="-Wall -Werror" \
    LDFLAGS="-ldl -pie -fPIE" \
    --prefix=/opt/vmware \
    --enable-debug=yes \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware \
    --with-config=./config --enable-plugins --enable-security_aws_kms \
