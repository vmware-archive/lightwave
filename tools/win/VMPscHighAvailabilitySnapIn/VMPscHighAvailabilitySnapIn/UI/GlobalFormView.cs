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
using System.Windows.Forms;
using VMPscHighAvailabilitySnapIn.ScopeNodes;

namespace VMPscHighAvailabilitySnapIn.UI{    /// <summary>
    /// Global form view
    /// </summary>    public class GlobalFormView : FormView    {        /// <summary>
        /// User control for global view
        /// </summary>        private GlobalView _control;                /// <summary>
        /// On initialization of the view
        /// </summary>
        /// <param name="status">Async status</param>        protected override void OnInitialize(AsyncStatus status)        {            base.OnInitialize(status);
            _control = (GlobalView)Control;

            if (ScopeNode is SiteNode)
            {
                var node = ScopeNode as SiteNode;
                _control.View = ViewType.Site;
                if (node != null) node.UserControl = _control;
            }
            else if (ScopeNode is InfrastructuresNode)
            {
                var node = ScopeNode as InfrastructuresNode;
                _control.View = ViewType.InfraNodeGroup;
                if (node != null) node.UserControl = _control;
            }
            else if (ScopeNode is ManagementsNode)
            {
                var node = ScopeNode as ManagementsNode;
                _control.View = ViewType.MgmtNodeGroup;
                if (node != null) node.UserControl = _control;
            }
        }

        protected override void OnShow()
        {
            base.OnShow();
            _control.RefreshDataSource();
        }    }}