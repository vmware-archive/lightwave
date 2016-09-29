/* * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved. * * Licensed under the Apache License, Version 2.0 (the “License”); you may not * use this file except in compliance with the License.  You may obtain a copy * of the License at http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an “AS IS” BASIS, without * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the * License for the specific language governing permissions and limitations * under the License. */
using VMIdentity.CommonUtils;
namespace VMPSCHighAvailability.Common.DTO
{
    /// <summary>
    /// Server dto
    /// </summary>
	public class ServerDto : IServerDTO
	{
		/// <summary>
		/// Gets or sets the name of the server.
		/// </summary>
		/// <value>The name of the server.</value>
		public string Server 
        {
			get;
			set;
		}

		/// <summary>
		/// Gets or sets the username.
		/// </summary>
		/// <value>The username.</value>
		public string UserName 
        {
			get;
			set;
		}
		/// <summary>
		/// Gets or sets the password.
		/// </summary>
		/// <value>The password.</value>
		public string Password 
        {
			get;
			set;
		}

		/// <summary>
		/// Gets or sets the domain.
		/// </summary>
		/// <value>The domain.</value>
		public string DomainName
        {
			get;
			set;
		}
		/// <summary>
		/// Gets or sets the upn.
		/// </summary>
		/// <value>The upn.</value>
		public string Upn 
        {
			get;
			set;
		}
		/// <summary>
		/// Gets or sets the GUID.
		/// </summary>
		/// <value>The GUID.</value>
		public string GUID 
        { 
            get; set; 
        }
	}
}

