bin_PROGRAMS = vdcsrp

vdcsrp_SOURCES = \
    main.c \
    parseargs.c

vdcsrp_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/tools/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcsrp_LDADD = \
    $(top_builddir)/lwraft/client/liblwraftclient.la \
    $(top_builddir)/lwraft/common/libcommon.la \
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
    @LDAP_LIBS@

vdcsrp_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
