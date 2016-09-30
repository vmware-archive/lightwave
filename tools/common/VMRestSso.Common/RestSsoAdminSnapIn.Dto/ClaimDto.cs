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
using System.ComponentModel;
using System.Runtime.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    [TypeConverter(typeof(ExpandableObjectConverter))]
    [Serializable]
    [DisplayName("Claim Details")]
    public class ClaimDto
    {
        [DataMember]
        [DescriptionAttribute("Displays the Issuer of the token")]
        [ReadOnlyAttribute(true)]
        public string Issuer { get; set; }
        [DataMember]
        [DescriptionAttribute("Displays the Original Issuer of the token")]
        [ReadOnlyAttribute(true)]
        public string OriginalIssuer { get; set; }
        [DataMember]
        [DescriptionAttribute("Displays the Type of the token")]
        [ReadOnlyAttribute(true)]
        public string Type { get; set; }
        [DataMember]
        [DescriptionAttribute("Displays the type of the claim")]
        [ReadOnlyAttribute(true)]
        public string ValueType { get; set; }
        [DataMember]
        [DescriptionAttribute("Displays the value of the claim")]
        [ReadOnlyAttribute(true)]
        public string Value { get; set; }
    }
}
