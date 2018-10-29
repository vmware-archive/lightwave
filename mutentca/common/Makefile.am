noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = \
    fsutils.c \
    globals.c \
    jsonutils.c \
    logging.c \
    ldap.c \
    memory.c \
    misc.c \
    opensslctx.c \
    pkcs.c \
    regexutil.c \
    string.c \
    thread.c \
    token_util.c \
    utils.c \
    vmafd.c

libcommon_la_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@

libcommon_la_LIBADD = \
    $(top_builddir)/vmidentity/ssoclients/oidc/src/libssooidc.la \
    @LBER_LIBS@ \
    @LDAP_LIBS@ \
    @LWREG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@

libcommon_la_LDFLAGS = \
    -DLDAP_DEPRECATED \
    -static \
    @OPENSSL_LDFLAGS@
