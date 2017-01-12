bin_PROGRAMS = vdcleavefed

vdcleavefed_SOURCES = \
    parseargs.c \
    main.c

vdcleavefed_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/tools/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcleavefed_LDADD = \
    $(top_builddir)/client/liblwraftclient.la \
    $(top_builddir)/common/libcommon.la \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@

vdcleavefed_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
