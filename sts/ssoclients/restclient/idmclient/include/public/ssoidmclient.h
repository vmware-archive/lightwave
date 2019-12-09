/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef INCLUDE_PUBLIC_SSOIDMCLIENT_H_
#define INCLUDE_PUBLIC_SSOIDMCLIENT_H_

// data attributes

typedef enum
{
    IDM_CERTIFICATE_SCOPE_TYPE_TENANT, /* Tenant certificates */
    IDM_CERTIFICATE_SCOPE_TYPE_EXTERNAL_IDP, /* External Identity Provider certificates */
} IDM_CERTIFICATE_SCOPE_TYPE;

typedef enum
{
    IDM_MEMBER_TYPE_USER, /* Users */
    IDM_MEMBER_TYPE_GROUP, /* Groups */
    IDM_MEMBER_TYPE_SOLUTIONUSER, /* Solution Users */
    IDM_MEMBER_TYPE_ALL /* All available member types */
} IDM_MEMBER_TYPE;

typedef enum
{
    IDM_SEARCH_TYPE_CERT_SUBJECTDN, /* Search for Solution Users by certificate subject distinguished name */
    IDM_SEARCH_TYPE_NAME /* Search for members by name */
} IDM_SEARCH_TYPE;

typedef enum
{
    IDM_TENANT_CONFIG_TYPE_ALL, /* Retrieve all tenant configuration policies */
    IDM_TENANT_CONFIG_TYPE_LOCKOUT, /* Retrieve only the tenant lockout policy */
    IDM_TENANT_CONFIG_TYPE_PASSWORD, /* Retrieve only the tenant password policy */
    IDM_TENANT_CONFIG_TYPE_PROVIDER, /* Retrieve only the tenant provider policy */
    IDM_TENANT_CONFIG_TYPE_TOKEN, /* Retrieve only the tenant token policy */
    IDM_TENANT_CONFIG_TYPE_BRAND, /* Retrieve only the tenant brand policy */
    IDM_TENANT_CONFIG_TYPE_AUTHENTICATION /* Retrieve only the tenant authentication policy */
} IDM_TENANT_CONFIG_TYPE;

typedef enum
{
    IDM_COMPUTER_TYPE_DC, /* Domain Controller */
    IDM_COMPUTER_TYPE_COMPUTER, /* Computers (VMs) */
    IDM_COMPUTER_TYPE_ALL, /* Both Domain Controllers and Computers */
} IDM_COMPUTER_TYPE;

typedef struct
{
    IDM_TENANT_CONFIG_TYPE** ppEntry;
    size_t length;
} IDM_TENANT_CONFIG_TYPE_ARRAY;

typedef struct
{
    PSTRING* ppEntry;
    size_t length;
} IDM_STRING_ARRAY_DATA;

typedef struct
{
    PSTRING key;
    PSTRING value;
} IDM_STRING_MAP_ENTRY_DATA;

typedef struct
{
    IDM_STRING_MAP_ENTRY_DATA** ppEntry;
    size_t length;
} IDM_STRING_MAP_DATA;

typedef struct
{
    PSTRING name;
    PSTRING endpoint;
    PSTRING binding;
    INTEGER* index;
} IDM_ASSERTION_CONSUMER_SERVICE_DATA;

typedef struct
{
    IDM_ASSERTION_CONSUMER_SERVICE_DATA** ppEntry;
    size_t length;
} IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA;

typedef struct
{
    PSTRING name;
    PSTRING friendlyName;
    PSTRING nameFormat;
} IDM_ATTRIBUTE_DATA;

typedef struct
{
    IDM_ATTRIBUTE_DATA** ppEntry;
    size_t length;
} IDM_ATTRIBUTE_ARRAY_DATA;

typedef struct
{
    PSTRING name;
    INTEGER* index;
    IDM_ATTRIBUTE_ARRAY_DATA* attributes;
} IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA;

typedef struct
{
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA** ppEntry;
    size_t length;
} IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA;

typedef struct
{
    IDM_STRING_ARRAY_DATA* certPolicyOIDs;
    REST_CERTIFICATE_ARRAY_DATA* trustedCACertificates;
    bool* revocationCheckEnabled;

    // OCSP (Online Certificate Status Protocol) related configurable entities
    bool* ocspEnabled;
    bool* failOverToCrlEnabled;
    PSTRING ocspUrlOverride;

    // CRL (Certificate Revocation List) related configurable entities
    bool* crlDistributionPointUsageEnabled;
    PSTRING crlDistributionPointOverride;
} IDM_CLIENT_CERTIFICATE_POLICY_DATA;

typedef struct
{
    bool* passwordBasedAuthenticationEnabled;
    bool* windowsBasedAuthenticationEnabled;
    bool* certificateBasedAuthenticationEnabled;
    IDM_CLIENT_CERTIFICATE_POLICY_DATA* clientCertificatePolicy;
} IDM_AUTHENTICATION_POLICY_DATA;

typedef struct
{
    PSTRING name;
    PSTRING logonBannerTitle;
    PSTRING logonBannerContent;
    bool* logonBannerCheckboxEnabled;
    bool* logonBannerDisabled;
} IDM_BRAND_POLICY_DATA;

typedef struct
{
    REST_CERTIFICATE_ARRAY_DATA* certificates;
} IDM_CERTIFICATE_CHAIN_DATA;

typedef struct
{
    IDM_CERTIFICATE_CHAIN_DATA** ppEntry;
    size_t length;
} IDM_CERTIFICATE_CHAIN_ARRAY_DATA;

typedef struct
{
    PSTRING baseDN;
    PSTRING query;
    PSTRING connection;
    SSO_LONG* elapsedMillis;
    INTEGER* count;
} IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA;

typedef struct
{
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA** ppEntry;
    size_t length;
} IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA;

typedef struct
{
    PSTRING name;
    PSTRING type;
    bool* matchingRuleInChainEnabled;
    bool* baseDnForNestedGroupsEnabled;
    bool* directGroupsSearchEnabled;
    bool* siteAffinityEnabled;
} IDM_EVENT_LOG_PROVIDER_METADATA_DATA;

