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
   /// <summary>
    /// Tenant service entity (serializable)
    /// </summary>
    [DataContract]
    public class TenantDto : IDataContext
    {
        /// <summary>
        /// Internal Guid
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        public string guid;

        /// <summary>
        /// Tenant Name
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        public string name;

        /// <summary>
        /// User Name
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        public string username;

        /// <summary>
        /// Password
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        public string password;

        /// <summary>
        /// Tenant Long Name
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        public string longName;

        /// <summary>
        /// Tenant Issuer Name
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        public string issuer;

        /// <summary>
        /// Tenant Key
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        public string key;

        [DataMember(EmitDefaultValue = false)]
        private TenantCredentialsDto credentials;

        public TenantCredentialsDto Credentials
        {
            get
            {
                return credentials;
            }
            set
            {
                credentials = value;
            }
        }
        /// <summary>
        /// Tenant Name
        /// </summary>
        public string Name
        {
            get
            {
                return name;
            }
            set { name = value; }
        }

        /// <summary>
        /// Tenant Long Name
        /// </summary>
        public string LongName
        {
            get
            {
                return longName;
            }
            set { longName = value; }
        }

        /// <summary>
        /// Tenant Issuer Name
        /// </summary>
        public string IssuerName
        {
            get
            {
                return issuer;
            }
            set { issuer = value; }
        }

        /// <summary>
        /// Tenant Key
        /// </summary>
        public string TenantKey
        {
            get
            {
                return key;
            }
            set { key = value; }
        }

        /// <summary>
        /// Internal Guid
        /// </summary>
        public string Guid
        {
            get
            {
                return guid;
            }
            set { guid = value; }
        }

        /// <summary>
        /// User name
        /// </summary>
        public string Username
        {
            get
            {
                return username;
            }
            set { username = value; }
        }

        /// <summary>
        /// Password
        /// </summary>
        public string Password
        {
            get
            {
                return password;
            }
            set { password = value; }
        }
    }
}