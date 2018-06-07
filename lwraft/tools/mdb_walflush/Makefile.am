bin_PROGRAMS = mdb_walflush

mdb_walflush_SOURCES = \
    main.c

mdb_walflush_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/mdb \
    -I$(top_srcdir)/vmmetrics/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

mdb_walflush_LDADD = \
    $(top_builddir)/lwraft/common/libcommon.la \
    $(top_builddir)/lwraft/client/libpostclient.la \
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
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@


mdb_walflush_LDFLAGS = \
    -static \
    $(top_builddir)/lwraft/thirdparty/openldap/libraries/mdb/liblwraftmdb.la \
    @LW_LDFLAGS@
