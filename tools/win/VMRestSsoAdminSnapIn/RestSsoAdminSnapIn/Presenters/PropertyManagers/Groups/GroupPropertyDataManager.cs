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
using Vmware.Tools.RestSsoAdminSnapIn.Service.Group;

namespace Vmware.Tools.RestSsoAdminSnapIn.Presenters.PropertyManagers.Groups
{
    public class GroupPropertyDataManager
    {
        private readonly GroupDto _groupDto;
        private readonly ServiceGateway _service;
        private readonly ServerDto _server;
        private readonly string _tenantName;
        private readonly string _domainName;

        public GroupPropertyDataManager(GroupDto group, ServiceGateway service, ServerDto server, string tenantName, string domainName)
        {
            _groupDto = group;
            _service = service;
            _server = server;
            _tenantName = tenantName;
            _domainName = domainName;
        }

        public GroupMembershipDto GetMembershipInfo()
        {
            GroupMembershipDto memberInfo = new GroupMembershipDto();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_server, _tenantName);
            ActionHelper.Execute(delegate()
            {

                var userInfo = _service.Group.GetMembers(_server, _tenantName, _groupDto, GroupMemberType.USER, auth.Token);
                memberInfo.Users = userInfo.Users == null ? new List<UserDto>() : new List<UserDto>(userInfo.Users);
                userInfo = _service.Group.GetMembers(_server, _tenantName, _groupDto, GroupMemberType.GROUP, auth.Token);
                memberInfo.Groups = userInfo.Groups == null ? new List<GroupDto>() : new List<GroupDto>(userInfo.Groups);
            }, auth);
            if (memberInfo == null)
                memberInfo = new GroupMembershipDto();
            return memberInfo;
        }

        public bool UpdateMembers(GroupMembershipDto members)
        {
            if (members == null) return false;
            return RemoveMembers(members) && AddMembers(members);
        }

        public bool RemoveMembers(GroupMembershipDto members)
        {
            if (members == null) return false;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_server, _tenantName);
            ActionHelper.Execute(delegate()
            {

                var users = members.Users.Where(x => x.State == State.ForDelete).ToList();
                if (users.Count > 0)
                {
                    _service.Group.RemoveUsers(_server, _tenantName, _groupDto, users, auth.Token);
                }

                var solnUsers = members.SolutionUsers.Where(x => x.State == State.ForDelete).ToList();
                if (solnUsers.Count > 0)
                {
                    _service.Group.RemoveSolutionUsers(_server, _tenantName, _groupDto, solnUsers, auth.Token);
                }

                var groups = members.Groups.Where(x => x.State == State.ForDelete).ToList();
                if (groups.Count > 0)
                {
                    _service.Group.RemoveGroups(_server, _tenantName, _groupDto, groups, auth.Token);
                }
            }, auth);
            return true;
        }

        public bool AddMembers(GroupMembershipDto members)
        {
            if (members == null) return false;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_server, _tenantName);
            ActionHelper.Execute(delegate()
            {
                var users = members.Users.Where(x => x.State == State.ForAdd).ToList();
                if (users.Count > 0)
                {
                    _service.Group.AddUsers(_server, _tenantName, _groupDto, users, auth.Token);
                }

                var solnUsers = members.SolutionUsers.Where(x => x.State == State.ForAdd).ToList();
                if (solnUsers.Count > 0)
                {
                    _service.Group.AddSolutionUsers(_server, _tenantName, _groupDto, solnUsers, auth.Token);
                }

                var groups = members.Groups.Where(x => x.State == State.ForAdd).ToList();
                if (groups.Count > 0)
                {
                    _service.Group.AddGroups(_server, _tenantName, _groupDto, groups, auth.Token);
                }
            }, auth);
            return true;
        }
        public IList<GroupDto> GetGroups(string name, string domainName)
        {
            var result = new List<GroupDto>();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_server, _tenantName);
            ActionHelper.Execute(delegate()
            {
                var membershipDto = _service.Tenant.Search(_server, _tenantName, domainName, MemberType.GROUP, SearchType.NAME, auth.Token, name);
                result = membershipDto.Groups;
            }, auth);
            return result;
        }
        public IList<UserDto> GetUsers(string name, string domainName)
        {
            var result = new List<UserDto>();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_server, _tenantName);
            ActionHelper.Execute(delegate()
            {
                var membershipDto = _service.Tenant.Search(_server, _tenantName, domainName, MemberType.USER, SearchType.NAME, auth.Token, name);
                result = membershipDto.Users;
            }, auth);
            return result;
        }


        public GroupDto Update(GroupDto groupDto)
        {
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_server, _tenantName);
            ActionHelper.Execute(delegate()
            {
                groupDto = _service.Group.Update(_server, _tenantName, groupDto, auth.Token);
            }, auth);
            return groupDto;
        }

        public List<IdentityProviderDto> GetDomains()
        {
            var identityProviders = new List<IdentityProviderDto>();
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_server, _tenantName);
            ActionHelper.Execute(delegate()
            {
                identityProviders = _service.IdentityProvider.GetAll(_server, _tenantName, auth.Token);
            }, auth);
            return identityProviders;
        }
    }
}