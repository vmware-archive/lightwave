#!/bin/sh

autoreconf -vif .. \
&& \
../configure \
    CFLAGS="-Wall -Werror -D_FORTIFY_SOURCE=2 -O2" \
    LDFLAGS="-ldl -pie -fPIE" \
    --prefix=/opt/vmware \
    --enable-debug=yes \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware \
    --with-config=./config --enable-plugins --enable-security_aws_kms \
