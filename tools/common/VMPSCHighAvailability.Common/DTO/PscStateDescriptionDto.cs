/* * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved. * * Licensed under the Apache License, Version 2.0 (the “License”); you may not * use this file except in compliance with the License.  You may obtain a copy * of the License at http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an “AS IS” BASIS, without * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the * License for the specific language governing permissions and limitations * under the License. */
/// <summary>
/// Psc state description dto namespace.
/// </summary>
namespace VMPSCHighAvailability.Common.DTO
{
	/// <summary>
	/// Psc state description dto.
	/// </summary>
	public class StateDescriptionDto
	{
		/// <summary>
		/// Gets or sets the description.
		/// </summary>
		/// <value>The description.</value>
		public string Description {get;set;}
		/// <summary>
		/// Gets or sets the long description.
		/// </summary>
		/// <value>The long description.</value>
		public string LongDescription {get;set;}

        /// <summary>
        /// Health
        /// </summary>
        public Health Heath { get; set; }
	}
}

