bin_PROGRAMS = replication2

check_PROGRAMS = replication2
TESTS = replication2

replication2_SOURCES = \
    extractevents.c \
    main.c \
    mock.c \
    testcommon.c \
    update.c \
    updatelist.c

replication2_CPPFLAGS = \
    -g \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/include \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal/krb5-crypto \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal/asn1 \
    -I$(top_srcdir)/vmdir/thirdparty \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmdir/server \
    @OPENSSL_INCLUDES@ \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

replication2_LDADD = \
    $(top_builddir)/vmdir/server/vmkdc/libvmkdcserv.la \
    $(top_builddir)/vmdir/server/metrics/libmetrics.la \
    $(top_builddir)/vmdir/server/ldap-head/libldap-head.la \
    $(top_builddir)/vmdir/server/rest-head/librest-head.la \
    $(top_builddir)/vmdir/server/indexcfg/libindexcfg.la \
    $(top_builddir)/vmdir/server/middle-layer/libmiddle-layer.la \
    $(top_builddir)/vmdir/server/schema/libschema.la \
    $(top_builddir)/vmdir/server/acl/libvmacl.la \
    $(top_builddir)/vmdir/server/backend/libbackend.la \
    $(top_builddir)/vmdir/server/mdb-store/libmdb-store.la \
    $(top_builddir)/vmdir/thirdparty/openldap/libraries/liblmdb/libvmdirmdb.la \
    $(top_builddir)/vmdir/server/replication2/libreplication.la \
    $(top_builddir)/vmdir/server/clusterstate/libclusterstate.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmdir/server/common/libsrvcommon.la \
    $(top_builddir)/vmdir/common/libcommon.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/krb5-crypto/libkrb5crypto.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/asn1/libasn1db.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/asn1/libasn1.la \
    $(top_builddir)/vmdir/thirdparty/csrp/libcsrp.la \
    $(top_builddir)/vmidentity/ssoclients/common/src/libssocommon.la \
    $(top_builddir)/vmidentity/ssoclients/oidc/src/libssooidc.la \
    $(top_builddir)/vmcommon/libvmcommon.la \
    $(top_builddir)/vmdir/server/vmdir/libvmdird.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWBASE_LIBS@ \
    @LWADVAPI_LIBS@ \
    @GSSAPI_LIBS@ \
    @SASL_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @COPENAPI_LIBS@ \
    @CRESTENGINE_LIBS@ \
    @CMOCKA_LIBS@

replication2_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    -Wl,--wrap,VmDirEntryGetAllMustAttrs
