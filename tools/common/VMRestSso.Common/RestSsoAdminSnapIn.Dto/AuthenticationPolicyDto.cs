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
    public class AuthenticationPolicyDto : IDataContext
    {
        public AuthenticationPolicyDto() { }
        
        [DataMember]
        private bool passwordBasedAuthenticationEnabled;

        [DescriptionAttribute("Sets Password Based Authentication ")]
        public bool PasswordBasedAuthentication
        {
            get { return passwordBasedAuthenticationEnabled; }
            set { passwordBasedAuthenticationEnabled = value; }
        }

        [DataMember]
        private bool windowsBasedAuthenticationEnabled;

        [DescriptionAttribute("Sets Windows Based Authentication ")]
        public bool WindowsBasedAuthentication
        {
            get { return windowsBasedAuthenticationEnabled; }
            set { windowsBasedAuthenticationEnabled = value; }
        }

        [DataMember]
        private bool certificateBasedAuthenticationEnabled;

        [DescriptionAttribute("Sets the Certificate Based Authentication")]
        public bool CertificateBasedAuthentication
        {
            get { return certificateBasedAuthenticationEnabled; }
            set { certificateBasedAuthenticationEnabled = value; }
        }

        [DataMember]
        private ClientCertificatePolicyDto clientCertificatePolicy;

         [DescriptionAttribute("Displays the Certificate Policy")]
        public ClientCertificatePolicyDto ClientCertificatePolicy
        {
            get { return clientCertificatePolicy; }
            set { clientCertificatePolicy = value; }
        }
        
    }
}
