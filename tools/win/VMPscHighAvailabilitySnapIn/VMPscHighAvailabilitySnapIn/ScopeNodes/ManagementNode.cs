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

using Microsoft.ManagementConsole;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPscHighAvailabilitySnapIn.UI;
using System.Collections.Generic;
using System.Linq;

namespace VMPscHighAvailabilitySnapIn.ScopeNodes
{
    /// <summary>
    /// Management node
    /// </summary>
    public class ManagementNode : ScopeNode
    {
        /// <summary>
        /// Management view user control
        /// </summary>
        public ManagementViewControl UserControl;

        /// <summary>
        /// Server dto
        /// </summary>
        public ServerDto ServerDto { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="name">Node name</param>
        /// <param name="serverDto">Server dto</param>
        public ManagementNode(string name, ServerDto serverDto)
            : base(true)
        {
            DisplayName = name;
            ServerDto = serverDto;
            SelectedInfrastructureItem = 0;
            ImageIndex = SelectedImageIndex = (int)VMPSCHighAvailability.Common.ImageIndex.Management;
            AddViewDescription();
        }

        /// <summary>
        /// Add view description for the node
        /// </summary>
        void AddViewDescription()
        {            
            var fvd = new FormViewDescription
            {
                DisplayName = Constants.PscTableColumnNameId,
                ViewType = typeof(ManagementFormView),
                ControlType = typeof(ManagementViewControl)
            };
            ViewDescriptions.Add(fvd);
            ViewDescriptions.DefaultIndex = 0;
        }

        /// <summary>
        /// Gets the root node
        /// </summary>
        /// <returns>Root node</returns>
        public ServerNode  GetServerNode()
        {
            return this.Parent.Parent.Parent as ServerNode;
        }

        /// <summary>
        /// Gets the site name
        /// </summary>
        /// <returns>Site name</returns>
        public string GetSiteName()
        {
            return this.Parent.Parent.DisplayName;
        }

        /// <summary>
        /// Selected infrastructure entry.
        /// </summary>
        public int SelectedInfrastructureItem { get; set; }

        public void Cleanup()
        {
            this.UserControl.Cleanup();
        }
    }
}
