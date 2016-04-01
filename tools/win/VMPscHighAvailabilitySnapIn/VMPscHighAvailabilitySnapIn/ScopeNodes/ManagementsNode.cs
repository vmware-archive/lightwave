﻿/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

using System.Collections.Generic;
using System.Linq;
using Microsoft.ManagementConsole;
using VMPSCHighAvailability.Common.DTO;

namespace VMPscHighAvailabilitySnapIn.ScopeNodes
{
    /// <summary>
    /// Managements Group node
    /// </summary>
    public class ManagementsNode : HostNode
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="name">Node name</param>
        public ManagementsNode(bool collapse)
            : base(collapse)
        {
            DisplayName = "vCenter Servers";
            ImageIndex = SelectedImageIndex = (int)(VMPSCHighAvailability.Common.ImageIndex.ManagementGroup);
            AddViewDescription();
        }

        /// <summary>
        /// Gets the root node
        /// </summary>
        /// <returns>Root node</returns>
        public ServerNode GetServerNode()
        {
            return this.Parent.Parent as ServerNode;
        }

        /// <summary>
        /// Gets the site name
        /// </summary>
        /// <returns>Site name</returns>
        public string GetSiteName()
        {
            return this.Parent.DisplayName;
        }

        /// <summary>
        /// Hosts
        /// </summary>
        public List<NodeDto> Hosts
        {
            get
            {
                var serverNode = GetServerNode();
                var siteName = GetSiteName();
                return serverNode.Hosts
                    .Where(x => x.Sitename == siteName
                        && x.NodeType == VMPSCHighAvailability.Common.NodeType.Management)
                    .ToList();
            }
        }
    }
}