typedef struct
{
    PSTRING username;
    IDM_EVENT_LOG_PROVIDER_METADATA_DATA* provider;
    IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA* ldapQueryStats;
    IDM_STRING_MAP_DATA* extensions;
} IDM_EVENT_LOG_METADATA_DATA;

typedef struct
{
    PSTRING type;
    PSTRING correlationId;
    PSTRING level;
    SSO_LONG* start;
    SSO_LONG* elapsedMillis;
    IDM_EVENT_LOG_METADATA_DATA* metadata;
} IDM_EVENT_LOG_DATA;

typedef struct
{
    IDM_EVENT_LOG_DATA** ppEntry;
    size_t length;
} IDM_EVENT_LOG_ARRAY_DATA;

typedef struct
{
    bool* enabled;
    SSO_LONG* size;
} IDM_EVENT_LOG_STATUS_DATA;

typedef struct
{
    PSTRING name;
    PSTRING endpoint;
    PSTRING binding;
} IDM_SERVICE_ENDPOINT_DATA;

typedef struct
{
    IDM_SERVICE_ENDPOINT_DATA** ppEntry;
    size_t length;
} IDM_SERVICE_ENDPOINT_ARRAY_DATA;

typedef struct
{
    PSTRING claimName;
    PSTRING claimValue;
    IDM_STRING_ARRAY_DATA* groups;
} IDM_TOKEN_CLAIM_GROUP_DATA;

typedef struct
{
    IDM_TOKEN_CLAIM_GROUP_DATA** ppEntry;
    size_t length;
} IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA;

typedef struct
{
    PSTRING entityID;
    PSTRING alias;
    IDM_STRING_ARRAY_DATA* nameIDFormats;
    IDM_SERVICE_ENDPOINT_ARRAY_DATA* ssoServices;
    IDM_SERVICE_ENDPOINT_ARRAY_DATA* sloServices;
    IDM_CERTIFICATE_CHAIN_DATA* signingCertificates;
    IDM_STRING_MAP_DATA* subjectFormats;
    IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA* tokenClaimGroups;
    bool* jitEnabled;
    PSTRING upnSuffix;
} IDM_EXTERNAL_IDP_DATA;

typedef struct
{
    IDM_EXTERNAL_IDP_DATA** ppEntry;
    size_t length;
} IDM_EXTERNAL_IDP_ARRAY_DATA;

typedef struct
{
    PSTRING description;
} IDM_GROUP_DETAILS_DATA;

typedef struct
{
    PSTRING name;
    PSTRING domain;
} IDM_PRINCIPAL_DATA;

typedef struct
{
    PSTRING name;
    PSTRING domain;
    IDM_GROUP_DETAILS_DATA* details;
    IDM_PRINCIPAL_DATA* alias;
    PSTRING objectId;
} IDM_GROUP_DATA;

typedef struct
{
    IDM_GROUP_DATA** ppEntry;
    size_t length;
} IDM_GROUP_ARRAY_DATA;

typedef struct
{
    PSTRING objectClass;
    IDM_STRING_MAP_DATA* attributeMappings;
} IDM_SCHEMA_OBJECT_MAPPING_DATA;

typedef struct
{
    PSTRING key;
    IDM_SCHEMA_OBJECT_MAPPING_DATA* value;
} IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA;

typedef struct
{
    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA** ppEntry;
    size_t length;
} IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA;

typedef struct
{
    PSTRING domainType;
    PSTRING name;

    // The following params are required only for external identity source. i.e If DomainType is DomainType#External.
    PSTRING alias;
    PSTRING type;
    PSTRING authenticationType;
    PSTRING friendlyName;
    SSO_LONG* searchTimeOutInSeconds;
    PSTRING username;
    PSTRING password;
    bool* machineAccount;
    PSTRING servicePrincipalName;
    PSTRING userBaseDN;
    PSTRING groupBaseDN;
    IDM_STRING_ARRAY_DATA* connectionStrings;
    IDM_STRING_MAP_DATA* attributesMap;
    IDM_STRING_ARRAY_DATA* upnSuffixes;
    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* schema;
    bool* matchingRuleInChainEnabled;
    bool* baseDnForNestedGroupsEnabled;
    bool* directGroupsSearchEnabled;
    bool* siteAffinityEnabled;
    REST_CERTIFICATE_ARRAY_DATA* certificates;
} IDM_IDENTITY_PROVIDER_DATA;

typedef struct
{
    IDM_IDENTITY_PROVIDER_DATA** ppEntry;
    size_t length;
} IDM_IDENTITY_PROVIDER_ARRAY_DATA;

typedef struct
{
    PSTRING description;
    SSO_LONG* failedAttemptIntervalSec;
    INTEGER* maxFailedAttempts;
    SSO_LONG* autoUnlockIntervalSec;
} IDM_LOCKOUT_POLICY_DATA;

typedef struct
{
    IDM_STRING_ARRAY_DATA* redirectUris;
    PSTRING tokenEndpointAuthMethod;
    IDM_STRING_ARRAY_DATA* postLogoutRedirectUris;
    PSTRING logoutUri;
    PSTRING certSubjectDN;
    SSO_LONG* authnRequestClientAssertionLifetimeMS;
} IDM_OIDC_CLIENT_METADATA_DATA;

typedef struct
{
    PSTRING clientId;
    IDM_OIDC_CLIENT_METADATA_DATA* oidcclientMetadataDTO;
} IDM_OIDC_CLIENT_DATA;

typedef struct
{
    IDM_OIDC_CLIENT_DATA** ppEntry;
    size_t length;
} IDM_OIDC_CLIENT_ARRAY_DATA;

typedef struct
{
    PSTRING password;
    SSO_LONG* lastSet;
    SSO_LONG* lifetime;
} IDM_PASSWORD_DETAILS_DATA;

typedef struct
{
    PSTRING description;
    INTEGER* maxIdenticalAdjacentCharacters;
    INTEGER* maxLength;
    INTEGER* minAlphabeticCount;
    INTEGER* minLength;
    INTEGER* minLowercaseCount;
    INTEGER* minNumericCount;
    INTEGER* minSpecialCharCount;
    INTEGER* minUppercaseCount;
    INTEGER* passwordLifetimeDays;
    INTEGER* prohibitedPreviousPasswordCount;
} IDM_PASSWORD_POLICY_DATA;

