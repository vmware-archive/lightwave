bin_PROGRAMS = test-svr test-logon test-ldapbind

test_svr_SOURCES = test-svr.c
test_logon_SOURCES = test-logon.c
test_ldapbind_SOURCES = test-ldap-bind.c

test_svr_LDADD = \
    $(top_builddir)/interop/idm/ad/libidm.la \
    -llsaclient \
    -llwnetclientapi \
    -llwnetcommon \
    -llsacommon \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @LWADVAPI_LIBS@ \
    @GSSAPI_LIBS@ \
    @SASL_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @PTHREAD_LIBS@

test_svr_CPPFLAGS = \
    -I$(top_srcdir)/interop/include/public \
    -I$(top_srcdir)/interop/include \
    -I$(top_srcdir)/interop/idm/include \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

test_svr_LDFLAGS = \
    -rdynamic \
    $(VMDIRD_BACKEND_LD_FLAGS) \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

test_logon_LDADD = \
    $(top_builddir)/interop/idm/ad/libidm.la \
    -llsaclient \
    -llwnetclientapi \
    -llwnetcommon \
    -llsacommon \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @LWADVAPI_LIBS@ \
    @GSSAPI_LIBS@ \
    @SASL_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @PTHREAD_LIBS@

test_logon_CPPFLAGS = \
    -D_USE_GSS_SRP \
    -I$(top_srcdir)/interop/include/public \
    -I$(top_srcdir)/interop/include \
    -I$(top_srcdir)/interop/idm/include \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

test_logon_LDFLAGS = \
    -rdynamic \
    $(VMDIRD_BACKEND_LD_FLAGS) \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

test_ldapbind_CPPFLAGS = \
    -I$(top_srcdir)/interop/include/public \
    -I$(top_srcdir)/interop/include \
    -I$(top_srcdir)/interop/idm/include \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

test_ldapbind_LDFLAGS = \
    -rdynamic \
    $(VMDIRD_BACKEND_LD_FLAGS) \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

test_ldapbind_LDADD = \
    $(top_builddir)/interop/idm/ad/libidm.la \
    -llsaclient \
    -llwnetclientapi \
    -llwnetcommon \
    -llsacommon \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @LWADVAPI_LIBS@ \
    @GSSAPI_LIBS@ \
    @SASL_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @PTHREAD_LIBS@
