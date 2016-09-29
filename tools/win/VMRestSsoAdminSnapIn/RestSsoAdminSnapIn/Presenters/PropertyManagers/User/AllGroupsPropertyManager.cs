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
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.User
{
    public class AllGroupsPropertyManager
    {
        private readonly ServiceGateway _service;
        private readonly ServerDto _serverDto;
        private string _domainName;
        private string _tenantName;
        List<GroupDto> _selectedGroups;
        public AllGroupsPropertyManager(ServiceGateway service, ServerDto serverDto, string tenantName, string domainName)
        {
            _service = service;
            _serverDto = serverDto;
            _domainName = domainName;
            _tenantName = tenantName;
        }

        public List<GroupDto> Search(string name)
        {
            var groupsDto = new List<GroupDto>();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
            ActionHelper.Execute(delegate()
            {
                var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                var membershipDto = service.Tenant.Search(_serverDto, _tenantName, _domainName, MemberType.GROUP, SearchType.NAME, auth.Token, name);
                groupsDto = membershipDto.Groups;// _service.Group.GetAll(_serverDto, _tenantName, auth.Token, _domainName);
            }, auth);
            return groupsDto;
        }

        public bool Apply(object obj)
        {
            _selectedGroups = obj as List<GroupDto>;
            return true;
        }

        public List<GroupDto> GetSelectedGroups()
        {
            return _selectedGroups;
        }
    }
}