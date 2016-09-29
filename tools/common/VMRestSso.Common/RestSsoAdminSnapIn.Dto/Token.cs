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
using System;
using System.ComponentModel;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    /// <summary>
    /// Token service entity
    /// </summary>
    [DataContract]
    [TypeConverter(typeof(ExpandableObjectConverter))]
    [Serializable]
    [DisplayName("Token")]
    public class Token : IDataContext
    {
        public Token()
        {

        }

        public Token(string token, TokenType tokenType)
        {
            access_token = token;
            token_type = tokenType.ToString();
        }
        /// <summary>
        /// Expiration time in seconds
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        int expires_in = 0;

        /// <summary>
        /// Type of Token
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        string token_type = null;

        /// <summary>
        /// Token id
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        string id_token = null;

        /// <summary>
        /// Access Token
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        string access_token = null;

        /// <summary>
        /// Refresh Token
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        string refresh_token = null;

        /// <summary>
        /// Client Id
        /// </summary>
        [DataMember(EmitDefaultValue = false)]
        string clientId = null;

        private static DateTime _creationDate = DateTime.UtcNow;

        /// <summary>
        /// Expiration time in seconds
        /// </summary>
        [ReadOnlyAttribute(true)]
        public int ExpiresIn
        {
            // Implement get and set.
            get { return expires_in; }
            //set { expires_in = value; }

        }

        /// <summary>
        /// Type of Token
        /// </summary>
        [ReadOnlyAttribute(true)]
        public string TokenType
        {
            // Implement get and set.
            get { return token_type; }
            set { token_type = value; }
        }

        /// <summary>
        /// Token id
        /// </summary>
        [ReadOnlyAttribute(true)]
        public string IdToken
        {
            // Implement get and set.
            get { return id_token; }
            //set { id_token = value; }

        }

        /// <summary>
        /// Access Token
        /// </summary>
        [ReadOnlyAttribute(true)]
        public string AccessToken
        {
            // Implement get and set.
            get { return access_token; }
            //set { access_token = value; }
        }
        public string Signature { get; set; }

        /// <summary>
        /// Refresh Token
        /// </summary>
        [ReadOnlyAttribute(true)]
        public string RefreshToken
        {
            // Implement get and set.
            get { return refresh_token; }
            set { refresh_token = value; }
        }


        /// <summary>
        /// Client Id
        /// </summary>
        [ReadOnlyAttribute(true)]
        public string ClientId
        {
            // Implement get and set.
            get { return clientId; }
            set { clientId = value; }
        }

        /// <summary>
        /// Has the token expired
        /// </summary>
        [ReadOnlyAttribute(true)]
        public bool HasExpired
        {
            get
            {
                return token_type != "SAML" && (DateTime.UtcNow.Subtract(_creationDate).TotalSeconds > 595);
            }
        }

        /// <summary>
        /// Has the token expired
        /// </summary>
        [ReadOnlyAttribute(true)]
        public string Raw
        {
            get;
            set;
        }

		/// <summary>
		/// Role of the user holding the token
		/// </summary>
		[ReadOnlyAttribute(true)]
		public string Role {
			get;
			set;
		}
    }
}