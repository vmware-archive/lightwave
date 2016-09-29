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
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Service;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using System.Linq;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.User
{
    public class UserGroupsPropertyManager : IPropertyDataManager
    {
        private readonly UserDto _user;
        private readonly ServerDto _serverDto;
        private readonly ServiceGateway _service;
        private readonly string _tenantName;
        private readonly string _domainName;
        private List<GroupDto> _orignialUserMembership;

        public UserGroupsPropertyManager(ServiceGateway service, UserDto user, ServerDto serverDto, string tenantName, string domainName)
        {
            _user = user;
            _tenantName = tenantName;
            _serverDto = serverDto;
            _domainName = domainName;
            _service = service;
        }

        public object GetData()
        {
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
            ActionHelper.Execute(delegate()
            {                
                _orignialUserMembership = _service.User.GetUserGroups(_serverDto, _tenantName, _user, auth.Token);
            }, auth);
            return new List<GroupDto>(_orignialUserMembership);
        }

        public bool Apply(object obj)
        {
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenantName);
            ActionHelper.Execute(delegate()
            {
                var modifiedGroupMembership = (List<GroupDto>)obj;
                var users = new List<UserDto> { _user };                
                foreach (var group in _orignialUserMembership)
                {
                    var fullName = group.GroupName + "@" + group.GroupDomain;
                    if (modifiedGroupMembership.FirstOrDefault(x => (x.GroupName + "@" + x.GroupDomain) == fullName) == null)
                    {
                        // remove
                        _service.Group.RemoveUsers(_serverDto, _tenantName, group, users, auth.Token);
                    }
                }

                foreach (var group in modifiedGroupMembership)
                {
                    var fullName = group.GroupName + "@" + group.GroupDomain;
                    if (_orignialUserMembership.FirstOrDefault(x => (x.GroupName + "@" + x.GroupDomain) == fullName) == null)
                    {
                        // add
                        _service.Group.AddUsers(_serverDto, _tenantName, group, users, auth.Token);
                    }
                }
            }, auth);
            return true;
        }

        public List<GroupDto> AddGroups(PropertySheet sheet)
        {
            var mgr = new AllGroupsPropertyManager(_service, _serverDto, _tenantName, _domainName);
            var frm = new SelectGroups(mgr);
            if (sheet.ShowDialog(frm) == DialogResult.OK)
                return mgr.GetSelectedGroups();
            return null;
        }
    }
}