typedef struct
{
    PSTRING currentPassword;
    PSTRING newPassword;
} IDM_PASSWORD_RESET_REQUEST_DATA;

typedef struct
{
    PSTRING encoded;
    PSTRING algorithm;
} IDM_PRIVATE_KEY_DATA;

typedef struct
{
    PSTRING defaultProvider;
    PSTRING defaultProviderAlias;
    bool* providerSelectionEnabled;
} IDM_PROVIDER_POLICY_DATA;

typedef struct
{
    INTEGER* maxKeySize;
    INTEGER* minKeySize;
    INTEGER* priority;
} IDM_SIGNATURE_ALGORITHM_DATA;

typedef struct
{
    IDM_SIGNATURE_ALGORITHM_DATA** ppEntry;
    size_t length;
} IDM_SIGNATURE_ALGORITHM_ARRAY_DATA;

typedef struct
{
    PSTRING name;
    PSTRING url;
    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* signatureAlgorithms;
    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* assertionConsumerServices;
    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* attributeConsumerServices;
    IDM_SERVICE_ENDPOINT_ARRAY_DATA* singleLogoutServices;
    REST_CERTIFICATE_DATA* certificate;
    PSTRING defaultAssertionConsumerService;
    PSTRING defaultAttributeConsumerService;
    bool* authnRequestsSigned;
} IDM_RELYING_PARTY_DATA;

typedef struct
{
    IDM_RELYING_PARTY_DATA** ppEntry;
    size_t length;
} IDM_RELYING_PARTY_ARRAY_DATA;

typedef struct
{
    PSTRING name;
    IDM_STRING_ARRAY_DATA* groupFilter;
} IDM_RESOURCE_SERVER_DATA;

typedef struct
{
    IDM_RESOURCE_SERVER_DATA** ppEntry;
    size_t length;
} IDM_RESOURCE_SERVER_ARRAY_DATA;

typedef struct
{
    PSTRING email;
    PSTRING upn;
    PSTRING firstName;
    PSTRING lastName;
    PSTRING description;
} IDM_USER_DETAILS_DATA;

typedef struct
{
    PSTRING name;
    PSTRING domain;
    IDM_PRINCIPAL_DATA* alias;
    IDM_USER_DETAILS_DATA* details;
    bool* disabled;
    bool* locked;
    PSTRING objectId;
    IDM_PASSWORD_DETAILS_DATA* passwordDetails;
} IDM_USER_DATA;

typedef struct
{
    IDM_USER_DATA** ppEntry;
    size_t length;
} IDM_USER_ARRAY_DATA;

typedef struct
{
    PSTRING name;
    PSTRING domain;
    PSTRING description;
    IDM_PRINCIPAL_DATA* alias;
    REST_CERTIFICATE_DATA* certificate;
    bool* disabled;
    PSTRING objectId;
} IDM_SOLUTION_USER_DATA;

typedef struct
{
    IDM_SOLUTION_USER_DATA** ppEntry;
    size_t length;
} IDM_SOLUTION_USER_ARRAY_DATA;

typedef struct
{
    IDM_USER_ARRAY_DATA* users;
    IDM_GROUP_ARRAY_DATA* groups;
    IDM_SOLUTION_USER_ARRAY_DATA* solutionUsers;
} IDM_SEARCH_RESULT_DATA;

typedef struct
{
    PSTRING hostname;
    bool* domainController;
} IDM_SERVER_DETAILS_DATA;

typedef struct
{
    IDM_SERVER_DETAILS_DATA** ppEntry;
    size_t length;
} IDM_SERVER_DETAILS_ARRAY_DATA;

typedef struct
{
    SSO_LONG* clockToleranceMillis;
    INTEGER* delegationCount;
    SSO_LONG* maxBearerTokenLifeTimeMillis;
    SSO_LONG* maxHOKTokenLifeTimeMillis;
    SSO_LONG* maxBearerRefreshTokenLifeTimeMillis;
    SSO_LONG* maxHOKRefreshTokenLifeTimeMillis;
    INTEGER* renewCount;
} IDM_TOKEN_POLICY_DATA;

typedef struct
{
    IDM_PASSWORD_POLICY_DATA* passwordPolicy;
    IDM_LOCKOUT_POLICY_DATA* lockoutPolicy;
    IDM_TOKEN_POLICY_DATA* tokenPolicy;
    IDM_PROVIDER_POLICY_DATA* providerPolicy;
    IDM_BRAND_POLICY_DATA* brandPolicy;
    IDM_AUTHENTICATION_POLICY_DATA* authenticationPolicy;
} IDM_TENANT_CONFIGURATION_DATA;

typedef struct
{
    IDM_PRIVATE_KEY_DATA* privateKey;
    REST_CERTIFICATE_ARRAY_DATA* certificates;
} IDM_TENANT_CREDENTIALS_DATA;

typedef struct
{
    PSTRING name;
    PSTRING longName;
    PSTRING key;
    PSTRING guid;
    PSTRING issuer;
    IDM_TENANT_CREDENTIALS_DATA* credentials;
    PSTRING username;
    PSTRING password;
} IDM_TENANT_DATA;

// Data APIs

SSOERROR
IdmStringArrayDataNew(
    IDM_STRING_ARRAY_DATA** ppStrings,
    PSTRING* ppEntry,
    size_t length);

void
IdmStringArrayDataDelete(
    IDM_STRING_ARRAY_DATA* pStrings);

SSOERROR
IdmStringMapEntryDataNew(
    IDM_STRING_MAP_ENTRY_DATA** ppStringMapEntry,
    PCSTRING key,
    PCSTRING value);

void
IdmStringMapEntryDataDelete(
    IDM_STRING_MAP_ENTRY_DATA* pStringMapEntry);

SSOERROR
IdmStringMapDataNew(
    IDM_STRING_MAP_DATA** ppStringMap,
    IDM_STRING_MAP_ENTRY_DATA** ppEntry,
    size_t length);

void
IdmStringMapDataDelete(
    IDM_STRING_MAP_DATA* pStringMap);

SSOERROR
IdmPrivateKeyDataNew(
    IDM_PRIVATE_KEY_DATA** ppPrivateKey,
    PCSTRING encoded,
    PCSTRING algorithm);

