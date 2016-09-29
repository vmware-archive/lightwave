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
    public class AttributeDto
    {
        [DataMember(EmitDefaultValue = false)]
        private string name;

        public string Name
        {
            get { return name; }
            set { name = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string friendlyName;

        public string FriendlyName
        {
            get { return friendlyName; }
            set { friendlyName = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string nameFormat;

        public string NameFormat
        {
            get { return nameFormat; }
            set { nameFormat = value; }
        }
    }
}
