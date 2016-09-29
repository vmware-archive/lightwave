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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    public class AssertionConsumerServiceDto : IDataContext
    {
        [DataMember(EmitDefaultValue = false)]
        private string name;

        public string Name
        {
            get { return name; }
            set { name = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string endpoint;

        public string Endpoint
        {
            get { return endpoint; }
            set { endpoint = value; }
        }
        [DataMember(EmitDefaultValue = false)]
        private string binding;

        public string Binding
        {
            get { return binding; }
            set { binding = value; }
        }
        [DataMember]
        public int index;

        public int Index
        {
            get { return index; }
            set { index = value; }
        }

        public bool IsDefault { get; set; }
    }
}
