#!/bin/sh

autoreconf -vif ..

STAGEDIR=$PWD/stage

../configure \
    CFLAGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-pointer-sign -Wno-implicit-function-declaration -Wno-address -Wno-enum-compare" \
    STAGEDIR=$STAGEDIR \
    --disable-dependency-tracking \
    --prefix=/opt/vmware \
    --libdir=/opt/vmware/lib64 \
    --localstatedir=/var/lib/vmware \
    --with-vmevent=$STAGEDIR/opt/vmware \
    --with-vmdir=$STAGEDIR/opt/vmware \
    --with-vmdns=$STAGEDIR/opt/vmware \
    --with-afd=$STAGEDIR/opt/vmware \
    --with-vmca=$STAGEDIR/opt/vmware \
    --with-sts=$STAGEDIR/opt/vmware \
    --with-oidc=$STAGEDIR/opt/vmware \
    --with-likewise=/opt/likewise \
    --with-logdir=/var/log/lightwave \
    --with-ssl=/usr \
    --with-sqlite=/usr \
    --with-jansson=/usr \
    --with-copenapi=/usr \
    --with-c-rest-engine=/usr \
    --with-java=/var/opt/OpenJDK-1.8.0.112-bin \
    --with-maven=/var/opt/apache-maven-3.3.9 \
    --with-ant=/var/opt/apache-ant-1.9.6 \
    --with-boost=/usr \
    --with-python=/usr \
    --with-sasl=/usr \
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
    ac_cv_header_jansson_h=yes \
    ac_cv_lib_jansson_json_object=yes \
    ac_cv_header_copenapi_copenapi_h=yes \
    ac_cv_lib_copenapi_coapi_load_from_file=yes \
    ac_cv_header_vmrest_h=yes \
    ac_cv_lib_restengine_VmRESTInit=yes \
    ac_cv_header_oidc_h=yes \
    ac_cv_header_oidc_types_h=yes \
    ac_cv_lib_ssooidc_OidcAccessTokenBuild=yes \
    ac_cv_header_common_types_h=yes \
    ac_cv_header_vmdns_h=yes \
    ac_cv_lib_vmdnsclient_VmDnsOpenServerA=yes \
    ac_cv_header_vmafdclient_h=yes \
    ac_cv_lib_vmafdclient_VmAfdGetDomainNameA=yes \
    ac_cv_header_vmca_h=yes \
    ac_cv_lib_vmcaclient_VMCACreateSelfSignedCertificateA=yes && make
