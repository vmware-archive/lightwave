bin_PROGRAMS = mdb_walflush

if MDB_0_9_22
    MDB_DIR = vmdir/thirdparty/openldap/libraries/liblmdb
    MDB_LIB = libvmdirmdb.la
else
    MDB_DIR = lwraft/thirdparty/openldap/libraries/mdb
    MDB_LIB = liblwraftmdb.la
endif

mdb_walflush_SOURCES = \
    main.c

mdb_walflush_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/$(MDB_DIR) \
    -I$(top_srcdir)/vmcommon/include/public \
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
    $(top_builddir)/$(MDB_DIR)/$(MDB_LIB) \
    @LW_LDFLAGS@