void
IdmPrivateKeyDataDelete(
    IDM_PRIVATE_KEY_DATA* pPrivateKey);

SSOERROR
IdmTenantCredentialsDataNew(
    IDM_TENANT_CREDENTIALS_DATA** ppTenantCredentials,
    const IDM_PRIVATE_KEY_DATA* privateKey,
    const REST_CERTIFICATE_ARRAY_DATA* certificates);

void
IdmTenantCredentialsDataDelete(
    IDM_TENANT_CREDENTIALS_DATA* pTenantCredentials);

SSOERROR
IdmTenantDataNew(
    IDM_TENANT_DATA** ppTenant,
    PCSTRING name,
    PCSTRING longName,
    PCSTRING key,
    PCSTRING guid,
    PCSTRING issuer,
    const IDM_TENANT_CREDENTIALS_DATA* tenantCredentials,
    PCSTRING username,
    PCSTRING password);

void
IdmTenantDataDelete(
    IDM_TENANT_DATA* pTenant);

SSOERROR
IdmPasswordPolicyDataNew(
    IDM_PASSWORD_POLICY_DATA** ppPasswordPolicy,
    PCSTRING description,
    const INTEGER* maxIdenticalAdjacentCharacters,
    const INTEGER* maxLength,
    const INTEGER* minAlphabeticCount,
    const INTEGER* minLength,
    const INTEGER* minLowercaseCount,
    const INTEGER* minNumericCount,
    const INTEGER* minSpecialCharCount,
    const INTEGER* minUppercaseCount,
    const INTEGER* passwordLifetimeDays,
    const INTEGER* prohibitedPreviousPasswordCount);

void
IdmPasswordPolicyDataDelete(
    IDM_PASSWORD_POLICY_DATA* pPasswordPolicy);

SSOERROR
IdmLockoutPolicyDataNew(
    IDM_LOCKOUT_POLICY_DATA** ppLockoutPolicy,
    PCSTRING description,
    const SSO_LONG* failedAttemptIntervalSec,
    const INTEGER* maxFailedAttempts,
    const SSO_LONG* autoUnlockIntervalSec);

void
IdmLockoutPolicyDataDelete(
    IDM_LOCKOUT_POLICY_DATA* pLockoutPolicy);

SSOERROR
IdmTokenPolicyDataNew(
    IDM_TOKEN_POLICY_DATA** ppTokenPolicy,
    const SSO_LONG* clockToleranceMillis,
    const INTEGER* delegationCount,
    const SSO_LONG* maxBearerTokenLifeTimeMillis,
    const SSO_LONG* maxHOKTokenLifeTimeMillis,
    const SSO_LONG* maxBearerRefreshTokenLifeTimeMillis,
    const SSO_LONG* maxHOKRefreshTokenLifeTimeMillis,
    const INTEGER* renewCount);

void
IdmTokenPolicyDataDelete(
    IDM_TOKEN_POLICY_DATA* pTokenPolicy);

SSOERROR
IdmProviderPolicyDataNew(
    IDM_PROVIDER_POLICY_DATA** ppProviderPolicy,
    PCSTRING defaultProvider,
    PCSTRING defaultProviderAlias,
    const bool* providerSelectionEnabled);

void
IdmProviderPolicyDataDelete(
    IDM_PROVIDER_POLICY_DATA* pProviderPolicy);

SSOERROR
IdmBrandPolicyDataNew(
    IDM_BRAND_POLICY_DATA** ppBrandPolicy,
    PCSTRING name,
    PCSTRING logonBannerTitle,
    PCSTRING logonBannerContent,
    const bool* logonBannerCheckboxEnabled,
    const bool* logonBannerDisabled);

void
IdmBrandPolicyDataDelete(
    IDM_BRAND_POLICY_DATA* pBrandPolicy);

SSOERROR
IdmAuthenticationPolicyDataNew(
    IDM_AUTHENTICATION_POLICY_DATA** ppAuthenticationPolicy,
    const bool* passwordBasedAuthenticationEnabled,
    const bool* windowsBasedAuthenticationEnabled,
    const bool* certificateBasedAuthenticationEnabled,
    const IDM_CLIENT_CERTIFICATE_POLICY_DATA* clientCertificatePolicy);

void
IdmAuthenticationPolicyDataDelete(
    IDM_AUTHENTICATION_POLICY_DATA* pAuthenticationPolicy);

SSOERROR
IdmClientCertificatePolicyDataNew(
    IDM_CLIENT_CERTIFICATE_POLICY_DATA** ppClientCertificatePolicy,
    const IDM_STRING_ARRAY_DATA* certPolicyOIDs,
    const REST_CERTIFICATE_ARRAY_DATA* trustedCACertificates,
    const bool* revocationCheckEnabled,
    const bool* ocspEnabled,
    const bool* failOverToCrlEnabled,
    PCSTRING ocspUrlOverride,
    const bool* crlDistributionPointUsageEnabled,
    PCSTRING crlDistributionPointOverride);

void
IdmClientCertificatePolicyDataDelete(
    IDM_CLIENT_CERTIFICATE_POLICY_DATA* pClientCertificatePolicy);

SSOERROR
IdmTenantConfigurationDataNew(
    IDM_TENANT_CONFIGURATION_DATA** ppTenantConfiguration,
    const IDM_PASSWORD_POLICY_DATA* passwordPolicy,
    const IDM_LOCKOUT_POLICY_DATA* lockoutPolicy,
    const IDM_TOKEN_POLICY_DATA* tokenPolicy,
    const IDM_PROVIDER_POLICY_DATA* providerPolicy,
    const IDM_BRAND_POLICY_DATA* brandPolicy,
    const IDM_AUTHENTICATION_POLICY_DATA* authenticationPolicy);

void
IdmTenantConfigurationDataDelete(
    IDM_TENANT_CONFIGURATION_DATA* pTenantConfiguration);

SSOERROR
IdmPrincipalDataNew(
    IDM_PRINCIPAL_DATA** ppPrincipal,
    PCSTRING name,
    PCSTRING domain);

void
IdmPrincipalDataDelete(
    IDM_PRINCIPAL_DATA* pPrincipal);

