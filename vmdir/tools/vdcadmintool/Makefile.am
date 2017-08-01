bin_PROGRAMS = vdcadmintool

vdcadmintool_SOURCES = \
    main.c \
    ldapbindclient.c \
    util.c

vdcadmintool_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/tools/include \
    -I$(top_srcdir)/vmdir/client \
    -I$(top_builddir)/vmdir/client \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcadmintool_LDADD = \
    $(top_builddir)/vmdir/common/libcommon.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @SASL_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@

vdcadmintool_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@
