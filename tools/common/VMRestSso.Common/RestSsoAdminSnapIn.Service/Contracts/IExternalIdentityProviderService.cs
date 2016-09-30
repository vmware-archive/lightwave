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

using System.Net;
using System.Collections.Generic;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.Contracts
{
    public interface IExternalIdentityProviderService
    {
        List<ExternalIdentityProviderDto> GetAll(ServerDto serverDto, string tenantName, Token token);
        ExternalIdentityProviderDto Get(ServerDto serverDto, string tenantName, ExternalIdentityProviderDto externalIdentityProvider, Token token);
        ExternalIdentityProviderDto Create(ServerDto server, string tenantName, ExternalIdentityProviderDto externalIdentityProvider, Token token);
        ExternalIdentityProviderDto Create(ServerDto server, string tenantName, string xmlMetadata, Token token);
        bool Delete(ServerDto serverDto, string tenant, ExternalIdentityProviderDto externalIdentityProvider, Token token);
        ExternalIdentityProviderDto Update(ServerDto serverDto, string tenant, ExternalIdentityProviderDto externalIdentityProvider, Token token);
    }
}
