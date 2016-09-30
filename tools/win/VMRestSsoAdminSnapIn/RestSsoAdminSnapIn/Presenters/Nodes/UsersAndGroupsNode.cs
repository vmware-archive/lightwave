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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes
{
    public class UsersAndGroupsNode : ScopeNode
    {
        private readonly string _tenantName;
        private readonly string _domainName;
        private readonly bool _systemDomain;
        private readonly ServerDto _serverDto;
        public UsersAndGroupsNode(ServerDto dto, string tenantName, IdentityProviderDto provider, bool systemDomain)
            : base(false)
        {
            ImageIndex = SelectedImageIndex = (int)TreeImageIndex.Group;
            _tenantName = tenantName;
            _domainName = provider.Name;
            _systemDomain = systemDomain;
            _serverDto = dto;
            DisplayName = "Users and Groups";
        }
        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);
            var node = new UsersNode(_serverDto, _tenantName, _domainName, _systemDomain);
            Children.Add(node);
            if (_systemDomain)
            {
                var solnNode = new SolutionUsersNode(_serverDto, _tenantName, _domainName, _systemDomain);
                Children.Add(solnNode);
            }
            var groupNode = new GroupsNode(_serverDto, _tenantName, _domainName, _systemDomain);
            Children.Add(groupNode);
        }
    }
}
