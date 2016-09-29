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

namespace VMPSCHighAvailability.Common.DTO
{
    /// <summary>
    /// Service Dto
    /// </summary>
    public class ServiceDto
    {
		/// <summary>
		/// Gets or sets the name of the host.
		/// </summary>
		/// <value>The name of the host.</value>
		public string HostName 
        { 
            get; set; 
        }

        /// <summary>
        /// Name of the service
        /// </summary>
        public string ServiceName 
        { 
            get; set; 
        }

		/// <summary>
		/// Description of the service
		/// </summary>
		public string Description 
		{ 
			get; set; 
		}

        /// <summary>
        /// Port on which the service is listening
        /// </summary>
        public uint Port 
        { 
            get; set; 
        }

        /// <summary>
        /// Last heartbeat
        /// </summary>
        public DateTime LastHeartbeat 
        { 
            get; set; 
        }

        /// <summary>
        /// Is the service active or inactive
        /// </summary>
        public bool Alive 
        { 
            get; set; 
        }
    }
}
