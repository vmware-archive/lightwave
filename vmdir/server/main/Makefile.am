sbin_PROGRAMS = vmdird

vmdird_SOURCES = \
    main.c

VMDIRD_BACKEND_STORE= $(top_builddir)/vmdir/server/mdb-store/libmdb-store.la

if REPLICATION_V2
    VMDIRD_REPLICATION_LIB = $(top_builddir)/vmdir/server/replication2/libreplication.la
else
    VMDIRD_REPLICATION_LIB = $(top_builddir)/vmdir/server/replication/libreplication.la
endif

vmdird_CPPFLAGS = \
    -g \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/include \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/vmdir/thirdparty \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmevent/include/public \
    -I$(top_srcdir)/vmcommon/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vmdird_LDADD = \
    $(top_builddir)/vmdir/server/vmkdc/libvmkdcserv.la \
    $(top_builddir)/vmdir/server/metrics/libmetrics.la \
    $(top_builddir)/vmdir/server/ldap-head/libldap-head.la \
    $(top_builddir)/vmdir/server/rest-head/librest-head.la \
    $(top_builddir)/vmdir/server/indexcfg/libindexcfg.la \
    $(top_builddir)/vmdir/server/dbintegritychk/libdb-integritychk.la \
    $(top_builddir)/vmdir/server/middle-layer/libmiddle-layer.la \
    $(top_builddir)/vmdir/server/schema/libschema.la \
    $(top_builddir)/vmdir/server/acl/libvmacl.la \
    $(top_builddir)/vmdir/server/backend/libbackend.la \
    $(VMDIRD_BACKEND_STORE) \
    $(VMDIRD_REPLICATION_LIB) \
    $(top_builddir)/vmdir/server/clusterstate/libclusterstate.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmdir/server/common/libsrvcommon.la \
    $(top_builddir)/vmdir/common/libcommon.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/krb5-crypto/libkrb5crypto.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/asn1/libasn1db.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/asn1/libasn1.la \
    $(top_builddir)/vmdir/thirdparty/csrp/libcsrp.la \
    $(top_builddir)/vmdir/thirdparty/openldap/libraries/liblmdb/.libs/libvmdirmdb.so \
    $(top_builddir)/vmidentity/ssoclients/common/src/libssocommon.la \
    $(top_builddir)/vmidentity/ssoclients/oidc/src/libssooidc.la \
    $(top_builddir)/vmcommon/libvmcommon.la \
    $(top_builddir)/vmdir/server/vmdir/libvmdird.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
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
    @CRESTENGINE_LIBS@

vmdird_LDFLAGS = \
    -rdynamic -ldl \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@ \
    @COPENAPI_LDFLAGS@ \
    @CRESTENGINE_LDFLAGS@
