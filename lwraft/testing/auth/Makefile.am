noinst_PROGRAMS = lwraft_test_access

lwraft_test_access_SOURCES = \
    main.c

lwraft_test_access_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -DLDAP_DEPRECATED=1 \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

lwraft_test_access_LDADD = \
    $(top_builddir)/lwraft/common/libcommon.la \
    @LWBASE_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @GSSAPI_LIBS@ \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

lwraft_test_access_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