SSOERROR
IdmUserDetailsDataNew(
    IDM_USER_DETAILS_DATA** ppUserDetails,
    PCSTRING email,
    PCSTRING upn,
    PCSTRING firstName,
    PCSTRING lastName,
    PCSTRING description);

void
IdmUserDetailsDataDelete(
    IDM_USER_DETAILS_DATA* pUserDetails);

SSOERROR
IdmPasswordDetailsDataNew(
    IDM_PASSWORD_DETAILS_DATA** ppPasswordDetails,
    PCSTRING password,
    const SSO_LONG* lastSet,
    const SSO_LONG* lifetime);

void
IdmPasswordDetailsDataDelete(
    IDM_PASSWORD_DETAILS_DATA* pPasswordDetails);

SSOERROR
IdmUserDataNew(
    IDM_USER_DATA** ppUser,
    PCSTRING name,
    PCSTRING domain,
    const IDM_PRINCIPAL_DATA* alias,
    const IDM_USER_DETAILS_DATA* details,
    const bool* disabled,
    const bool* locked,
    PCSTRING objectId,
    const IDM_PASSWORD_DETAILS_DATA* passwordDetails);

void
IdmUserDataDelete(
    IDM_USER_DATA* pUser);

SSOERROR
IdmAssertionConsumerServiceDataNew(
    IDM_ASSERTION_CONSUMER_SERVICE_DATA** ppAssertionConsumerService,
    PCSTRING name,
    PCSTRING endpoint,
    PCSTRING binding,
    const INTEGER* index);

void
IdmAssertionConsumerServiceDataDelete(
    IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerService);

SSOERROR
IdmAssertionConsumerServiceArrayDataNew(
    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA** ppAssertionConsumerServiceArray,
    IDM_ASSERTION_CONSUMER_SERVICE_DATA** ppEntry,
    size_t length);

void
IdmAssertionConsumerServiceArrayDataDelete(
    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* pAssertionConsumerServiceArray);

SSOERROR
IdmAttributeDataNew(
    IDM_ATTRIBUTE_DATA** ppAttribute,
    PCSTRING name,
    PCSTRING friendlyName,
    PCSTRING nameFormat);

void
IdmAttributeDataDelete(
    IDM_ATTRIBUTE_DATA* pAttribute);

SSOERROR
IdmAttributeArrayDataNew(
    IDM_ATTRIBUTE_ARRAY_DATA** ppAttributeArray,
    IDM_ATTRIBUTE_DATA** ppEntry,
    size_t length);

void
IdmAttributeArrayDataDelete(
    IDM_ATTRIBUTE_ARRAY_DATA* pAttributeArray);

SSOERROR
IdmAttributeConsumerServiceDataNew(
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA** ppAttributeConsumerService,
    PCSTRING name,
    const INTEGER* index,
    const IDM_ATTRIBUTE_ARRAY_DATA* attributes);

void
IdmAttributeConsumerServiceDataDelete(
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerService);

SSOERROR
IdmAttributeConsumerServiceArrayDataNew(
    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA** ppAttributeConsumerServiceArray,
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA** ppEntry,
    size_t length);

void
IdmAttributeConsumerServiceArrayDataDelete(
    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* pAttributeConsumerServiceArray);

SSOERROR
IdmCertificateChainDataNew(
    IDM_CERTIFICATE_CHAIN_DATA** ppCertificateChain,
    const REST_CERTIFICATE_ARRAY_DATA* certificates);

void
IdmCertificateChainDataDelete(
    IDM_CERTIFICATE_CHAIN_DATA* pCertificateChain);

SSOERROR
IdmCertificateChainArrayDataNew(
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA** ppCertificateChainArray,
    IDM_CERTIFICATE_CHAIN_DATA** ppEntry,
    size_t length);

void
IdmCertificateChainArrayDataDelete(
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA* pCertificateChainArray);

SSOERROR
IdmEventLogLdapQueryStatDataNew(
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA** ppEventLogLdapQueryStat,
    PCSTRING baseDN,
    PCSTRING query,
    PCSTRING connection,
    const SSO_LONG* elapsedMillis,
    const INTEGER* count);

void
IdmEventLogLdapQueryStatDataDelete(
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA* pEventLogLdapQueryStat);

SSOERROR
IdmEventLogLdapQueryStatArrayDataNew(
    IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA** ppEventLogLdapQueryStatArray,
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA** ppEntry,
    size_t length);

void
IdmEventLogLdapQueryStatArrayDataDelete(
    IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA* pEventLogLdapQueryStatArray);

SSOERROR
IdmEventLogProviderMetadataDataNew(
    IDM_EVENT_LOG_PROVIDER_METADATA_DATA** ppEventLogProviderMetadata,
    PCSTRING name,
    PCSTRING type,
    const bool* matchingRuleInChainEnabled,
    const bool* baseDnForNestedGroupsEnabled,
    const bool* directGroupsSearchEnabled,
    const bool* siteAffinityEnabled);

void
IdmEventLogProviderMetadataDataDelete(
    IDM_EVENT_LOG_PROVIDER_METADATA_DATA* pEventLogProviderMetadata);

SSOERROR
IdmEventLogMetadataDataNew(
    IDM_EVENT_LOG_METADATA_DATA** ppEventLogMetadata,
    PCSTRING username,
    const IDM_EVENT_LOG_PROVIDER_METADATA_DATA* provider,
    const IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA* ldapQueryStats,
    const IDM_STRING_MAP_DATA* extensions);

void
IdmEventLogMetadataDataDelete(
    IDM_EVENT_LOG_METADATA_DATA* pEventLogMetadata);

SSOERROR
IdmEventLogDataNew(
    IDM_EVENT_LOG_DATA** ppEventLog,
    PCSTRING type,
    PCSTRING correlationId,
    PCSTRING level,
    const SSO_LONG* start,
    const SSO_LONG* elapsedMillis,
    const IDM_EVENT_LOG_METADATA_DATA* metadata);

void
IdmEventLogDataDelete(
    IDM_EVENT_LOG_DATA* pEventLog);

SSOERROR
IdmEventLogArrayDataNew(
    IDM_EVENT_LOG_ARRAY_DATA** ppEventLogArray,
    IDM_EVENT_LOG_DATA** ppEntry,
    size_t length);

