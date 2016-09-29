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
using System.ComponentModel;
using System.Runtime.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    [TypeConverter(typeof(ExpandableObjectConverter))]
    [Serializable]
    [DisplayName("Token Summary")]
    [DescriptionAttribute("Displays the details of the token and claims.")]
    public class TokenDisplayDto
    {
        public TokenDisplayDto() { }
        [DataMember]
        [DescriptionAttribute("Displays the name of the Claims Identity Provider.")]
        [ReadOnlyAttribute(true)]
        public string Name { get; set; }
        [DataMember]
        [DescriptionAttribute("Displays the type of authentication used by Calims Identity Provider.")]
        [ReadOnlyAttribute(true)]
        public string AuthenticationType { get; set; }
        [DataMember]
        [DescriptionAttribute("Displays if the authentication succeeded.")]
        [ReadOnlyAttribute(true)]
        public bool IsAuthenticated{ get; set; }       
        [DataMember]
        [DescriptionAttribute("Displays the raw token issued by the Identity Provider.")]
        [ReadOnlyAttribute(true)]
        public string RawToken { get; set; }
        [DataMember]
        [DescriptionAttribute("Displays the id token issued by the Identity Provider.")]
        [ReadOnlyAttribute(true)]
        public string IdToken { get; set; }
        [DataMember]
        [DescriptionAttribute("Displays the access token issued by the Identity Provider.")]
        [ReadOnlyAttribute(true)]
        public string AccessToken { get; set; }
        [DataMember]
        [DescriptionAttribute("Displays the refresh token issued by the Identity Provider.")]
        [ReadOnlyAttribute(true)]
        public string RefreshToken { get; set; }        
    }
}
