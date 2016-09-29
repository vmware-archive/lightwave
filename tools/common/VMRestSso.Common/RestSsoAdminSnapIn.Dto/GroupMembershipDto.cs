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
using System.Runtime.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    public class GroupMembershipDto : IDataContext
    {
        public GroupMembershipDto()
        {
            users = new List<UserDto>();
            groups = new List<GroupDto>();
            solutionUsers = new List<SolutionUserDto>();
        }

        [DataMember]
        private List<UserDto> users;

        [DataMember]
        private List<GroupDto> groups;

        [DataMember]
        private List<SolutionUserDto> solutionUsers;

        public List<UserDto> Users
        {
            get { return users; }
            set { users = value; }
        }

        public List<GroupDto> Groups
        {
            get { return groups; }
            set { groups = value; }
        }

        public List<SolutionUserDto> SolutionUsers
        {
            get { return solutionUsers; }
            set { solutionUsers = value; }
        }
    }
}