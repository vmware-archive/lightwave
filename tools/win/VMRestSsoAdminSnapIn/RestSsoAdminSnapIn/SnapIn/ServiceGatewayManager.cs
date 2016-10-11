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

using System.Collections.Generic;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Cache;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn
{
    public class ServiceGatewayManager
    {
        private Dictionary<string, ServiceGateway> _serviceGatewayCache;
        private readonly ServiceGateway _serviceGateway;
        public ServiceGatewayManager()
        {
            _serviceGatewayCache = new Dictionary<string, ServiceGateway>();
            var serviceConfigManager = new ServiceConfigManager();
            _serviceGateway = new ServiceGateway(serviceConfigManager);
        }

        public ServiceGateway Add(string server, ServiceGateway instance)
        {
            ServiceGateway value;
            if (_serviceGatewayCache.ContainsKey(server))
                _serviceGatewayCache.Remove(server);
            _serviceGatewayCache.Add(server, instance);
            return instance;
        }

        public ServiceGateway Get(string server)
        {
            ServiceGateway value;
            _serviceGatewayCache.TryGetValue(server, out value);
            return value;
        }

        public void Remove(string server)
        {
            if (_serviceGatewayCache.ContainsKey(server))
                _serviceGatewayCache.Remove(server);
        }

        public ServiceGateway GetAny()
        {
            return _serviceGateway;
        }
    }
}
