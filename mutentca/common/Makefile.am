noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = \
    fsutils.c \
    globals.c \
    logging.c \
    memory.c \
    misc.c \
    opensslctx.c \
    string.c \
    thread.c \
    utils.c \
    vecs.c

libcommon_la_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libcommon_la_LIBADD = \
    @LBER_LIBS@ \
    @LDAP_LIBS@ \
    @LWREG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @PTHREAD_LIBS@

libcommon_la_LDFLAGS = \
    -DLDAP_DEPRECATED \
    -static \
    @OPENSSL_LDFLAGS@
