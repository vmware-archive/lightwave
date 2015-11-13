/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

namespace Vmware.Tools.RestSsoAdminSnapIn.Service
{
    public class ServiceConfigManager
    {
        // Authentication Endpoints
        public const string LoginTokenFragments = "/openidconnect/token/";
        public const string LoginEndPoint = "{0}://{1}:{2}/openidconnect/token/{3}";
        public const string LoginArguments = "grant_type=password&username={0}%40{2}&password={1}&scope=openid+offline_access+id_groups+at_groups+rs_admin_server";
        public const string GssTicketLoginArguments = "grant_type=urn:vmware:grant_type:gss_ticket&gss_ticket={0}&context_id=_context_id_{1}&scope=openid+offline_access";
        public const string JwtTokenBySolutionUserArguments = "grant_type=urn:vmware:grant_type:solution_user_credentials&solution_assertion={0}&scope=openid";
        public const string RefreshTokenEndPoint = "{0}://{1}:{2}/openidconnect/token/{3}";
        public const string RefreshTokenArguments = "grant_type=refresh_token&refresh_token={0}";

        // User Resource Endpoints
        public const string UsersEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/users";
        public const string UserEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/users/{4}";
        public const string UserGroupsEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/users/{4}/groups";
        public const string UserPasswordEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/users/{4}/password?currentpassword={5}&newpassword={6}";
        public const string GetUserEndPostPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/users/{4}";
        public const string GetUserGroupsPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/users/{4}/groups";
        public const string UsersString = "/users";
        public const string UserPasswordString = "/password";

        // SolutionUser Resource Endpoints
        public const string SolutionUsersEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/solutionusers";
        public const string SolutionUserEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/solutionusers/{4}";
        public const string GetSolutionUserPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/solutionusers/{4}";
        public const string GetSolutionUserGroupsPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/solutionusers/{4}/groups";
        
        // Group Resource Endpoints
        public const string GroupsEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/groups";
        public const string GroupEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/groups/{4}";
        public const string ParentsOfGroupEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/groups/{4}/parents";
        public const string MembersOfGroupEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/groups/{4}/members?{5}&type={6}";
        public const string AllMembersOfGroupEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/groups/{4}/members?type={5}";
        public const string GetGroupPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/groups/{4}";
        public const string GetParentsOfGroupPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/groups/{4}/parents";
        public const string GetMembersOfGroupPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/groups/{4}/members?{5}&type={6}";
        public const string GetAllMembersOfGroupPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/groups/{4}/members?type={5}";


        // Tenant Resource Endpoints
        public const string TenantEndPoint = "{0}://{1}:{2}/idm/tenant/{3}";
        public const string TenantsEndPoint = "{0}://{1}:{2}/idm/tenant";        
        public const string TenantConfigEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/config";
        public const string TenantSearchEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/search";
        public const string TenantString = "/idm/tenant";
        public const string SamlLegacyEndPoint = "{0}://{1}:{2}/{3}";
        public const string GetTenantPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}";
        public const string GetTenantConfigPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/config";
        public const string GetTenantSearchPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/search";

        // External IdentityProvider Resource Endpoints
        public const string ExternalIdentityProvidersEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/externalidp";
        public const string ExternalIdentityProviderEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/externalidp/{4}";
        public const string GetExternalIdentityProvidersPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/externalidp";
        public const string GetExternalIdentityProviderPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/externalidp/{4}";

        // Relying Party Resource Endpoints
        public const string RelyingPartysEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/relyingparty";
        public const string RelyingPartyEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/relyingparty/{4}";
        public const string GetRelyingPartysPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/relyingparty";
        public const string GetRelyingPartyPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/relyingparty/{4}";

        // IdentityProvider Resource Endpoints
        public const string IdentityProvidersEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/providers";
        public const string IdentityProviderEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/providers/{4}";
        public const string GetIdentityProvidersPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/providers";
        public const string GetIdentityProviderPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/providers/{4}";
        public const string ProvidersString = "/providers";

        // Certificate Endpoints
        public const string CertificatesEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/certificates";
        public const string CertificatePrivateKeyEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/certificates/privatekey";
        public const string GetCertificatesPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/certificates";
        public const string GetCertificatePrivateKeyPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/certificates/privatekey";

        // Oidc client Endpoints
        public const string OidcClientsEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/oidcclient";
        public const string OidcClientEndPoint = "{0}://{1}:{2}/idm/tenant/{3}/oidcclient/{4}";
        public const string OidcClientsPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/oidcclient";
        public const string OidcClientPostEndPoint = "{0}://{1}:{2}/idm/post/tenant/{3}/oidcclient/{4}";

        // Http headers
        public const string ContentType = "application/x-www-form-urlencoded; charset=utf-8";
        public const string JsonContentType = "application/json";
        public const string PlainTextContentType = "text/plain";
        public const string UserAgent = "VMware-client/5.1.0";

        // Validation
        public const string ValidationUri = "{0}://{1}/{2}";

        // Server Endpoints
        //public const string GetServerComputersEndPoint = "{0}://{1}:{2}/idm/server/computers?type=all";
        //public const string GetServerStatusEndPoint = "{0}://{1}:{2}/idm/server/status";
        public const string GetServerComputersPostEndPoint = "{0}://{1}:{2}/idm/post/server/computers";
        public const string GetServerStatusPostEndPoint = "{0}://{1}:{2}/idm/post/server/status";

        // Adf Endpoints
        public const string AdfEndPoint = "{0}://{1}:{2}/afd/provider/ad";
        public const string PostAdfEndPoint = "{0}://{1}:{2}/afd/post/provider/ad";

        // Certificate Types
        public const string LdapTrustedCertificate = "LDAP_TRUSTED_CERT";
        public const string StsTrustedCertificate = "STS_TRUST_CERT";
    }
}
