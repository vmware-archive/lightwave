bin_PROGRAMS = ssorestclienttest

ssorestclienttest_SOURCES = \
    ad_provider_resource_test.c \
    certificate_resource_test.c \
    diagnostics_resource_test.c \
    external_idp_resource_test.c \
    group_resource_test.c \
    identity_provider_resource_test.c \
    oidc_client_resource_test.c \
    relying_party_resource_test.c \
    resource_server_resource_test.c \
    server_resource_test.c \
    solution_user_resource_test.c \
    tenant_resource_test.c \
    test.c \
    test_util.c \
    user_resource_test.c \
    vecs_resource_test.c

ssorestclienttest_CPPFLAGS = \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/ \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public/ \
    -I$(top_srcdir)/vmidentity/ssoclients/restclient/coreclient/include/ \
    -I$(top_srcdir)/vmidentity/ssoclients/restclient/coreclient/include/public/ \
    -I$(top_srcdir)/vmidentity/ssoclients/restclient/idmclient/include/public/ \
    -I$(top_srcdir)/vmidentity/ssoclients/restclient/vmdirclient/include/public/ \
    -I$(top_srcdir)/vmidentity/ssoclients/restclient/afdclient/include/public/ \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public/

ssorestclienttest_LDADD = \
    @top_builddir@/vmidentity/ssoclients/common/src/libssocommon.la \
    @top_builddir@/vmidentity/ssoclients/restclient/idmclient/src/libssoidmclient.la \
    @top_builddir@/vmidentity/ssoclients/restclient/vmdirclient/src/libssovmdirclient.la \
    @top_builddir@/vmidentity/ssoclients/restclient/afdclient/src/libssoafdclient.la \
    @top_builddir@/vmidentity/ssoclients/restclient/coreclient/src/libssocoreclient.la \
    @top_builddir@/vmidentity/ssoclients/oidc/src/libssooidc.la
