noinst_LTLIBRARIES = libvmdirtesting.la

libvmdirtesting_la_SOURCES = \
    assert.c \
    connection.c \
    misc.c \
    ldap.c \
    oidctoken.c \
    testinfrastructure.c \
    user.c

libvmdirtesting_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    -I$(top_srcdir)/vmcommon/include/public \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libvmdirtesting_la_LIBADD = \
    $(top_builddir)/vmcommon/libvmcommon.la \
    $(top_builddir)/vmidentity/ssoclients/common/src/libssocommon.la \
    $(top_builddir)/vmidentity/ssoclients/oidc/src/libssooidc.la \
    @top_builddir@/vmdir/common/libcommon.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

libvmdirtesting_la_LDFLAGS = \
    -static \
    @OPENSSL_LDFLAGS@
