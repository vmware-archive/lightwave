lib_LTLIBRARIES = libsecuritydescriptortests.la

libsecuritydescriptortests_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @OPENSSL_INCLUDES@ \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libsecuritydescriptortests_la_SOURCES = \
    administratorrights.c \
    administratorsrights.c \
    bad_parameters.c \
    domainadminsrights.c \
    domainclientsrights.c \
    inheritance.c \
    legacy_access_checks.c \
    main.c \
    ntsecuritydescriptor.c \
    protected_entries.c \
    sddl.c \
    standard_operations.c \
    util.c \
    wellknownsids.c

libsecuritydescriptortests_la_LIBADD = \
    @top_builddir@/testing/test_lib/libvmdirtesting.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

libsecuritydescriptortests_la_LDFLAGS = \
    -export-symbols @top_srcdir@/testing/integration_tests/acls/libsecuritydescriptortests.exp \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
