bin_PROGRAMS = lwraftadmintool

lwraftadmintool_SOURCES = \
    main.c \
    ldapbindclient.c \
    util.c

lwraftadmintool_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/tools/include \
    -I$(top_srcdir)/client \
    -I$(top_builddir)/client \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

lwraftadmintool_LDADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/liblwraftclient.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @SASL_LIBS@ \
    @LDAP_LIBS@

lwraftadmintool_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@
