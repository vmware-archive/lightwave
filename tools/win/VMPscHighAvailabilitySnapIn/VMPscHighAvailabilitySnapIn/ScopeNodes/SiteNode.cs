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

using System.Collections.Generic;
using System.Linq;
using Microsoft.ManagementConsole;
using VMPSCHighAvailability.Common.DTO;

namespace VMPscHighAvailabilitySnapIn.ScopeNodes
{
    /// <summary>
    /// Site node
    /// </summary>
    public class SiteNode : HostNode
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="sitename">Site name</param>
        public SiteNode(string displayName)
        {
            DisplayName = displayName;
            ImageIndex = SelectedImageIndex = (int)VMPSCHighAvailability.Common.ImageIndex.Site;
            AddViewDescription();
        }

        public List<NodeDto> Hosts
        {
            get
            {
                var serverNode = this.Parent as ServerNode;
                return serverNode.Hosts.Where(x => x.Sitename == DisplayName).ToList();
            }            
        }
    }
}
