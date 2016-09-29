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
namespace VMPSCHighAvailability.Common.DTO
{
    /// <summary>
    /// Management node details Dto
    /// </summary>
    public class ManagementDto : NodeDto
    {
        /// <summary>
        /// State of the node
        /// </summary>
        public StateDescriptionDto State { get; set; }
        /// <summary>
        /// Last heartbeat of the node
        /// </summary>
        public DateTime LastHeartBeat { get; set; }
        /// <summary>
        /// Domain controller
        /// </summary>
        public InfrastructureDto DomainController { get; set; }
        /// <summary>
        /// Legacy mode or default HA mode
        /// </summary>
        public bool Legacy { get; set; }

        /// <summary>
        /// List of all the Domain controllers and their state
        /// </summary>
        public List<InfrastructureDto> DomainControllers { get; set; }
    }
}
