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


using System.Runtime.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    public class ActiveDirectoryJoinRequestDto : IDataContext
    {
        public ActiveDirectoryJoinRequestDto()
        {
        }
        [DataMember(EmitDefaultValue = false)]
        private string username;

        public string Username
        {
            get { return username; }
            set { username = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string password;

        public string Password
        {
            get { return password; }
            set { password = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string domain;

        public string Domain
        {
            get { return domain; }
            set { domain = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string ou;

        public string OrganizationalUnit
        {
            get { return ou; }
            set { ou = value; }
        }
    }
}
