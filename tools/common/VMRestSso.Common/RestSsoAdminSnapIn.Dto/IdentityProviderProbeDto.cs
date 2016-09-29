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
    public class IdentityProviderProbeDto : IDataContext
    {
        [DataMember]
        private string providerURI;

        public string ProviderURI
        {
            get { return providerURI; }
            set { providerURI = value; }
        }

        [DataMember]
        private string authenticationType;

        public string AuthenticationType
        {
            get { return authenticationType; }
            set { authenticationType = value; }
        }

        [DataMember]
        private string username;

        public string Username
        {
            get { return username; }
            set { username = value; }
        }

        [DataMember]
        private string password;

        public string Password
        {
            get { return password; }
            set { password = value; }
        }
    }
}
