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

using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Xml;
using VMIdentity.CommonUtils;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service
{
    public interface IServiceConfigManager
    {
        void Initialize();
        string GetUsersEndPoint();
        string GetUserEndPoint();
        string GetUserGroupsEndPoint();
        string GetLoginTokenFragments();
        string GetLoginEndPoint();
        string GetServerEndPoint();
        string GetLoginArguments();
        string GetGssTicketLoginArguments();
        string GetJwtTokenBySolutionUserArguments();
        string GetRefreshTokenEndPoint();
        string GetRefreshTokenArguments();
        string GetUserPasswordEndPoint();
        string GetUserEndPostPoint();
        string GetUserGroupsPostEndPoint();
        string GetUsersString();
        string GetUserPasswordString();
        string GetSolutionUsersEndPoint();
        string GetSolutionUserEndPoint();
        string GetSolutionUserPostEndPoint();
        string GetSolutionUserGroupsPostEndPoint();
        string GetGroupsEndPoint();
        string GetGroupEndPoint();
        string GetParentsOfGroupEndPoint();
        string GetMembersOfGroupEndPoint();
        string GetAllMembersOfGroupEndPoint();
        string GetGroupPostEndPoint();
        string GetParentsOfGroupPostEndPoint();
        string GetMembersOfGroupPostEndPoint();
        string GetAllMembersOfGroupPostEndPoint();
        string GetTenantEndPoint();
        string GetTenantsEndPoint();
        string GetTenantConfigEndPoint();
        string GetTenantSearchEndPoint();
        string GetTenantString();
        string GetSamlLegacyEndPoint();
        string GetTenantPostEndPoint();
        string GetTenantConfigPostEndPoint();
        string GetTenantSearchPostEndPoint();
        string GetExternalIdentityProvidersEndPoint();
        string GetExternalIdentityProviderEndPoint();
        string GetExternalIdentityProvidersPostEndPoint();
        string GetExternalIdentityProviderPostEndPoint();
        string GetRelyingPartysEndPoint();
        string GetRelyingPartyEndPoint();
        string GetRelyingPartysPostEndPoint();
        string GetRelyingPartyPostEndPoint();
        string GetIdentityProvidersEndPoint();
        string GetIdentityProviderEndPoint();
        string GetIdentityProvidersPostEndPoint();
        string GetIdentityProviderPostEndPoint();
        string GetProvidersString();
        string GetCertificatesEndPoint();
        string GetCertificatePrivateKeyEndPoint();
        string GetCertificatesPostEndPoint();
        string GetCertificatePrivateKeyPostEndPoint();
        string GetOidcClientsEndPoint();
        string GetOidcClientEndPoint();
        string GetOidcClientsPostEndPoint();
        string GetOidcClientPostEndPoint();
        string GetServerComputersPostEndPoint();
        string GetServerStatusPostEndPoint();
        string GetAdfEndPoint();
        string GetPostAdfEndPoint();
        string GetEventLogPostEndPoint();
        string GetEventLogEndPoint();
        string GetStartEventLogPostEndPoint();
        string GetStopEventLogPostEndPoint();
        string GetStatusEventLogPostEndPoint();
        string GetStatusEventLogEndPoint();
        string GetValidationUri();
        string GetServerAboutInfoEndPoint();
        string GetPasswordAndLockoutPolicyEndPoint();
        string GetPasswordAndLockoutPolicyPostEndPoint();
        string FormatLoginArgs(LoginDto loginDto);
        string GetLoginUrl(ServerDto serverDto, string tenant);

        string GetRefreshUrl(ServerDto serverDto, string tenant);
        string FormatRefreshTokenArgs(string refreshToken);

        string GetCertificatesUrl(ServerDto serverDto, string tenant);

        string GetTokenFromCertificateUrl(ServerDto serverDto);

        string GetAudience(ServerDto serverDto);

        string GetValidIssuer(ServerDto serverDto, string hostName, string tenantName);

        string GetJwtTokenBySolutionUserArgs(string signedToken);

        string GetTokenFromGssTicketUrl(ServerDto serverDto);

        string GetTokenFromGssTicketArgs(string base64EncodedBytes, string clientId);

        string UpdatePasswordUrl(ServerDto serverDto, string tenantName, string upn);
    }
}
