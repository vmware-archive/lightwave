sbin_PROGRAMS = mutentcad

mutentcad_SOURCES = \
    globals.c \
    init.c \
    main.c \
    service.c \
    signal.c \
    state.c

mutentcad_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/mutentca/server/common \
    -I$(top_srcdir)/mutentca/server/config \
    -I$(top_srcdir)/vmafd/include/public \
    @LW_INCLUDES@ \
    @POPT_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@ \
    @COPENAPI_INCLUDES@ \
    @CRESTENGINE_INCLUDES@

mutentcad_LDADD = \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/config/libmutentcasrvconfig.la \
    $(top_builddir)/mutentca/server/rest-head/libmutentcaresthead.la \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @POPT_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @COPENAPI_LIBS@ \
    @CRESTENGINE_LIBS@ \
    @DL_LIBS@

mutentcad_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @POPT_LDFLAGS@ \
    @JANSSON_LDFLAGS@ \
    @COPENAPI_LDFLAGS@ \
    @CRESTENGINE_LDFLAGS@
