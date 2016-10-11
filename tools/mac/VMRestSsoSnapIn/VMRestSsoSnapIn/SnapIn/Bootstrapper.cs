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


using System.IO;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Cache;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Persistence;
using Vmware.Tools.RestSsoAdminSnapIn.Data.Storage;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn
{
    public static class Bootstrapper
    {
        private static IPersistentStorage<SimpleCache<AuthTokenDto>> _store;
        private static IPersistentStorage<HttpTransportCollection> _httpTransportStore;

        public static void Startup()
        {
            SnapInContext.Instance.AuthTokenManager = new AuthTokenManager();
            var filePath = Path.Combine(SnapInContext.Instance.ApplicationPath, Constants.LocalStorageFileName);
            _store = new LocalFileStorage(filePath);
            SnapInContext.Instance.AuthTokenManager.CacheAuthTokens(_store.Load());

            var httpTransportFilePath = Path.Combine(SnapInContext.Instance.ApplicationPath, Constants.HttpTransportLocalStorageFileName);
            _httpTransportStore = new HttpTransportLocalFileStorage(httpTransportFilePath);

            var httpData = _httpTransportStore.Load();
            var service = SnapInContext.Instance.ServiceGateway;
            service.HttpTransport.SetAll(httpData);
        }

        public static void Shutdown()
        {
            _store.Save(SnapInContext.Instance.AuthTokenManager.GetAuthTokenCache());
            var service = SnapInContext.Instance.ServiceGateway;
            _httpTransportStore.Save(service.HttpTransport.GetAll());
        }
    }
}
