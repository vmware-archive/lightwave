/* * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved. * * Licensed under the Apache License, Version 2.0 (the “License”); you may not * use this file except in compliance with the License.  You may obtain a copy * of the License at http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an “AS IS” BASIS, without * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the * License for the specific language governing permissions and limitations * under the License. */
using System;

namespace VMPSCHighAvailability.Common.DTO
{
    public class NodeDto
    {        /// <summary>        /// Domain name
        /// </summary>
        public string Domain { get; set; }
        /// <summary>
        /// Site name
        /// </summary>
        public string Sitename { get; set; }
        /// <summary>
        /// Name of the node
        /// </summary>
        public string Name { get; set; }
        /// <summary>
        /// IP Address
        /// </summary>
        public string Ip { get; set; }
        /// <summary>
        /// Gets or sets the last heart beat.
        /// </summary>
        /// <value>The last heart beat.</value>
        public DateTime? LastHeartBeat { get; set; }
        /// <summary>
        /// Gets or sets the type of the node.
        /// </summary>
        /// <value>The type of the node.</value>
        public NodeType NodeType { get; set; }
        /// <summary>
        /// Is the node active or inactive
        /// </summary>
        public bool Active { get; set; }

        /// <summary>
        /// Gets or sets if the node on a remote site
        /// </summary>
        public bool IsRemote { get; set; }
    }
}
