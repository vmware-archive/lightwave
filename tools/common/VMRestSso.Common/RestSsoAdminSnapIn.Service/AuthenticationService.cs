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

using System;
using System.Collections.Generic;
using System.IdentityModel.Tokens;
using System.Net;
using System.Security.Claims;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.Authentication
{
    public class AuthenticationService
    {
        private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public AuthenticationService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
        {
            _webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
        }

        public AuthTokenDto Login(ServerDto serverDto, LoginDto loginDto, string clientId)
        {
            IAuthenticationService tokenService = null;
            if(serverDto.TokenType == TokenType.SAML)
                tokenService = new SamlTokenService(_webRequestManager, _serviceConfigManager);
            else
                tokenService = new JwtTokenService(_webRequestManager, _serviceConfigManager);
            return tokenService.Authenticate(serverDto, loginDto, clientId);
        }

        public Token Refresh(ServerDto serverDto, LoginDto loginDto, Token tokenToRefresh)
        {
            IAuthenticationService tokenService = null;
            if (serverDto.TokenType == TokenType.SAML)
                tokenService = new SamlTokenService(_webRequestManager, _serviceConfigManager);
            else
                tokenService = new JwtTokenService(_webRequestManager, _serviceConfigManager);
            return tokenService.Refresh(serverDto, loginDto, tokenToRefresh);
        }        
    }
}