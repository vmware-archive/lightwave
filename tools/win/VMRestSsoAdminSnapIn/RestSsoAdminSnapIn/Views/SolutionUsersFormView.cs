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

using System.Windows.Forms;
using Microsoft.ManagementConsole;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters.Nodes;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views{    public class SolutionUsersFormView : FormView    {               protected override void OnInitialize(AsyncStatus status)        {            // Call the parent method.            base.OnInitialize(status);
            // Get a typed reference to the hosted control            // that is set up by the form view description.            var userControl = (SolutionUsersControl)Control;
            var ssoSolutionUsersNode = ScopeNode as SolutionUsersNode;
            if (ssoSolutionUsersNode != null)
                ssoSolutionUsersNode.SolutionUsersControl = userControl;        }       
    }}