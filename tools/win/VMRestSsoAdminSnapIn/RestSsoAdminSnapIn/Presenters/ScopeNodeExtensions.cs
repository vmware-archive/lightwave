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
 
using Microsoft.ManagementConsole;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters
{
    public static class ScopeNodeExtensions
    {
        //public static ServiceGateway GetServiceGateway(this ScopeNode node)
        //{
        //    return SnapInContext.Instance.ServiceGatewayManager.GetAny();
        //}
        public static ServiceGateway GetServiceGateway(string server)
        {
            return SnapInContext.Instance.ServiceGatewayManager.Get(server);
        }

        public static SnapInContext GetApplicationContext(this ScopeNode node)
        {
            return SnapInContext.Instance;
        }
        public static ServiceGateway GetServiceGateway()
        {
            return SnapInContext.Instance.ServiceGatewayManager.GetAny();
        }
    }
}
