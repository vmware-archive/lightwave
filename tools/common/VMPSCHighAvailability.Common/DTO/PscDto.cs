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
using VMAFD.Client;
namespace VMPSCHighAvailability.Common.DTO
{
	/// <summary>
	/// Infrastucture dto.
	/// </summary>
	public class InfrastructureDto : NodeDto
	{
		public InfrastructureDto()
		{
			Sitename = string.Empty;
			IsAffinitized = false;
            Services = new List<ServiceDto>();
		}
        /// <summary>
        /// Gets or sets a value indicating whether this instance is affinitized.
        /// </summary>
        /// <value><c>true</c> if this instance is affinitized; otherwise, <c>false</c>.</value>
        public bool IsAffinitized { get; set; }
        /// <summary>
        /// List of the services and their status
        /// </summary>
        public List<ServiceDto> Services { get; set; }
        /// <summary>
        /// Last ping date time
        /// </summary>
        public DateTime LastPing { get; set; }
        /// <summary>
        /// Last response time
        /// </summary>
        public DateTime LastResponseTime { get; set; }
        /// <summary>
        /// Last error details
        /// </summary>
        public ErrorDto LastError { get; set; }
        /// <summary>
        /// Partners
        /// </summary>
        public IList<string> Partners { get; set; }
    }
}