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
    public class GroupDto : IDataContext
    {
        public GroupDto()
        {
            State = State.UnChanged;
            GroupDetails = new GroupDetailsDto();
        }

        [DataMember(EmitDefaultValue = false)]
        private string name;

        [DataMember(EmitDefaultValue = false)]
        private string objectId;

        [DataMember(EmitDefaultValue = false)]
        private GroupDetailsDto details;

        [DataMember(EmitDefaultValue = false)]
        private string domain;

        [DataMember(EmitDefaultValue = false)]
        private PrincipalDto alias;

        public string GroupName
        {
            get { return name; }
            set { name = value; }
        }

        public GroupDetailsDto GroupDetails
        {
            get { return details; }
            set { details = value; }
        }

        public string GroupDomain
        {
            get { return domain; }
            set { domain = value; }
        }

        public PrincipalDto Alias
        {
            get { return alias; }
            set { alias = value; }
        }

        public string ObjectId
        {
            get { return objectId; }
            set { objectId = value; }
        }

        public State State { get; set; }
    }
}