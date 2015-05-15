noinst_PROGRAMS = vmdir_test_query

vmdir_test_query_SOURCES = \
    globals.c \
    main.c

vmdir_test_query_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -DLDAP_DEPRECATED=1 \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vmdir_test_query_LDADD = \
    $(top_builddir)/common/libcommon.la \
    @LWBASE_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @GSSAPI_LIBS@ \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

vmdir_test_query_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
