/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


using Microsoft.ManagementConsole;
using VMDirSnapIn.Views;
using VMIdentity.CommonUtils;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSnapIn.ScopeNodes
{
    public class VMDirRootNode : ScopeNode
    {
        public VMDirRootNode()
        {
            DisplayName = MMCMiscUtil.GetBrandConfig(CommonConstants.DIR_ROOT);
            AddViewDescription();
        }
        void AddViewDescription()
        {
            FormViewDescription fvd = new FormViewDescription();
            fvd.DisplayName = "Users (FormView)";
            fvd.ViewType = typeof(ResultPaneFormView);
            fvd.ControlType = typeof(ResultPaneControl);

            // Attach the view to the root node
            this.ViewDescriptions.Add(fvd);
            this.ViewDescriptions.DefaultIndex = 0;
        }
    }
}
