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
    public class ServiceConfigManager: IServiceConfigManager
    {
        private Dictionary<string, string> _config;
        private string _product;
        public ServiceConfigManager(){
            _product = CommonConstants.GetConfigValue("product");
            Initialize();
        }
        public ServiceConfigManager(string product){
            _product = product.ToLower();
            Initialize();
        }
        public void Initialize()
        {
            if (_config == null)
            {
                _config = new Dictionary<string, string>();
                var filename = string.Format(@"Vmware.Tools.RestSsoAdminSnapIn.Service.configs.{0}.services.endpoint.xml", _product);
                Parse(filename);
            }
        }

        private void Parse(string filename)
        {
            var xml = XmlResourceHelper.GetResourceXml(filename);
            var xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(xml);
            if (xmlDoc.ChildNodes != null
                && xmlDoc.ChildNodes.Count > 1
                && xmlDoc.ChildNodes[1].ChildNodes != null)
            {
                foreach (XmlNode node in xmlDoc.ChildNodes[1].ChildNodes)
                {
                    var key = node.Attributes["name"].Value;
                    var value = node.InnerText;
                    value.Replace("&amp;", "&");
                    _config.Add(key, value);
                }
            }
        }
        public string GetUsersEndPoint()  {  return _config["UsersEndPoint"].ToString(); }
        public string GetUserEndPoint()  {  return _config["UserEndPoint"].ToString(); }
        public string GetUserGroupsEndPoint() {   return _config["UserGroupsEndPoint"].ToString(); }
        public string GetLoginTokenFragments()  {  return _config["LoginTokenFragments"].ToString(); }
        public string GetLoginEndPoint()  {  return _config["LoginEndPoint"].ToString(); }
        public string GetServerEndPoint()  {  return _config["ServerEndPoint"].ToString(); }
        public string GetLoginArguments()  {  return _config["LoginArguments"].ToString(); }
        public string GetGssTicketLoginArguments()  {  return _config["GssTicketLoginArguments"].ToString(); }
        public string GetJwtTokenBySolutionUserArguments()  {  return _config["JwtTokenBySolutionUserArguments"].ToString(); }
        public string GetRefreshTokenEndPoint()  {  return _config["RefreshTokenEndPoint"].ToString(); }
        public string GetRefreshTokenArguments()  {  return _config["RefreshTokenArguments"].ToString(); }
        public string GetUserPasswordEndPoint()  {  return _config["UserPasswordEndPoint"].ToString(); }
        public string GetUserEndPostPoint()  {  return _config["GetUserEndPostPoint"].ToString(); }
        public string GetUserGroupsPostEndPoint()  {  return _config["GetUserGroupsPostEndPoint"].ToString(); }
        public string GetUsersString()  {  return _config["UsersString"].ToString(); }
        public string GetUserPasswordString()  {  return _config["UserPasswordString"].ToString(); }
        public string GetSolutionUsersEndPoint()  {  return _config["SolutionUsersEndPoint"].ToString(); }
        public string GetSolutionUserEndPoint()  {  return _config["SolutionUserEndPoint"].ToString(); }
        public string GetSolutionUserPostEndPoint()  {  return _config["GetSolutionUserPostEndPoint"].ToString(); }
        public string GetSolutionUserGroupsPostEndPoint()  {  return _config["GetSolutionUserGroupsPostEndPoint"].ToString(); }
        public string GetGroupsEndPoint()  {  return _config["GroupsEndPoint"].ToString(); }
        public string GetGroupEndPoint()  {  return _config["GroupEndPoint"].ToString(); }
        public string GetParentsOfGroupEndPoint()  {  return _config["ParentsOfGroupEndPoint"].ToString(); }
        public string GetMembersOfGroupEndPoint()  {  return _config["MembersOfGroupEndPoint"].ToString(); }
        public string GetAllMembersOfGroupEndPoint()  {  return _config["AllMembersOfGroupEndPoint"].ToString(); }
        public string GetGroupPostEndPoint()  {  return _config["GetGroupPostEndPoint"].ToString(); }
        public string GetParentsOfGroupPostEndPoint()  {  return _config["GetParentsOfGroupPostEndPoint"].ToString(); }
        public string GetMembersOfGroupPostEndPoint()  {  return _config["GetMembersOfGroupPostEndPoint"].ToString(); }
        public string GetAllMembersOfGroupPostEndPoint()  {  return _config["GetAllMembersOfGroupPostEndPoint"].ToString(); }
        public string GetTenantEndPoint()  {  return _config["TenantEndPoint"].ToString(); }
        public string GetTenantsEndPoint()  {  return _config["TenantsEndPoint"].ToString(); }
        public string GetTenantConfigEndPoint()  {  return _config["TenantConfigEndPoint"].ToString(); }
        public string GetTenantSearchEndPoint()  {  return _config["TenantSearchEndPoint"].ToString(); }
        public string GetTenantString()  {  return _config["TenantString"].ToString(); }
        public string GetSamlLegacyEndPoint()  {  return _config["SamlLegacyEndPoint"].ToString(); }
        public string GetTenantPostEndPoint()  {  return _config["GetTenantPostEndPoint"].ToString(); }
        public string GetTenantConfigPostEndPoint()  {  return _config["GetTenantConfigPostEndPoint"].ToString(); }
        public string GetTenantSearchPostEndPoint()  {  return _config["GetTenantSearchPostEndPoint"].ToString(); }
        public string GetExternalIdentityProvidersEndPoint()  {  return _config["ExternalIdentityProvidersEndPoint"].ToString(); }
        public string GetExternalIdentityProviderEndPoint()  {  return _config["ExternalIdentityProviderEndPoint"].ToString(); }
        public string GetExternalIdentityProvidersPostEndPoint()  {  return _config["GetExternalIdentityProvidersPostEndPoint"].ToString(); }
        public string GetExternalIdentityProviderPostEndPoint()  {  return _config["GetExternalIdentityProviderPostEndPoint"].ToString(); }
        public string GetRelyingPartysEndPoint()  {  return _config["RelyingPartysEndPoint"].ToString(); }
        public string GetRelyingPartyEndPoint()  {  return _config["RelyingPartyEndPoint"].ToString(); }
        public string GetRelyingPartysPostEndPoint()  {  return _config["GetRelyingPartysPostEndPoint"].ToString(); }
        public string GetRelyingPartyPostEndPoint()  {  return _config["GetRelyingPartyPostEndPoint"].ToString(); }
        public string GetIdentityProvidersEndPoint()  {  return _config["IdentityProvidersEndPoint"].ToString(); }
        public string GetIdentityProviderEndPoint()  {  return _config["IdentityProviderEndPoint"].ToString(); }
        public string GetIdentityProvidersPostEndPoint()  {  return _config["GetIdentityProvidersPostEndPoint"].ToString(); }
        public string GetIdentityProviderPostEndPoint()  {  return _config["GetIdentityProviderPostEndPoint"].ToString(); }
        public string GetProvidersString()  {  return _config["ProvidersString"].ToString(); }
        public string GetCertificatesEndPoint()  {  return _config["CertificatesEndPoint"].ToString(); }
        public string GetCertificatePrivateKeyEndPoint()  {  return _config["CertificatePrivateKeyEndPoint"].ToString(); }
        public string GetCertificatesPostEndPoint()  {  return _config["GetCertificatesPostEndPoint"].ToString(); }
        public string GetCertificatePrivateKeyPostEndPoint()  {  return _config["GetCertificatePrivateKeyPostEndPoint"].ToString(); }
        public string GetOidcClientsEndPoint()  {  return _config["OidcClientsEndPoint"].ToString(); }
        public string GetOidcClientEndPoint()  {  return _config["OidcClientEndPoint"].ToString(); }
        public string GetOidcClientsPostEndPoint()  {  return _config["OidcClientsPostEndPoint"].ToString(); }
        public string GetOidcClientPostEndPoint()  {  return _config["OidcClientPostEndPoint"].ToString(); }
        public string GetServerComputersPostEndPoint()  {  return _config["GetServerComputersPostEndPoint"].ToString(); }
        public string GetServerStatusPostEndPoint()  {  return _config["GetServerStatusPostEndPoint"].ToString(); }
        public string GetAdfEndPoint()  {  return _config["AdfEndPoint"].ToString(); }
        public string GetPostAdfEndPoint()  {  return _config["PostAdfEndPoint"].ToString(); }
        public string GetEventLogPostEndPoint()  {  return _config["GetEventLogPostEndPoint"].ToString(); }
        public string GetEventLogEndPoint()  {  return _config["EventLogEndPoint"].ToString(); }
        public string GetStartEventLogPostEndPoint()  {  return _config["StartEventLogPostEndPoint"].ToString(); }
        public string GetStopEventLogPostEndPoint()  {  return _config["StopEventLogPostEndPoint"].ToString(); }
        public string GetStatusEventLogPostEndPoint()  {  return _config["StatusEventLogPostEndPoint"].ToString(); }
        public string GetStatusEventLogEndPoint()  {  return _config["StatusEventLogEndPoint"].ToString(); }
        public string GetValidationUri()  {  return _config["ValidationUri"].ToString(); }
        public string GetServerAboutInfoEndPoint()  {  return _config["ServerAboutInfoEndPoint"].ToString(); }
        public string GetPasswordAndLockoutPolicyEndPoint() { return _config["PasswordAndLockoutPolicyEndPoint"].ToString(); }
        public string GetPasswordAndLockoutPolicyPostEndPoint() { return _config["GetPasswordAndLockoutPolicyPostEndPoint"].ToString(); }
        public string FormatLoginArgs(LoginDto loginDto)
        {
            return string.Format(GetLoginArguments(), loginDto.User, loginDto.Pass, loginDto.DomainName);
        }

        public string GetLoginUrl(ServerDto serverDto, string tenant)
        {
            return string.Format(GetLoginEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
        }

        public string GetRefreshUrl(ServerDto serverDto, string tenant)
        {
            return string.Format(GetRefreshTokenEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
        }
        public string FormatRefreshTokenArgs(string refreshToken)
        {
            return string.Format(GetRefreshTokenArguments(), refreshToken);
        }

        public string GetCertificatesUrl(ServerDto serverDto, string tenant)
        {
            return string.Format(GetCertificatesEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenant);
        }

        public string GetTokenFromCertificateUrl(ServerDto serverDto)
        {
            return string.Format(GetLoginEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, serverDto.Tenant);
        }

        public string GetAudience(ServerDto serverDto)
        {
            return string.Format(GetServerEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Tenant);
        }

        public string GetValidIssuer(ServerDto serverDto, string hostName, string tenantName)
        {
            return string.Format(GetValidationUri(), serverDto.Protocol, hostName, tenantName);
        }

        public string GetJwtTokenBySolutionUserArgs(string signedToken)
        {
            return string.Format(GetJwtTokenBySolutionUserArguments(), signedToken);
        }

        public string GetTokenFromGssTicketUrl(ServerDto serverDto)
        {
            return string.Format(GetLoginEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, serverDto.Tenant);
        }

        public string GetTokenFromGssTicketArgs(string base64EncodedBytes, string clientId)
        {
            return string.Format(GetGssTicketLoginArguments(), base64EncodedBytes, clientId);
        }

        public string UpdatePasswordUrl(ServerDto serverDto, string tenantName, string upn)
        {
            return string.Format(GetUserPasswordEndPoint(), serverDto.Protocol, serverDto.ServerName, serverDto.Port, tenantName, upn);
        }
    }
}