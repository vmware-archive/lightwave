bin_PROGRAMS = srvcommon

check_PROGRAMS = srvcommon
TESTS = srvcommon

srvcommon_SOURCES = \
    main.c \
    metadata.c \
    valuemetadata.c

srvcommon_CPPFLAGS = \
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
    @OPENSSL_INCLUDES@ \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

srvcommon_LDADD = \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmdir/server/common/libsrvcommon.la \
    $(top_builddir)/vmdir/common/libcommon.la \
    @PTHREAD_LIBS@ \
    @CMOCKA_LIBS@

srvcommon_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
