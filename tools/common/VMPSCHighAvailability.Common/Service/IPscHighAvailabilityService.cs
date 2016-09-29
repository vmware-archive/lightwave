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
using VMPSCHighAvailability.Common.DTO;

namespace VMPSCHighAvailability.Common.Service
{
    /// <summary>
    /// Interface for the service
    /// </summary>
	public interface IPscHighAvailabilityService
	{
        /// <summary>
        /// Get the topology of the HA setup
        /// </summary>
        /// <returns>List of the nodes</returns>
        List<NodeDto> GetTopology(ManagementDto dto, ServerDto serverDto);

        /// <summary>
        /// Checks connectivity of the server
        /// </summary>
        /// <param name="serverDto">Server with credentials i.e. UPN and password</param>
        ManagementDto Connect(ServerDto serverDto);

        /// <summary>
        /// Updates the node dto with the latest status
        /// </summary>
        /// <param name="pscDto">Node Dto</param>
        /// <param name="serverDto">Node details</param>
        /// <returns>Updated Node dto</returns>
		NodeDto UpdateStatus(NodeDto dto, ServerDto serverDto);

        /// <summary>
        /// Sets the management node to legacy mode
        /// </summary>
        /// <param name="legacy">True if legacy mode needs to be set false otherwise</param>
        /// <param name="serverDto">Management node details</param>
        void SetLegacyMode(bool legacy, ServerDto serverDto);

		/// <summary>
		/// Gets the management node details.
		/// </summary>
		/// <returns>The management node details.</returns>
		/// <param name="serverDto">Server dto.</param>
		ManagementDto GetManagementNodeDetails (ServerDto serverDto);

        /// <summary>
        /// Gets the domain functional level for the server
        /// </summary>
        /// <param name="serverDto">Server details</param>
        /// <returns>Domain functional level</returns>
        string GetDomainFunctionalLevel(ServerDto serverDto);
	}
}