void
IdmEventLogArrayDataDelete(
    IDM_EVENT_LOG_ARRAY_DATA* pEventLogArray);

SSOERROR
IdmEventLogStatusDataNew(
    IDM_EVENT_LOG_STATUS_DATA** ppEventLogStatus,
    const bool* enabled,
    const SSO_LONG* size);

void
IdmEventLogStatusDataDelete(
    IDM_EVENT_LOG_STATUS_DATA* pEventLogStatus);

SSOERROR
IdmServiceEndpointDataNew(
    IDM_SERVICE_ENDPOINT_DATA** ppServiceEndpoint,
    PCSTRING name,
    PCSTRING endpoint,
    PCSTRING binding);

void
IdmServiceEndpointDataDelete(
    IDM_SERVICE_ENDPOINT_DATA* pServiceEndpoint);

SSOERROR
IdmServiceEndpointArrayDataNew(
    IDM_SERVICE_ENDPOINT_ARRAY_DATA** ppServiceEndpointArray,
    IDM_SERVICE_ENDPOINT_DATA** ppEntry,
    size_t length);

void
IdmServiceEndpointArrayDataDelete(
    IDM_SERVICE_ENDPOINT_ARRAY_DATA* pServiceEndpointArray);

SSOERROR
IdmTokenClaimGroupDataNew(
    IDM_TOKEN_CLAIM_GROUP_DATA** ppTokenClaimGroup,
    PCSTRING claimName,
    PCSTRING claimValue,
    const IDM_STRING_ARRAY_DATA* groups);

void
IdmTokenClaimGroupDataDelete(
    IDM_TOKEN_CLAIM_GROUP_DATA* pTokenClaimGroup);

SSOERROR
IdmTokenClaimGroupArrayDataNew(
    IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA** ppTokenClaimGroupArray,
    IDM_TOKEN_CLAIM_GROUP_DATA** ppEntry,
    size_t length);

void
IdmTokenClaimGroupArrayDataDelete(
    IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA* pTokenClaimGroupArray);

SSOERROR
IdmExternalIdpDataNew(
    IDM_EXTERNAL_IDP_DATA** ppExternalIdp,
    PCSTRING entityID,
    PCSTRING alias,
    const IDM_STRING_ARRAY_DATA* nameIDFormats,
    const IDM_SERVICE_ENDPOINT_ARRAY_DATA* ssoServices,
    const IDM_SERVICE_ENDPOINT_ARRAY_DATA* sloServices,
    const IDM_CERTIFICATE_CHAIN_DATA* signingCertificates,
    const IDM_STRING_MAP_DATA* subjectFormats,
    const IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA* tokenClaimGroups,
    const bool* jitEnabled,
    PCSTRING upnSuffix);

void
IdmExternalIdpDataDelete(
    IDM_EXTERNAL_IDP_DATA* pExternalIdp);

SSOERROR
IdmExternalIdpArrayDataNew(
    IDM_EXTERNAL_IDP_ARRAY_DATA** ppExternalIdpArray,
    IDM_EXTERNAL_IDP_DATA** ppEntry,
    size_t length);

void
IdmExternalIdpArrayDataDelete(
    IDM_EXTERNAL_IDP_ARRAY_DATA* pExternalIdpArray);

SSOERROR
IdmGroupDetailsDataNew(
    IDM_GROUP_DETAILS_DATA** ppGroupDetails,
    PCSTRING description);

void
IdmGroupDetailsDataDelete(
    IDM_GROUP_DETAILS_DATA* pGroupDetails);

SSOERROR
IdmGroupDataNew(
    IDM_GROUP_DATA** ppGroup,
    PCSTRING name,
    PCSTRING domain,
    const IDM_GROUP_DETAILS_DATA* details,
    const IDM_PRINCIPAL_DATA* alias,
    PCSTRING objectId);

void
IdmGroupDataDelete(
    IDM_GROUP_DATA* pGroup);

SSOERROR
IdmGroupArrayDataNew(
    IDM_GROUP_ARRAY_DATA** ppGroupArray,
    IDM_GROUP_DATA** ppEntry,
    size_t length);

void
IdmGroupArrayDataDelete(
    IDM_GROUP_ARRAY_DATA* pGroupArray);

SSOERROR
IdmSchemaObjectMappingDataNew(
    IDM_SCHEMA_OBJECT_MAPPING_DATA** ppSchemaObjectMapping,
    PCSTRING objectClass,
    const IDM_STRING_MAP_DATA* attributeMappings);

void
IdmSchemaObjectMappingDataDelete(
    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMapping);

SSOERROR
IdmSchemaObjectMappingMapEntryDataNew(
    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA** ppSchemaObjectMappingMapEntry,
    PCSTRING key,
    const IDM_SCHEMA_OBJECT_MAPPING_DATA* value);

void
IdmSchemaObjectMappingMapEntryDataDelete(
    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaObjectMappingMapEntry);

SSOERROR
IdmSchemaObjectMappingMapDataNew(
    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA** ppSchemaObjectMappingMap,
    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA** ppEntry,
    size_t length);

void
IdmSchemaObjectMappingMapDataDelete(
    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* pSchemaObjectMappingMap);

SSOERROR
IdmIdentityProviderDataNew(
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProvider,
    PCSTRING domainType,
    PCSTRING name,
    PCSTRING alias,
    PCSTRING type,
    PCSTRING authenticationType,
    PCSTRING friendlyName,
    const SSO_LONG* searchTimeOutInSeconds,
    PCSTRING username,
    PCSTRING password,
    const bool* machineAccount,
    PCSTRING servicePrincipalName,
    PCSTRING userBaseDN,
    PCSTRING groupBaseDN,
    const IDM_STRING_ARRAY_DATA* connectionStrings,
    const IDM_STRING_MAP_DATA* attributesMap,
    const IDM_STRING_ARRAY_DATA* upnSuffixes,
    const IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* schema,
    const bool* matchingRuleInChainEnabled,
    const bool* baseDnForNestedGroupsEnabled,
    const bool* directGroupsSearchEnabled,
    const bool* siteAffinityEnabled,
    const REST_CERTIFICATE_ARRAY_DATA* certificates);

