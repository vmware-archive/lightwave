bin_PROGRAMS = ssoidmclienttest

ssoidmclienttest_SOURCES = \
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
    user_resource_hok_token_test.c \
    user_resource_test.c

ssoidmclienttest_CPPFLAGS = \
    -I$(top_srcdir)/ssoclients/common/include/ \
    -I$(top_srcdir)/ssoclients/common/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/idmclient/include/public/

ssoidmclienttest_LDADD = \
    @top_builddir@/ssoclients/restclient/idmclient/src/libssoidmclient.la
