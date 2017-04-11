noinst_LTLIBRARIES = libvmdirtesting.la

libvmdirtesting_la_SOURCES = \
    assert.c \
    connection.c \
    misc.c \
    ldap.c \
    testinfrastructure.c \
    user.c

libvmdirtesting_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libvmdirtesting_la_LIBADD = \
    @top_builddir@/common/libcommon.la \
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