SSOERROR
IdmIdentityProviderArrayDataNew(
    IDM_IDENTITY_PROVIDER_ARRAY_DATA** ppIdentityProviderArray,
    IDM_IDENTITY_PROVIDER_DATA** ppEntry,
    size_t length);

void
IdmIdentityProviderArrayDataDelete(
    IDM_IDENTITY_PROVIDER_ARRAY_DATA* pIdentityProviderArray);

void
IdmIdentityProviderDataDelete(
    IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider);

SSOERROR
IdmOidcClientMetadataDataNew(
    IDM_OIDC_CLIENT_METADATA_DATA** ppOidcClientMetadata,
    const IDM_STRING_ARRAY_DATA* redirectUris,
    PCSTRING tokenEndpointAuthMethod,
    const IDM_STRING_ARRAY_DATA* postLogoutRedirectUris,
    PCSTRING logoutUri,
    PCSTRING certSubjectDN,
    const SSO_LONG* authnRequestClientAssertionLifetimeMS);

void
IdmOidcClientMetadataDataDelete(
    IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata);

SSOERROR
IdmOidcClientDataNew(
    IDM_OIDC_CLIENT_DATA** ppOidcClient,
    PCSTRING clientId,
    const IDM_OIDC_CLIENT_METADATA_DATA* oidcClientMetadata);

void
IdmOidcClientDataDelete(
    IDM_OIDC_CLIENT_DATA* pOidcClient);

SSOERROR
IdmOidcClientArrayDataNew(
    IDM_OIDC_CLIENT_ARRAY_DATA** ppOidcClientArray,
    IDM_OIDC_CLIENT_DATA** ppEntry,
    size_t length);

void
IdmOidcClientArrayDataDelete(
    IDM_OIDC_CLIENT_ARRAY_DATA* pOidcClientArray);

SSOERROR
IdmPasswordResetRequestDataNew(
    IDM_PASSWORD_RESET_REQUEST_DATA** ppPasswordResetRequest,
    PCSTRING currentPassword,
    PCSTRING newPassword);

void
IdmPasswordResetRequestDataDelete(
    IDM_PASSWORD_RESET_REQUEST_DATA* pPasswordResetRequest);

SSOERROR
IdmSignatureAlgorithmDataNew(
    IDM_SIGNATURE_ALGORITHM_DATA** ppSignatureAlgorithm,
    const INTEGER* maxKeySize,
    const INTEGER* minKeySize,
    const INTEGER* priority);

void
IdmSignatureAlgorithmDataDelete(
    IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithm);

SSOERROR
IdmSignatureAlgorithmArrayDataNew(
    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA** ppSignatureAlgorithmArray,
    IDM_SIGNATURE_ALGORITHM_DATA** ppEntry,
    size_t length);

void
IdmSignatureAlgorithmArrayDataDelete(
    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* pSignatureAlgorithmArray);

SSOERROR
IdmRelyingPartyDataNew(
    IDM_RELYING_PARTY_DATA** ppRelyingParty,
    PCSTRING name,
    PCSTRING url,
    const IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* signatureAlgorithms,
    const IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* assertionConsumerServices,
    const IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* attributeConsumerServices,
    const IDM_SERVICE_ENDPOINT_ARRAY_DATA* singleLogoutServices,
    const REST_CERTIFICATE_DATA* certificate,
    PCSTRING defaultAssertionConsumerService,
    PCSTRING defaultAttributeConsumerService,
    const bool* authnRequestsSigned);

void
IdmRelyingPartyDataDelete(
    IDM_RELYING_PARTY_DATA* pRelyingParty);

SSOERROR
IdmRelyingPartyArrayDataNew(
    IDM_RELYING_PARTY_ARRAY_DATA** ppRelyingPartyArray,
    IDM_RELYING_PARTY_DATA** ppEntry,
    size_t length);

void
IdmRelyingPartyArrayDataDelete(
    IDM_RELYING_PARTY_ARRAY_DATA* pRelyingPartyArray);

SSOERROR
IdmResourceServerDataNew(
    IDM_RESOURCE_SERVER_DATA** ppResourceServer,
    PCSTRING name,
    const IDM_STRING_ARRAY_DATA* groupFilter);

void
IdmResourceServerDataDelete(
    IDM_RESOURCE_SERVER_DATA* pResourceServer);

SSOERROR
IdmResourceServerArrayDataNew(
    IDM_RESOURCE_SERVER_ARRAY_DATA** ppResourceServerArray,
    IDM_RESOURCE_SERVER_DATA** ppEntry,
    size_t length);

void
IdmResourceServerArrayDataDelete(
    IDM_RESOURCE_SERVER_ARRAY_DATA* pResourceServerArray);

SSOERROR
IdmUserArrayDataNew(
    IDM_USER_ARRAY_DATA** ppUserArray,
    IDM_USER_DATA** ppEntry,
    size_t length);

void
IdmUserArrayDataDelete(
    IDM_USER_ARRAY_DATA* pUserArray);

SSOERROR
IdmSolutionUserDataNew(
    IDM_SOLUTION_USER_DATA** ppSolutionUser,
    PCSTRING name,
    PCSTRING domain,
    PCSTRING description,
    const IDM_PRINCIPAL_DATA* alias,
    const REST_CERTIFICATE_DATA* certificate,
    const bool* disabled,
    PCSTRING objectId);

void
IdmSolutionUserDataDelete(
    IDM_SOLUTION_USER_DATA* pSolutionUser);

SSOERROR
IdmSolutionUserArrayDataNew(
    IDM_SOLUTION_USER_ARRAY_DATA** ppSolutionUserArray,
    IDM_SOLUTION_USER_DATA** ppEntry,
    size_t length);

void
IdmSolutionUserArrayDataDelete(
    IDM_SOLUTION_USER_ARRAY_DATA* pSolutionUserArray);

SSOERROR
IdmSearchResultDataNew(
    IDM_SEARCH_RESULT_DATA** ppSearchResult,
    const IDM_USER_ARRAY_DATA* users,
    const IDM_GROUP_ARRAY_DATA* groups,
    const IDM_SOLUTION_USER_ARRAY_DATA* solutionUsers);

