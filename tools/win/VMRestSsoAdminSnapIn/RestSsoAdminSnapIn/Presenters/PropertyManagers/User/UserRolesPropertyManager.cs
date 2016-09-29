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
using System.Linq;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.User
{
    class UserRolesPropertyManager : IPropertyDataManager
    {
        private readonly ServiceGateway _service;
        private UserDto _user;
        private readonly ServerDto _serverDto;
        private readonly string _tenantName;
        private readonly string _domainName;
        public UserRolesPropertyManager(ServiceGateway service, UserDto user, ServerDto serverDto, string tenantName, string domainName)
        {
            _service = service;
            _user = user;
            _serverDto = serverDto;
            _tenantName = tenantName;
            _domainName = domainName;
        }

        public object GetData()
        {
            var originalUser = _user;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
            ActionHelper.Execute(delegate()
            {                
                var _orignialUserMembership = _service.User.GetUserGroups(_serverDto, _tenantName, _user, auth.Token);
                originalUser.Role = _orignialUserMembership.Exists(x => x.GroupName == "Administrators") ? UserRole.Administrator
                    : _orignialUserMembership.Exists(x => x.GroupName == "Users") ? UserRole.RegularUser
                    : UserRole.GuestUser;
                originalUser.IsIdpAdmin = _orignialUserMembership.Exists(x => x.GroupName == "IdpProvisioningAdmin");
                originalUser.ActAsUsers = _orignialUserMembership.Exists(x => x.GroupName == "ActAsUsers");
            }, auth);
            _user = originalUser.DeepCopy();
            return originalUser;
        }

        public bool Apply(object obj)
        {
            bool result = true;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
            ActionHelper.Execute(delegate()
            {
                var dto = obj as UserDto;
                var users = new List<UserDto> { dto };
                var name = string.Empty;
                GroupDto group;                
                if (result && dto.Role != _user.Role)
                {
                    // Remove group membership
                    if (_user.Role != UserRole.GuestUser)
                    {
                        name = (_user.Role == UserRole.Administrator ? "Administrators" : "Users");
                        group = new GroupDto { GroupName = name, GroupDomain = _domainName };
                        result = _service.Group.RemoveUsers(_serverDto, _tenantName, group, users, auth.Token);
                    }
                    if (result && dto.Role != UserRole.GuestUser)
                    {
                        // Add group membership
                        name = (dto.Role == UserRole.Administrator ? "Administrators" : "Users");
                        group = new GroupDto { GroupName = name, GroupDomain = _domainName };
                        result = _service.Group.AddUsers(_serverDto, _tenantName, group, users, auth.Token);
                    }
                }

                if (result && dto.ActAsUsers != _user.ActAsUsers)
                {
                    name = "ActAsUsers";
                    group = new GroupDto { GroupName = name, GroupDomain = _domainName };
                    result = (dto.ActAsUsers)
                        ? _service.Group.AddUsers(_serverDto, _tenantName, group, users, auth.Token)
                        : _service.Group.RemoveUsers(_serverDto, _tenantName, group, users, auth.Token);
                }

                if (result && dto.IsIdpAdmin != _user.IsIdpAdmin)
                {
                    name = "IdpProvisioningAdmin";
                    group = new GroupDto { GroupName = name, GroupDomain = _domainName };
                    result = (dto.IsIdpAdmin)
                        ? _service.Group.AddUsers(_serverDto, _tenantName, group, users, auth.Token)
                        : _service.Group.RemoveUsers(_serverDto, _tenantName, group, users, auth.Token);
                }
            }, auth);

            return result;
        }
    }
}
