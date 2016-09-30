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
    public class OidcClientMetadataDto : IDataContext
    {

        [DataMember(EmitDefaultValue = false)]
        private List<String> redirectUris;

        public List<String> RedirectUris
        {
            get { return redirectUris; }
            set { redirectUris = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private String tokenEndpointAuthMethod;

        public String TokenEndpointAuthMethod
        {
            get { return tokenEndpointAuthMethod; }
            set { tokenEndpointAuthMethod = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private List<String> postLogoutRedirectUris;

        public List<String> PostLogoutRedirectUris
        {
            get { return postLogoutRedirectUris; }
            set { postLogoutRedirectUris = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private String logoutUri;

        public String LogoutUri
        {
            get { return logoutUri; }
            set { logoutUri = value; }
        }

        [DataMember(EmitDefaultValue = false)]
        private String certSubjectDN;

        public String CertSubjectDN
        {
            get { return certSubjectDN; }
            set { certSubjectDN = value; }
        }
    }
}
