lib_LTLIBRARIES = libssoidmclient.la

libssoidmclient_la_SOURCES = \
    assertion_consumer_service_array_data.c \
    assertion_consumer_service_data.c \
    attribute_array_data.c \
    attribute_consumer_service_array_data.c \
    attribute_consumer_service_data.c \
    attribute_data.c \
    authentication_policy_data.c \
    brand_policy_data.c \
    certificate_chain_array_data.c \
    certificate_chain_data.c \
    certificate_resource.c \
    client_certificate_policy_data.c \
    diagnostics_resource.c \
    event_log_array_data.c \
    event_log_data.c \
    event_log_ldap_query_stat_array_data.c \
    event_log_ldap_query_stat_data.c \
    event_log_metadata_data.c \
    event_log_provider_metadata_data.c \
    event_log_status_data.c \
    external_idp_array_data.c \
    external_idp_data.c \
    external_idp_resource.c \
    group_array_data.c \
    group_data.c \
    group_details_data.c \
    group_resource.c \
    identity_provider_array_data.c \
    identity_provider_data.c \
    identity_provider_resource.c \
    lockout_policy_data.c \
    oidc_client_array_data.c \
    oidc_client_data.c \
    oidc_client_metadata_data.c \
    oidc_client_resource.c \
    password_details_data.c \
    password_policy_data.c \
    password_reset_request_data.c \
    principal_data.c \
    private_key_data.c \
    provider_policy_data.c \
    relying_party_array_data.c \
    relying_party_data.c \
    relying_party_resource.c \
    resource_server_array_data.c \
    resource_server_data.c \
    resource_server_resource.c \
    schema_object_mapping_data.c \
    schema_object_mapping_map_data.c \
    schema_object_mapping_map_entry_data.c \
    search_result_data.c \
    server_details_array_data.c \
    server_details_data.c \
    server_resource.c \
    service_endpoint_array_data.c \
    service_endpoint_data.c \
    signature_algorithm_array_data.c \
    signature_algorithm_data.c \
    solution_user_array_data.c \
    solution_user_data.c \
    solution_user_resource.c \
    string_array_data.c \
    string_map_data.c \
    string_map_entry_data.c \
    tenant_configuration_data.c \
    tenant_credentials_data.c \
    tenant_data.c \
    tenant_resource.c \
    token_claim_group_array_data.c \
    token_claim_group_data.c \
    token_policy_data.c \
    user_array_data.c \
    user_data.c \
    user_details_data.c \
    user_resource.c

libssoidmclient_la_CPPFLAGS = \
    -I$(top_srcdir)/ssoclients/common/include/ \
    -I$(top_srcdir)/ssoclients/common/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/idmclient/include/public/

libssoidmclient_la_LIBADD = \
    @top_builddir@/ssoclients/restclient/coreclient/src/libssocoreclient.la
