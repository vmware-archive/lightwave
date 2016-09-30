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
using System.Runtime.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    public class ExternalIdentityProviderDto : IDataContext
    {
        [DataMember(EmitDefaultValue = false)]
        private String entityID;

        public String EntityID
        {
            get { return entityID; }
            set { entityID = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private String alias;

        public String Alias
        {
            get { return alias; }
            set { alias = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private List<String> nameIDFormats;

        public List<String> NameIDFormats
        {
            get { return nameIDFormats; }
            set { nameIDFormats = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private List<ServiceEndpointDto> ssoServices;

        public List<ServiceEndpointDto> SsoServices
        {
            get { return ssoServices; }
            set { ssoServices = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private List<ServiceEndpointDto> sloServices;

        public List<ServiceEndpointDto> SloServices
        {
            get { return sloServices; }
            set { sloServices = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private Dictionary<String, String> subjectFormats;

        public Dictionary<String, String> SubjectFormats
        {
            get { return subjectFormats; }
            set { subjectFormats = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private CertificateChainDto signingCertificates;

        public CertificateChainDto SigningCertificates
        {
            get { return signingCertificates; }
            set { signingCertificates = value; }
        }
        private List<TokenClaimGroupDto> tokenClaimGroups;

        public List<TokenClaimGroupDto> TokenClaimGroups
        {
            get { return tokenClaimGroups; }
            set { tokenClaimGroups = value; }
        }

        [DataMember(EmitDefaultValue = true)]
        private Boolean jitEnabled;

        public Boolean JitEnabled
        {
            get { return jitEnabled; }
            set { jitEnabled = value; }
        }

        private String upnSuffix;

        public String UpnSuffix
        {
            get { return upnSuffix; }
            set { upnSuffix = value; }
        }
    }
}
