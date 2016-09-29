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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.ExternalDomain
{
    public class ExternalDomainPropertyManager : IPropertyDataManager
    {
        private readonly ServiceGateway _service;
        private readonly ServerDto _server;        
        private IdentityProviderDto _provider;
        private readonly string _tenantName;        
        public ExternalDomainPropertyManager(string tenantName, ServiceGateway service, ServerDto server, IdentityProviderDto provider)
        {
            _tenantName = tenantName;
            _service = service;
            _server = server;
            _provider = provider;            
        }
        public object GetData()
        {            
            return _provider.DeepCopy();
        }
        public bool Apply(object obj)
        {            
            var dto = obj as IdentityProviderDto;
            var result = false;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_server, _tenantName);
            ActionHelper.Execute(delegate()
            {
                if (dto.IsSameAs(_provider))
                    _service.IdentityProvider.Update(_server, _tenantName, dto, auth.Token);
                result = true;
            }, auth);
            return result;
        }
    }
}
