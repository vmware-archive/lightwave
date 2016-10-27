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

using System.IO;
using System.Net;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Cache;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Persistence;
using Vmware.Tools.RestSsoAdminSnapIn.Data.Storage;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;

namespace Vmware.Tools.RestSsoAdminSnapIn
{
    public class Bootstrapper
    {
        private IPersistentStorage<SimpleCache<AuthTokenDto>> _store;
        private IPersistentStorage<HttpTransportCollection> _httpTransportStore;

        public void Startup()
        {
            SnapInContext.Instance.AuthTokenManager = new AuthTokenManager();
            var filePath = Path.Combine(SnapInContext.Instance.ApplicationPath, Constants.LocalStorageFileName);
            _store = new LocalFileStorage(filePath);
            SnapInContext.Instance.AuthTokenManager.CacheAuthTokens(_store.Load());

            var httpTransportFilePath = Path.Combine(SnapInContext.Instance.ApplicationPath, Constants.HttpTransportLocalStorageFileName);
            _httpTransportStore = new HttpTransportLocalFileStorage(httpTransportFilePath);

            var httpData = _httpTransportStore.Load();
            var service = ScopeNodeExtensions.GetServiceGateway();
            service.HttpTransport.SetAll(httpData);
            ServicePointManager.SecurityProtocol = SecurityProtocolType.Ssl3 | SecurityProtocolType.Tls12;
        }

        public void Shutdown()
        {
            _store.Save(SnapInContext.Instance.AuthTokenManager.GetAuthTokenCache());
            var service = ScopeNodeExtensions.GetServiceGateway();
            _httpTransportStore.Save(service.HttpTransport.GetAll());
        }
    }
}
