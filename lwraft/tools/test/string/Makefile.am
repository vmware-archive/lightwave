bin_PROGRAMS = stringtest

stringtest_SOURCES = \
    VmDirAllocateStringOfLenA.c \
    VmDirAllocateStringA.c \
    stringlist.c    \
    main.c

stringtest_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/tools/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

stringtest_LDADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/liblwraftclient.la \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@

stringtest_LDFLAGS = \
    @LW_LDFLAGS@
