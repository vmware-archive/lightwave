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

using Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Adf;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Authentication;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Certificate;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Contracts;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Group;
using Vmware.Tools.RestSsoAdminSnapIn.Service.HttpTransport;
using Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider;
using Vmware.Tools.RestSsoAdminSnapIn.Service.OidcClient;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Server;
using Vmware.Tools.RestSsoAdminSnapIn.Service.SolutionUser;
using Vmware.Tools.RestSsoAdminSnapIn.Service.Tenant;
using Vmware.Tools.RestSsoAdminSnapIn.Service.User;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service
{
    public class ServiceGateway
    {
        private readonly IWebRequestManager _webRequestManager;
        private SamlTokenService _samlTokenService;
        private JwtTokenService _jwtTokenService;
        private AuthenticationService _authenticationService;
        private UserService _userService;
        private SolutionUserService _solutionUserService;
        private TenantService _tenant;
        private ServerService _server;
        private GroupService _group;
        private CertificateService _certificate;
        private IdentityProviderService _identityProvider;
        private IExternalIdentityProviderService _externalIdentityProvider;
        private OidcClientService _oidcClient;
        private RelyingPartyService _relyingParty;
        private HttpTransportService _httpTransport;
        private AdfService _adfService;
        private SuperLoggingService _superLoggingService;
        private IServiceConfigManager _serviceConfigManager;
        private Mac.ExternalIdentityProviderService _macExternalIdentityProviderService;

        public ServiceGateway(IServiceConfigManager serviceConfigManager)
        {
            _serviceConfigManager = serviceConfigManager;
            _webRequestManager = new HttpWebRequestManager();
            _webRequestManager.Subscribe(HttpTransport.HandleMessage);
        }

        public SamlTokenService SamlTokenService
        {
            get
            {
                return _samlTokenService ?? (_samlTokenService = new SamlTokenService(_webRequestManager, _serviceConfigManager));
            }
        }

        public JwtTokenService JwtTokenService
        {
            get
            {
                return _jwtTokenService ?? (_jwtTokenService = new JwtTokenService(_webRequestManager, _serviceConfigManager));
            }
        }

        public AuthenticationService Authentication
        {
            get
            {
                return _authenticationService ?? (_authenticationService = new AuthenticationService(_webRequestManager, _serviceConfigManager));
            }
        }

        public UserService User
        {
            get
            {
                return _userService ?? (_userService = new UserService(_webRequestManager, _serviceConfigManager));
            }
        }

        public SolutionUserService SolutionUser
        {
            get
            {
                return _solutionUserService ?? (_solutionUserService = new SolutionUserService(_webRequestManager, _serviceConfigManager));
            }
        }

        public TenantService Tenant
        {
            get
            {
                return _tenant ?? (_tenant = new TenantService(_webRequestManager, _serviceConfigManager));
            }
        }

        public GroupService Group
        {
            get
            {
                return _group ?? (_group = new GroupService(_webRequestManager, _serviceConfigManager));
            }
        }

        public CertificateService Certificate
        {
            get
            {
                return _certificate ?? (_certificate = new CertificateService(_webRequestManager, _serviceConfigManager));
            }
        }

        public IdentityProviderService IdentityProvider
        {
            get
            {
                return _identityProvider ?? (_identityProvider = new IdentityProviderService(_webRequestManager, _serviceConfigManager));
            }
        }
        public IExternalIdentityProviderService ExternalIdentityProvider
        {
            get
            {
                if (_externalIdentityProvider == null)
                {
                    if (SystemHelper.IsMacOSX)
                        _externalIdentityProvider = new Mac.ExternalIdentityProviderService(_webRequestManager, _serviceConfigManager);
                    else
                        _externalIdentityProvider = new Win.ExternalIdentityProviderService(_webRequestManager, _serviceConfigManager);
                }
                return _externalIdentityProvider;
            }
        }
        public OidcClientService OidcClient
        {
            get
            {
                return _oidcClient ?? (_oidcClient = new OidcClientService(_webRequestManager, _serviceConfigManager));
            }
        }
        public RelyingPartyService RelyingParty
        {
            get
            {
                return _relyingParty ?? (_relyingParty = new RelyingPartyService(_webRequestManager, _serviceConfigManager));
            }
        }

        public HttpTransportService HttpTransport
        {
            get
            {
                return _httpTransport ?? (_httpTransport = new HttpTransportService(_serviceConfigManager));
            }
        }

        public ServerService Server
        {
            get
            {
                return _server ?? (_server = new ServerService(_webRequestManager, _serviceConfigManager));
            }
        }
        public SuperLoggingService SuperLogging
        {
            get
            {
                return _superLoggingService ?? (_superLoggingService = new SuperLoggingService(_webRequestManager, _serviceConfigManager));
            }
        }

        public AdfService Adf
        {
            get
            {
                return _adfService ?? (_adfService = new AdfService(_webRequestManager, _serviceConfigManager));
            }
        }

        public Mac.ExternalIdentityProviderService MacExternalIdentityProviderService
        {
            get
            {
                return _macExternalIdentityProviderService ?? (_macExternalIdentityProviderService = new Mac.ExternalIdentityProviderService(_webRequestManager, _serviceConfigManager));
            }
        }

        public string GetTenantEndpoint(bool legacy, string protocol, string server, string port, string tenant)
        {
            return legacy
                ? string.Format(_serviceConfigManager.GetSamlLegacyEndPoint(), protocol, server, port, tenant)
                    : string.Format(_serviceConfigManager.GetLoginEndPoint(), protocol, server, port, tenant);
        }
    }
}
