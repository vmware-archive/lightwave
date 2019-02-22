bin_PROGRAMS = lw_mdb_walflush

lw_mdb_walflush_SOURCES = \
    main.c

lw_mdb_walflush_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/mdb \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

lw_mdb_walflush_LDADD = \
    $(top_builddir)/vmdir/common/libcommon.la \
    $(top_builddir)/vmdir/thirdparty/openldap/libraries/mdb/.libs/libvmdirmdb.so \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @UUID_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@ \
    @DL_LIBS@


lw_mdb_walflush_LDFLAGS = \
    -static \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
