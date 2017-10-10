noinst_LTLIBRARIES = liblwrafttesting.la

liblwrafttesting_la_SOURCES = \
    assert.c \
    connection.c \
    misc.c \
    ldap.c \
    testinfrastructure.c \
    user.c

liblwrafttesting_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

liblwrafttesting_la_LIBADD = \
    @top_builddir@/lwraft/common/libcommon.la \
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

liblwrafttesting_la_LDFLAGS = \
    -static \
    @OPENSSL_LDFLAGS@
