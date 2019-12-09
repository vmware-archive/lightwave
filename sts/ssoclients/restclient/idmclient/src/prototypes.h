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

#ifndef PROTOTYPES_H_
#define PROTOTYPES_H_

// data object to/from Json

SSOERROR
IdmStringArrayDataToJson(
    const IDM_STRING_ARRAY_DATA* pStrings,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToStringArrayData(
    PCSSO_JSON pJson,
    IDM_STRING_ARRAY_DATA** ppStrings);

SSOERROR
IdmPrivateKeyDataToJson(
    const IDM_PRIVATE_KEY_DATA* pPrivateKeyData,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToPrivateKeyData(
    PCSSO_JSON pJson,
    IDM_PRIVATE_KEY_DATA** ppPrivateKey);

SSOERROR
IdmCertificateChainDataToJson(
    const IDM_CERTIFICATE_CHAIN_DATA* pCertificateChain,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToCertificateChainData(
    PCSSO_JSON pJson,
    IDM_CERTIFICATE_CHAIN_DATA** ppCertificateChain);

SSOERROR
IdmCertificateChainArrayDataToJson(
    const IDM_CERTIFICATE_CHAIN_ARRAY_DATA* pCertificateChainArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToCertificateChainArrayData(
    PCSSO_JSON pJson,
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA** ppCertificateChainArray);

SSOERROR
IdmTenantCredentialsDataToJson(
    const IDM_TENANT_CREDENTIALS_DATA* pTenantCredentialsData,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToTenantCredentialsData(
    PCSSO_JSON pJson,
    IDM_TENANT_CREDENTIALS_DATA** ppTenantCredentialsData);

SSOERROR
IdmTenantDataToJson(
    const IDM_TENANT_DATA* pTenantData,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToTenantData(
    PCSSO_JSON pJson,
    IDM_TENANT_DATA** ppTenantData);

SSOERROR
IdmPasswordPolicyDataToJson(
    const IDM_PASSWORD_POLICY_DATA* pPasswordPolicy,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToPasswordPolicyData(
    PCSSO_JSON pJson,
    IDM_PASSWORD_POLICY_DATA** ppPasswordPolicy);

SSOERROR
IdmLockoutPolicyDataToJson(
    const IDM_LOCKOUT_POLICY_DATA* pLockoutPolicy,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToLockoutPolicyData(
    PCSSO_JSON pJson,
    IDM_LOCKOUT_POLICY_DATA** ppLockoutPolicy);

SSOERROR
IdmTokenPolicyDataToJson(
    const IDM_TOKEN_POLICY_DATA* pTokenPolicy,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToTokenPolicyData(
    PCSSO_JSON pJson,
    IDM_TOKEN_POLICY_DATA** ppTokenPolicy);

SSOERROR
IdmProviderPolicyDataToJson(
    const IDM_PROVIDER_POLICY_DATA* pProviderPolicy,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToProviderPolicyData(
    PCSSO_JSON pJson,
    IDM_PROVIDER_POLICY_DATA** ppProviderPolicy);

SSOERROR
IdmBrandPolicyDataToJson(
    const IDM_BRAND_POLICY_DATA* pBrandPolicy,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToBrandPolicyData(
    PCSSO_JSON pJson,
    IDM_BRAND_POLICY_DATA** ppBrandPolicy);

SSOERROR
IdmAuthenticationPolicyDataToJson(
    const IDM_AUTHENTICATION_POLICY_DATA* pAuthenticationPolicy,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToAuthenticationPolicyData(
    PCSSO_JSON pJson,
    IDM_AUTHENTICATION_POLICY_DATA** ppAuthenticationPolicy);

SSOERROR
IdmClientCertificatePolicyDataToJson(
    const IDM_CLIENT_CERTIFICATE_POLICY_DATA* pClientCertificatePolicy,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToClientCertificatePolicyData(
    PCSSO_JSON pJson,
    IDM_CLIENT_CERTIFICATE_POLICY_DATA** ppClientCertificatePolicy);

SSOERROR
IdmTenantConfigurationDataToJson(
    const IDM_TENANT_CONFIGURATION_DATA* pTenantConfiguration,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToTenantConfigurationData(
    PCSSO_JSON pJson,
    IDM_TENANT_CONFIGURATION_DATA** ppTenantConfiguration);

SSOERROR
IdmPrincipalDataToJson(
    const IDM_PRINCIPAL_DATA* pPrincipal,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToPrincipalData(
    PCSSO_JSON pJson,
    IDM_PRINCIPAL_DATA** ppPrincipal);

SSOERROR
IdmUserDetailsDataToJson(
    const IDM_USER_DETAILS_DATA* pUserDetails,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToUserDetailsData(
    PCSSO_JSON pJson,
    IDM_USER_DETAILS_DATA** ppUserDetails);

SSOERROR
IdmPasswordDetailsDataToJson(
    const IDM_PASSWORD_DETAILS_DATA* pPasswordDetails,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToPasswordDetailsData(
    PCSSO_JSON pJson,
    IDM_PASSWORD_DETAILS_DATA** ppPasswordDetails);

SSOERROR
IdmUserDataToJson(
    const IDM_USER_DATA* pUser,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToUserData(
    PCSSO_JSON pJson,
    IDM_USER_DATA** ppUser);

SSOERROR
IdmAssertionConsumerServiceDataToJson(
    const IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerService,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToAssertionConsumerServiceData(
    PCSSO_JSON pJson,
    IDM_ASSERTION_CONSUMER_SERVICE_DATA** ppAssertionConsumerService);

SSOERROR
IdmAssertionConsumerServiceArrayDataToJson(
    const IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* pAssertionConsumerServiceArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToAssertionConsumerServiceArrayData(
    PCSSO_JSON pJson,
    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA** ppAssertionConsumerServiceArray);

SSOERROR
IdmAttributeDataToJson(
    const IDM_ATTRIBUTE_DATA* pAttribute,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToAttributeData(
    PCSSO_JSON pJson,
    IDM_ATTRIBUTE_DATA** ppAttribute);

SSOERROR
IdmAttributeArrayDataToJson(
    const IDM_ATTRIBUTE_ARRAY_DATA* pAttributeArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToAttributeArrayData(
    PCSSO_JSON pJson,
    IDM_ATTRIBUTE_ARRAY_DATA** ppAttributeArray);

SSOERROR
IdmAttributeConsumerServiceDataToJson(
    const IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerService,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToAttributeConsumerServiceData(
    PCSSO_JSON pJson,
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA** ppAttributeConsumerService);

SSOERROR
IdmAttributeConsumerServiceArrayDataToJson(
    const IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* pAttributeConsumerServiceArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToAttributeConsumerServiceArrayData(
    PCSSO_JSON pJson,
    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA** ppAttributeConsumerServiceArray);

SSOERROR
IdmStringMapDataToJson(
    const IDM_STRING_MAP_DATA* pStringMap,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToStringMapData(
    PCSSO_JSON pJson,
    IDM_STRING_MAP_DATA** ppStringMap);

SSOERROR
IdmEventLogLdapQueryStatDataToJson(
    const IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA* pEventLogLdapQueryStat,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToEventLogLdapQueryStatData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA** ppEventLogLdapQueryStat);

SSOERROR
IdmEventLogLdapQueryStatArrayDataToJson(
    const IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA* pEventLogLdapQueryStatArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToEventLogLdapQueryStatArrayData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA** ppEventLogLdapQueryStatArray);

SSOERROR
IdmEventLogProviderMetadataDataToJson(
    const IDM_EVENT_LOG_PROVIDER_METADATA_DATA* pEventLogProviderMetadata,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToEventLogProviderMetadataData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_PROVIDER_METADATA_DATA** ppEventLogProviderMetadata);

SSOERROR
IdmEventLogMetadataDataToJson(
    const IDM_EVENT_LOG_METADATA_DATA* pEventLogMetadata,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToEventLogMetadataData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_METADATA_DATA** ppEventLogMetadata);

SSOERROR
IdmEventLogDataToJson(
    const IDM_EVENT_LOG_DATA* pEventLog,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToEventLogData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_DATA** ppEventLog);

SSOERROR
IdmEventLogArrayDataToJson(
    const IDM_EVENT_LOG_ARRAY_DATA* pEventLogArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToEventLogArrayData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_ARRAY_DATA** ppEventLogArray);

SSOERROR
IdmEventLogStatusDataToJson(
    const IDM_EVENT_LOG_STATUS_DATA* pEventLogStatus,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToEventLogStatusData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_STATUS_DATA** ppEventLogStatus);

SSOERROR
IdmServiceEndpointDataToJson(
    const IDM_SERVICE_ENDPOINT_DATA* pServiceEndpoint,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToServiceEndpointData(
    PCSSO_JSON pJson,
    IDM_SERVICE_ENDPOINT_DATA** ppServiceEndpoint);

SSOERROR
IdmServiceEndpointArrayDataToJson(
    const IDM_SERVICE_ENDPOINT_ARRAY_DATA* pServiceEndpointArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToServiceEndpointArrayData(
    PCSSO_JSON pJson,
    IDM_SERVICE_ENDPOINT_ARRAY_DATA** ppServiceEndpointArray);

SSOERROR
IdmTokenClaimGroupDataToJson(
    const IDM_TOKEN_CLAIM_GROUP_DATA* pTokenClaimGroup,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToTokenClaimGroupData(
    PCSSO_JSON pJson,
    IDM_TOKEN_CLAIM_GROUP_DATA** ppTokenClaimGroup);

SSOERROR
IdmTokenClaimGroupArrayDataToJson(
    const IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA* pTokenClaimGroupArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToTokenClaimGroupArrayData(
    PCSSO_JSON pJson,
    IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA** ppTokenClaimGroupArray);

SSOERROR
IdmExternalIdpDataToJson(
    const IDM_EXTERNAL_IDP_DATA* pExternalIdp,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToExternalIdpData(
    PCSSO_JSON pJson,
    IDM_EXTERNAL_IDP_DATA** ppExternalIdp);

SSOERROR
IdmExternalIdpArrayDataToJson(
    const IDM_EXTERNAL_IDP_ARRAY_DATA* pExternalIdpArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToExternalIdpArrayData(
    PCSSO_JSON pJson,
    IDM_EXTERNAL_IDP_ARRAY_DATA** ppExternalIdpArray);

SSOERROR
IdmGroupDetailsDataToJson(
    const IDM_GROUP_DETAILS_DATA* pGroupDetails,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToGroupDetailsData(
    PCSSO_JSON pJson,
    IDM_GROUP_DETAILS_DATA** ppGroupDetails);

SSOERROR
IdmGroupDataToJson(
    const IDM_GROUP_DATA* pGroup,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToGroupData(
    PCSSO_JSON pJson,
    IDM_GROUP_DATA** ppGroup);

SSOERROR
IdmGroupArrayDataToJson(
    const IDM_GROUP_ARRAY_DATA* pGroupArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToGroupArrayData(
    PCSSO_JSON pJson,
    IDM_GROUP_ARRAY_DATA** ppGroupArray);

SSOERROR
IdmSchemaObjectMappingDataToJson(
    const IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMapping,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToSchemaObjectMappingData(
    PCSSO_JSON pJson,
    IDM_SCHEMA_OBJECT_MAPPING_DATA** ppSchemaObjectMapping);

SSOERROR
IdmSchemaObjectMappingMapDataToJson(
    const IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* pSchemaObjectMappingMap,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToSchemaObjectMappingMapData(
    PCSSO_JSON pJson,
    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA** ppSchemaObjectMappingMap);

SSOERROR
IdmIdentityProviderDataToJson(
    const IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToIdentityProviderData(
    PCSSO_JSON pJson,
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProvider);

SSOERROR
IdmIdentityProviderArrayDataToJson(
    const IDM_IDENTITY_PROVIDER_ARRAY_DATA* pIdentityProviderArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToIdentityProviderArrayData(
    PCSSO_JSON pJson,
    IDM_IDENTITY_PROVIDER_ARRAY_DATA** ppIdentityProviderArray);

SSOERROR
IdmOidcClientMetadataDataToJson(
    const IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToOidcClientMetadataData(
    PCSSO_JSON pJson,
    IDM_OIDC_CLIENT_METADATA_DATA** ppOidcClientMetadata);

SSOERROR
IdmOidcClientDataToJson(
    const IDM_OIDC_CLIENT_DATA* pOidcClient,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToOidcClientData(
    PCSSO_JSON pJson,
    IDM_OIDC_CLIENT_DATA** ppOidcClient);

SSOERROR
IdmOidcClientArrayDataToJson(
    const IDM_OIDC_CLIENT_ARRAY_DATA* pOidcClientArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToOidcClientArrayData(
    PCSSO_JSON pJson,
    IDM_OIDC_CLIENT_ARRAY_DATA** ppOidcClientArray);

SSOERROR
IdmPasswordResetRequestDataToJson(
    const IDM_PASSWORD_RESET_REQUEST_DATA* pPasswordResetRequest,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToPasswordResetRequestData(
    PCSSO_JSON pJson,
    IDM_PASSWORD_RESET_REQUEST_DATA** ppPasswordResetRequest);

SSOERROR
IdmSignatureAlgorithmDataToJson(
    const IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithm,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToSignatureAlgorithmData(
    PCSSO_JSON pJson,
    IDM_SIGNATURE_ALGORITHM_DATA** ppSignatureAlgorithm);

SSOERROR
IdmSignatureAlgorithmArrayDataToJson(
    const IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* pSignatureAlgorithmArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToSignatureAlgorithmArrayData(
    PCSSO_JSON pJson,
    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA** ppSignatureAlgorithmArray);

SSOERROR
IdmRelyingPartyDataToJson(
    const IDM_RELYING_PARTY_DATA* pRelyingParty,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToRelyingPartyData(
    PCSSO_JSON pJson,
    IDM_RELYING_PARTY_DATA** ppRelyingParty);

SSOERROR
IdmRelyingPartyArrayDataToJson(
    const IDM_RELYING_PARTY_ARRAY_DATA* pRelyingPartyArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToRelyingPartyArrayData(
    PCSSO_JSON pJson,
    IDM_RELYING_PARTY_ARRAY_DATA** ppRelyingPartyArray);

SSOERROR
IdmResourceServerDataToJson(
    const IDM_RESOURCE_SERVER_DATA* pResourceServer,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToResourceServerData(
    PCSSO_JSON pJson,
    IDM_RESOURCE_SERVER_DATA** ppResourceServer);

SSOERROR
IdmResourceServerArrayDataToJson(
    const IDM_RESOURCE_SERVER_ARRAY_DATA* pResourceServerArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToResourceServerArrayData(
    PCSSO_JSON pJson,
    IDM_RESOURCE_SERVER_ARRAY_DATA** ppResourceServerArray);

SSOERROR
IdmUserArrayDataToJson(
    const IDM_USER_ARRAY_DATA* pUserArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToUserArrayData(
    PCSSO_JSON pJson,
    IDM_USER_ARRAY_DATA** ppUserArray);

SSOERROR
IdmSolutionUserDataToJson(
    const IDM_SOLUTION_USER_DATA* pSolutionUser,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToSolutionUserData(
    PCSSO_JSON pJson,
    IDM_SOLUTION_USER_DATA** ppSolutionUser);

SSOERROR
IdmSolutionUserArrayDataToJson(
    const IDM_SOLUTION_USER_ARRAY_DATA* pSolutionUserArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToSolutionUserArrayData(
    PCSSO_JSON pJson,
    IDM_SOLUTION_USER_ARRAY_DATA** ppSolutionUserArray);

SSOERROR
IdmSearchResultDataToJson(
    const IDM_SEARCH_RESULT_DATA* pSearchResult,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToSearchResultData(
    PCSSO_JSON pJson,
    IDM_SEARCH_RESULT_DATA** ppSearchResult);

SSOERROR
IdmServerDetailsDataToJson(
    const IDM_SERVER_DETAILS_DATA* pServerDetails,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToServerDetailsData(
    PCSSO_JSON pJson,
    IDM_SERVER_DETAILS_DATA** ppServerDetails);

SSOERROR
IdmServerDetailsArrayDataToJson(
    const IDM_SERVER_DETAILS_ARRAY_DATA* pServerDetailsArray,
    PSSO_JSON pJson);

SSOERROR
IdmJsonToServerDetailsArrayData(
    PCSSO_JSON pJson,
    IDM_SERVER_DETAILS_ARRAY_DATA** ppServerDetailsArray);

#endif /* PROTOTYPES_H_ */
