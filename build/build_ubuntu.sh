#!/bin/sh

autoreconf -vif ..

STAGEDIR=$PWD/stage

../configure \
     LDFLAGS=-ldl \
     LIBS=-ldl \
     STAGEDIR=$STAGEDIR \
     CFLAGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare" \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --with-vmevent=$STAGEDIR/opt/vmware \
    --with-vmdir=$STAGEDIR/opt/vmware \
    --with-vmdns=$STAGEDIR/opt/vmware \
    --with-afd=$STAGEDIR/opt/vmware \
    --with-vmca=$STAGEDIR/opt/vmware \
    --with-sts=$STAGEDIR/opt/vmware \
    --with-likewise=/opt/likewise \
    --with-ssl=/usr \
    --with-jansson=/usr \
    --with-java=/usr/lib/jvm/java-1.8.0-openjdk-amd64 \
    --with-maven=/usr/share/maven \
    --with-ant=/usr/share/ant \
    --with-python=/usr \
    --with-config=./config \
    --with-version="1.3.0" \
    --with-datastore=mdb \
    --enable-server=yes \
    --enable-krb5-default=yes \
    --enable-lightwave-build=yes \
    ac_cv_header_vmevent=yes \
    ac_cv_header_vmdirclient_h=yes \
    ac_cv_lib_vmdirclient_VmDirSetupHostInstance=yes \
    ac_cv_lib_vmdirclient_VmDirConnectionOpen=yes \
    ac_cv_header_vmdns_h=yes \
    ac_cv_lib_vmdnsclient_VmDnsOpenServerA=yes \
    ac_cv_header_vmafdclient_h=yes \
    ac_cv_lib_vmafdclient_VmAfdGetDomainNameA=yes \
    ac_cv_header_vmca_h=yes \
    ac_cv_lib_vmcaclient_VMCACreateSelfSignedCertificateA=yes
    #--prefix=/opt/vmware \
    #--libdir=/opt/vmware/lib64 \
    #--with-vmevent=$STAGEDIR/opt/vmware \
    #--with-vmdir=$STAGEDIR/opt/vmware \
    #--with-vmdns=$STAGEDIR/opt/vmware \
    #--with-afd=$STAGEDIR/opt/vmware \
    #--with-likewise=/opt/likewise \
    #--with-ssl=/usr \
    #--with-jansson=/usr \
    #--with-java=/usr/lib/jvm/java-1.8.0-openjdk-amd64 \
    #--with-ant=/usr/share/ant \
    #--with-python=/usr \
    #--with-config=./config \
    #--with-version="1.3.0" \
    #--enable-lightwave-build=yes

make
