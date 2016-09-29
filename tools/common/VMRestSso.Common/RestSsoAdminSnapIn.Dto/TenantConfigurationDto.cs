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
using Vmware.Tools.RestSsoAdminSnapIn.Dto.PropertyDescriptors;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    [TypeConverter(typeof(ExpandableObjectConverter))]
    [Serializable]
    public class TenantConfigurationDto : IDataContext
    {
        public TenantConfigurationDto() { }
        [DataMember]
        [DescriptionAttribute("Displays the Brand policy configured for the tenant")]
        private BrandPolicyDto brandPolicy;

        public BrandPolicyDto BrandPolicy
        {
            get { return brandPolicy; }
            set { brandPolicy = value; }
        }

        [DataMember]
        [DescriptionAttribute("Displays the Authentication policy configured for the tenant")]
        private AuthenticationPolicyDto authenticationPolicy;

        public AuthenticationPolicyDto AuthenticationPolicy
        {
            get { return authenticationPolicy; }
            set { authenticationPolicy = value; }
        }

        [DataMember]
        private PasswordPolicyDto passwordPolicy;

        [DescriptionAttribute("Displays the Password policy configured for the tenant")]
        public PasswordPolicyDto PasswordPolicy
        {
            get { return passwordPolicy; }
            set { passwordPolicy = value; }
        }
        [DataMember]
        private LockoutPolicyDto lockoutPolicy;

        [DescriptionAttribute("Displays the Lockout policy configured for the tenant")]
        public LockoutPolicyDto LockoutPolicy
        {
            get { return lockoutPolicy; }
            set { lockoutPolicy = value; }
        }
        [DataMember]
        private TokenPolicyDto tokenPolicy;

        [DescriptionAttribute("Displays the Token policy configured for the tenant")]
        public TokenPolicyDto TokenPolicy
        {
            get { return tokenPolicy; }
            set { tokenPolicy = value; }
        }
        [DataMember]
        private ProviderPolicyDto providerPolicy;

        [DescriptionAttribute("Display the Provider policy configured for the tenant")]
        public ProviderPolicyDto ProviderPolicy
        {
            get { return providerPolicy; }
            set { providerPolicy = value; }
        }
    }
}
