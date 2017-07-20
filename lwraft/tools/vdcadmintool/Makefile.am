bin_PROGRAMS = postadmintool

postadmintool_SOURCES = \
    main.c \
    ldapbindclient.c \
    util.c

postadmintool_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/tools/include \
    -I$(top_srcdir)/client \
    -I$(top_builddir)/client \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

postadmintool_LDADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/libpostclient.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @UUID_LIBS@ \
    @GSSAPI_LIBS@ \
    @SASL_LIBS@ \
    @LDAP_LIBS@

postadmintool_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@