void
IdmSearchResultDataDelete(
    IDM_SEARCH_RESULT_DATA* pSearchResult);

SSOERROR
IdmServerDetailsDataNew(
    IDM_SERVER_DETAILS_DATA** ppServerDetails,
    PCSTRING hostname,
    const bool* domainController);

void
IdmServerDetailsDataDelete(
    IDM_SERVER_DETAILS_DATA* pServerDetails);

SSOERROR
IdmServerDetailsArrayDataNew(
    IDM_SERVER_DETAILS_ARRAY_DATA** ppServerDetailsArray,
    IDM_SERVER_DETAILS_DATA** ppEntry,
    size_t length);

void
IdmServerDetailsArrayDataDelete(
    IDM_SERVER_DETAILS_ARRAY_DATA* pServerDetailsArray);

// Resource APIs

SSOERROR
IdmCertificateGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_CERTIFICATE_SCOPE_TYPE certificateScopeType,
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA** ppCertificateChainArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmCertificateDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING fingerprint,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmCertificateGetPrivateKey(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_PRIVATE_KEY_DATA** ppPrivateKeyReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmCertificateSetCredentials(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_TENANT_CREDENTIALS_DATA* pTenantCredentials,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmDiagnosticsClearEventLog(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmDiagnosticsGetEventLog(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_EVENT_LOG_ARRAY_DATA** ppEventLogArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmDiagnosticsGetEventLogStatus(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_EVENT_LOG_STATUS_DATA** ppEventLogStatusReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmDiagnosticsStartEventLog(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    size_t size,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmDiagnosticsStopEventLog(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmExternalIdpRegister(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_EXTERNAL_IDP_DATA* pExternalIdp,
    IDM_EXTERNAL_IDP_DATA** ppExternalIdpReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmExternalIdpRegisterByMetadata(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING metadata,
    IDM_EXTERNAL_IDP_DATA** ppExternalIdpReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmExternalIdpGetAll(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_EXTERNAL_IDP_ARRAY_DATA** ppExternalIdpArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmExternalIdpGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING entityId,
    IDM_EXTERNAL_IDP_DATA** ppExternalIdpReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmExternalIdpDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING entityId,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmGroupGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    IDM_GROUP_DATA** ppGroupReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmGroupGetMembers(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    IDM_MEMBER_TYPE memberType,
    size_t limit,
    IDM_SEARCH_RESULT_DATA** ppSearchResultReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmGroupGetParents(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    IDM_GROUP_ARRAY_DATA** ppGroupArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmIdentityProviderCreate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider,
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProviderReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmIdentityProviderProbe(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmIdentityProviderGetAll(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_IDENTITY_PROVIDER_ARRAY_DATA** ppIdentityProviderArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmIdentityProviderGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING provider,
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProviderReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmIdentityProviderUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING provider,
    const IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider,
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProviderReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmIdentityProviderDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING provider,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmOidcClientRegister(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata,
    IDM_OIDC_CLIENT_DATA** ppOidcClientReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmOidcClientGetAll(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_OIDC_CLIENT_ARRAY_DATA** ppOidcClientArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmOidcClientGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING clientId,
    IDM_OIDC_CLIENT_DATA** ppOidcClientReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmOidcClientUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING clientId,
    const IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata,
    IDM_OIDC_CLIENT_DATA** ppOidcClientReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmOidcClientDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING clientId,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmRelyingPartyRegister(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_RELYING_PARTY_DATA* pRelyingParty,
    IDM_RELYING_PARTY_DATA** ppRelyingPartyReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmRelyingPartyGetAll(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_RELYING_PARTY_ARRAY_DATA** ppRelyingPartyArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmRelyingPartyGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    IDM_RELYING_PARTY_DATA** ppRelyingPartyReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmRelyingPartyUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    const IDM_RELYING_PARTY_DATA* pRelyingParty,
    IDM_RELYING_PARTY_DATA** ppRelyingPartyReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmRelyingPartyDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmResourceServerRegister(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_RESOURCE_SERVER_DATA* pResourceServer,
    IDM_RESOURCE_SERVER_DATA** ppResourceServerReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmResourceServerGetAll(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_RESOURCE_SERVER_ARRAY_DATA** ppResourceServerArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmResourceServerGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    IDM_RESOURCE_SERVER_DATA** ppResourceServerReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmResourceServerUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    const IDM_RESOURCE_SERVER_DATA* pResourceServer,
    IDM_RESOURCE_SERVER_DATA** ppResourceServerReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmResourceServerDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmServerGetComputers(
    PCREST_CLIENT pClient,
    IDM_COMPUTER_TYPE type,
    IDM_SERVER_DETAILS_ARRAY_DATA** ppServerDetailsArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmSolutionUserGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    IDM_SOLUTION_USER_DATA** ppSolutionUserReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmTenantCreate(
    PCREST_CLIENT pClient,
    const IDM_TENANT_DATA* pTenant,
    IDM_TENANT_DATA** ppTenantReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmTenantGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_TENANT_DATA** ppTenantDataReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmTenantDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmTenantGetConfig(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_TENANT_CONFIG_TYPE_ARRAY* pTenantConfigTypes,
    IDM_TENANT_CONFIGURATION_DATA** ppTenantConfiguration,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmTenantUpdateConfig(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_TENANT_CONFIGURATION_DATA* pTenantConfiguration,
    IDM_TENANT_CONFIGURATION_DATA** ppTenantConfigurationReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmTenantSearch(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING domain,
    PCSTRING query,
    IDM_MEMBER_TYPE memberType,
    IDM_SEARCH_TYPE searchType,
    size_t limit,
    IDM_SEARCH_RESULT_DATA** ppSearchResult,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmExternalUserCreate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_PRINCIPAL_DATA* pPrincipal,
    bool** ppCreated,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmUserGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    IDM_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmUserGetGroups(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    bool nested,
    IDM_GROUP_ARRAY_DATA** ppGroupArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
IdmExternalUserDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    REST_SERVER_ERROR** ppError);

#endif /* INCLUDE_PUBLIC_SSOIDMCLIENT_H_ */
