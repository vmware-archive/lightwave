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
using System.Collections.Generic;
using VMPSCHighAvailability.Common;
using VMPSCHighAvailability.Common.DTO;
using VMPscHighAvailabilitySnapIn.DataSources;
using VMPscHighAvailabilitySnapIn.UI;

namespace VMPscHighAvailabilitySnapIn.ScopeNodes
{
    /// <summary>
    /// Host Node
    /// </summary>
    public abstract class HostNode : ScopeNode
    {

        public HostNode(): base(true)
        {
        }

        public HostNode(bool collapse)
            : base(collapse)
        {
        }

        public void AddViewDescription()
        {
            var fvd = new FormViewDescription
            {
                DisplayName = Constants.PscTableColumnNameId,
                ViewType = typeof(GlobalFormView),
                ControlType = typeof(GlobalView)
            };
            ViewDescriptions.Add(fvd);
            ViewDescriptions.DefaultIndex = 0;
        }

        /// <summary>
        /// Gloabl list view user control
        /// </summary>
        public GlobalView UserControl;

        public void Refresh()
        {
            OnExpand(null);
        }
    }
}
